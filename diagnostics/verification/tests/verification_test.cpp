#include "isb/verification/assertions.hpp"
#include "isb/verification/json.hpp"
#include "isb/verification/runner.hpp"
#include "isb/verification/snapshot.hpp"
#include "isb/verification/types.hpp"

#include <chrono>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

using namespace isb::verification;

namespace {

void require(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

void testSnapshotDiffAllCategories() {
    Snapshot base;
    base.environment_data["os"] = "Linux";
    base.gpu_data["clock"] = "1000";
    base.api_data["vk"] = "1.2";
    base.capability_states["rt"] = CapabilityState::Supported;
    base.metadata["vendor"] = "Mock";

    Snapshot actual = base;
    actual.environment_data["os"] = "Windows";
    actual.gpu_data["clock"] = "1200";
    actual.api_data["vk"] = "1.3";
    actual.capability_states["rt"] = CapabilityState::Unsupported;
    actual.metadata["vendor"] = "MockVendor";

    const SnapshotDiff diff = compareSnapshots(base, actual);

    require(diff.has_differences, "snapshot diff should report differences");
    require(diff.environment_diffs.size() == 1, "environment diff count mismatch");
    require(diff.gpu_diffs.size() == 1, "gpu diff count mismatch");
    require(diff.api_data_diffs.size() == 1, "api diff count mismatch");
    require(diff.capability_diffs.size() == 1, "capability diff count mismatch");
    require(diff.metadata_diffs.size() == 1, "metadata diff count mismatch");
    require(diff.environment_diffs.at("os").baseline == "Linux", "baseline value mismatch");
    require(diff.environment_diffs.at("os").actual == "Windows", "actual value mismatch");
}

void testSnapshotDiffDetectsAddedAndRemovedKeys() {
    Snapshot base;
    base.environment_data["present"] = "yes";
    base.environment_data["removed"] = "old";

    Snapshot actual;
    actual.environment_data["present"] = "yes";
    actual.environment_data["added"] = "new";

    const SnapshotDiff diff = compareSnapshots(base, actual);
    require(diff.has_differences, "added/removed keys must be differences");
    require(diff.environment_diffs.at("added").baseline == "<missing>", "added baseline marker mismatch");
    require(diff.environment_diffs.at("removed").actual == "<missing>", "removed actual marker mismatch");
}

void testBackendModesAndUnknownStatus() {
    auto backend = std::make_shared<DefaultMockBackend>();
    RegressionRunner runner(backend);

    TestCase test_case;
    test_case.name = "Unknown Test";

    backend->setMode(BackendBehaviorMode::Unknown);
    const TestResult unknown = runner.runTestCase(test_case);
    require(unknown.status == TestStatus::Unknown, "unknown mode must remain unknown");

    backend->setMode(BackendBehaviorMode::Timeout);
    test_case.name = "Timeout Test";
    test_case.timeout = std::chrono::milliseconds(1);
    const TestResult timeout = runner.runTestCase(test_case);
    require(timeout.status == TestStatus::Timeout, "timeout mode mismatch");

    backend->setMode(BackendBehaviorMode::Regression);
    test_case.name = "Regression Test";
    Snapshot baseline;
    baseline.capability_states["ray_tracing"] = CapabilityState::Unsupported;
    test_case.baseline_snapshot = baseline;
    const TestResult regression = runner.runTestCase(test_case);
    require(regression.status == TestStatus::Regression, "regression mode mismatch");

    backend->setMode(BackendBehaviorMode::NoChange);
    test_case.name = "NoChange Test";
    test_case.baseline_snapshot.reset();
    const TestResult no_change = runner.runTestCase(test_case);
    require(no_change.status == TestStatus::NoChange, "no-change mode mismatch");

    backend->setMode(BackendBehaviorMode::Failure);
    test_case.name = "Failure Test";
    const TestResult failure = runner.runTestCase(test_case);
    require(failure.status == TestStatus::Fail, "failure mode mismatch");

    backend->setMode(BackendBehaviorMode::Success);
    test_case.name = "Success Test";
    const TestResult success = runner.runTestCase(test_case);
    require(success.status == TestStatus::Pass, "success mode mismatch");
}

void testRunnerGuaranteesCleanupOnException() {
    class ThrowingBackend final : public DefaultMockBackend {
    public:
        bool cleaned_up{false};

        void teardownEnvironment() noexcept override {
            cleaned_up = true;
            DefaultMockBackend::teardownEnvironment();
        }

        BackendExecutionResult executeWorkload(
            const VerificationConfig&, std::chrono::milliseconds) override {
            throw std::runtime_error("Simulated execution crash");
        }
    };

    auto backend = std::make_shared<ThrowingBackend>();
    RegressionRunner runner(backend);

    TestCase test_case;
    test_case.name = "Cleanup Guarantee Test";

    const TestResult result = runner.runTestCase(test_case);
    require(result.status == TestStatus::Fail, "execution exception must become failure");
    require(backend->cleaned_up, "teardown must run after execution exception");
}

void testAssertions() {
    auto backend = std::make_shared<DefaultMockBackend>();
    RegressionRunner runner(backend);

    TestCase test_case;
    test_case.name = "Assertion Test";
    test_case.assertions.push_back({"environment_data.os", "Linux"});
    test_case.assertions.push_back({"capability_states.ray_tracing", "unsupported"});

    const TestResult result = runner.runTestCase(test_case);
    require(result.status == TestStatus::Pass, "valid assertions must pass");
}

void testDeterministicJsonOutputAndEscaping() {
    Snapshot snap;
    snap.environment_data["b"] = "val2";
    snap.environment_data["a"] = "val1\n\"quoted\"";
    snap.gpu_data["z"] = "100";
    snap.gpu_data["a"] = "200";

    const std::string json1 = serializeSnapshotToDeterministicJson(snap);
    const std::string json2 = serializeSnapshotToDeterministicJson(snap);

    require(json1 == json2, "serialization must be deterministic");
    require(json1.find("\"a\": \"val1\\n\\\"quoted\\\"\"")
                < json1.find("\"b\": \"val2\""),
            "map keys must be sorted");
    require(json1.find("\\n") != std::string::npos, "newline must be escaped");
}

void testSuiteAccounting() {
    auto backend = std::make_shared<DefaultMockBackend>();
    RegressionRunner runner(backend);

    TestSuite suite;
    suite.name = "Accounting";
    suite.test_cases.resize(2);
    suite.test_cases[0].name = "pass";
    suite.test_cases[1].name = "no-change";

    backend->setMode(BackendBehaviorMode::Success);
    const TestSuiteResult first = runner.runTestSuite(suite);
    require(first.passed_count == 2, "suite pass count mismatch");

    backend->setMode(BackendBehaviorMode::NoChange);
    const TestSuiteResult second = runner.runTestSuite(suite);
    require(second.no_change_count == 2, "suite no-change count mismatch");
}

} // namespace

int main() {
    try {
        testSnapshotDiffAllCategories();
        testSnapshotDiffDetectsAddedAndRemovedKeys();
        testBackendModesAndUnknownStatus();
        testRunnerGuaranteesCleanupOnException();
        testAssertions();
        testDeterministicJsonOutputAndEscaping();
        testSuiteAccounting();
    } catch (const std::exception& e) {
        std::cerr << "Verification test failure: " << e.what() << '\n';
        return 1;
    }

    std::cout << "All Verification & Regression Harness P0 tests passed successfully.\n";
    return 0;
}
