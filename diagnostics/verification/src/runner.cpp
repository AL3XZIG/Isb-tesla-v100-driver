#include "isb/verification/runner.hpp"

#include <stdexcept>
#include <thread>
#include <utility>

namespace isb::verification {

DefaultMockBackend::DefaultMockBackend(BackendBehaviorMode initial_mode)
    : mode_(initial_mode) {}

void DefaultMockBackend::setMode(BackendBehaviorMode mode) noexcept {
    mode_ = mode;
}

BackendBehaviorMode DefaultMockBackend::getMode() const noexcept {
    return mode_;
}

void DefaultMockBackend::setFailureReason(std::string reason) {
    failure_reason_ = std::move(reason);
}

void DefaultMockBackend::setupEnvironment(const VerificationConfig& config) {
    (void)config;
    environment_setup_ = true;

    mock_snapshot_ = Snapshot{};
    mock_snapshot_.environment_data["driver_version"] = "550.54.14";
    mock_snapshot_.environment_data["os"] = "Linux";
    mock_snapshot_.gpu_data["clock_mhz"] = "1500";
    mock_snapshot_.gpu_data["memory_used_mb"] = "1024";
    mock_snapshot_.api_data["vk_version"] = "1.3.280";

    // These are deliberately conservative mock observations. The mock backend
    // must not imply unsupported V100 hardware features are actually present.
    mock_snapshot_.capability_states["ray_tracing"] = CapabilityState::Unsupported;
    mock_snapshot_.capability_states["mesh_shaders"] = CapabilityState::Unknown;
    mock_snapshot_.metadata["backend"] = "DefaultMockBackend";
}

void DefaultMockBackend::teardownEnvironment() noexcept {
    environment_setup_ = false;
}

BackendExecutionResult DefaultMockBackend::executeWorkload(
    const VerificationConfig& config,
    std::chrono::milliseconds timeout) {
    (void)config;

    BackendExecutionResult result;
    result.behavior = mode_;

    if (!environment_setup_) {
        result.behavior = BackendBehaviorMode::Failure;
        result.success = false;
        result.error_message = "Environment not initialized before workload execution";
        result.captured_snapshot = captureSnapshot();
        return result;
    }

    switch (mode_) {
        case BackendBehaviorMode::Success:
            result.success = true;
            result.metrics["fps"] = 120.0;
            result.metrics["frame_time_ms"] = 8.33;
            break;

        case BackendBehaviorMode::NoChange:
            result.success = true;
            result.metrics["fps"] = 100.0;
            result.metrics["frame_time_ms"] = 10.0;
            break;

        case BackendBehaviorMode::Regression:
            result.success = true;
            result.metrics["fps"] = 45.0;
            result.metrics["frame_time_ms"] = 22.22;
            mock_snapshot_.capability_states["ray_tracing"] = CapabilityState::Disabled;
            break;

        case BackendBehaviorMode::Unknown:
            result.success = false;
            result.error_message = "Backend state indeterminate / unknown device response";
            break;

        case BackendBehaviorMode::Timeout:
            std::this_thread::sleep_for(timeout + std::chrono::milliseconds(10));
            result.success = false;
            result.error_message =
                "Workload execution timed out after " + std::to_string(timeout.count()) + "ms";
            break;

        case BackendBehaviorMode::Failure:
            result.success = false;
            result.error_message = failure_reason_.empty()
                ? "Backend explicit execution failure"
                : failure_reason_;
            break;
    }

    result.captured_snapshot = captureSnapshot();
    return result;
}

Snapshot DefaultMockBackend::captureSnapshot() {
    return mock_snapshot_;
}

RegressionRunner::RegressionRunner(std::shared_ptr<MockBackend> backend)
    : backend_(std::move(backend)) {}

TestResult RegressionRunner::runTestCase(const TestCase& test_case) {
    TestResult result;
    result.test_name = test_case.name;

    if (!backend_) {
        result.status = TestStatus::Fail;
        result.failure_reason = "Backend pointer is null";
        return result;
    }

    try {
        backend_->setupEnvironment(test_case.config);
    } catch (const std::exception& e) {
        result.status = TestStatus::Fail;
        result.failure_reason = std::string("Setup exception: ") + e.what();
        return result;
    } catch (...) {
        result.status = TestStatus::Fail;
        result.failure_reason = "Unknown exception during setup";
        return result;
    }

    ScopeGuard teardown_guard([this]() noexcept {
        try {
            backend_->teardownEnvironment();
        } catch (...) {
            // MockBackend promises noexcept teardown; retain the guard boundary
            // so a broken implementation cannot escape during stack unwinding.
        }
    });

    BackendExecutionResult exec_result;
    try {
        exec_result = backend_->executeWorkload(test_case.config, test_case.timeout);
    } catch (const std::exception& e) {
        result.status = TestStatus::Fail;
        result.failure_reason = std::string("Execution exception: ") + e.what();
        return result;
    } catch (...) {
        result.status = TestStatus::Fail;
        result.failure_reason = "Unknown exception during execution";
        return result;
    }

    result.actual_snapshot = exec_result.captured_snapshot;

    if (exec_result.behavior == BackendBehaviorMode::Unknown) {
        result.status = TestStatus::Unknown;
        result.failure_reason = exec_result.error_message;
        return result;
    }

    if (exec_result.behavior == BackendBehaviorMode::Timeout) {
        result.status = TestStatus::Timeout;
        result.failure_reason = exec_result.error_message;
        return result;
    }

    if (exec_result.behavior == BackendBehaviorMode::Failure || !exec_result.success) {
        result.status = TestStatus::Fail;
        result.failure_reason = exec_result.error_message;
        return result;
    }

    if (test_case.baseline_snapshot.has_value()) {
        const SnapshotDiff diff = compareSnapshots(
            *test_case.baseline_snapshot, result.actual_snapshot);
        if (diff.has_differences) {
            result.status = TestStatus::Regression;
            result.failure_reason = "Snapshot differences detected against baseline";
            return result;
        }
    }

    for (const auto& assertion : test_case.assertions) {
        const AssertionResult assertion_result = evaluateAssertion(
            assertion, exec_result, result.actual_snapshot);
        if (!assertion_result.passed) {
            result.status = TestStatus::Fail;
            result.failure_reason = "Assertion failed: " + assertion_result.message;
            return result;
        }
    }

    result.status = exec_result.behavior == BackendBehaviorMode::NoChange
        ? TestStatus::NoChange
        : TestStatus::Pass;
    return result;
}

TestSuiteResult RegressionRunner::runTestSuite(const TestSuite& test_suite) {
    TestSuiteResult suite_result;
    suite_result.suite_name = test_suite.name;

    for (const auto& test_case : test_suite.test_cases) {
        TestResult result = runTestCase(test_case);

        switch (result.status) {
            case TestStatus::Pass: ++suite_result.passed_count; break;
            case TestStatus::Fail: ++suite_result.failed_count; break;
            case TestStatus::Regression: ++suite_result.regression_count; break;
            case TestStatus::NoChange: ++suite_result.no_change_count; break;
            case TestStatus::Unknown: ++suite_result.unknown_count; break;
            case TestStatus::Timeout: ++suite_result.timeout_count; break;
        }

        suite_result.results.push_back(std::move(result));
    }

    return suite_result;
}

} // namespace isb::verification
