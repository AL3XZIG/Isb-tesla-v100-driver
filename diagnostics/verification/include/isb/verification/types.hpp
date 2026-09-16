#ifndef ISB_VERIFICATION_TYPES_HPP
#define ISB_VERIFICATION_TYPES_HPP

#include <chrono>
#include <cstddef>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace isb::verification {

enum class CapabilityState {
    Supported,
    Unsupported,
    Unknown,
    Disabled
};

std::string capabilityStateToString(CapabilityState state);

enum class BackendBehaviorMode {
    Success,
    NoChange,
    Regression,
    Unknown,
    Timeout,
    Failure
};

enum class TestStatus {
    Pass,
    Fail,
    Regression,
    NoChange,
    Unknown,
    Timeout
};

std::string testStatusToString(TestStatus status);

struct VerificationConfig {};

struct Snapshot {
    std::map<std::string, std::string> environment_data;
    std::map<std::string, std::string> gpu_data;
    std::map<std::string, std::string> api_data;
    std::map<std::string, CapabilityState> capability_states;
    std::map<std::string, std::string> metadata;
};

struct BackendExecutionResult {
    BackendBehaviorMode behavior{BackendBehaviorMode::Success};
    bool success{false};
    std::string error_message;
    std::map<std::string, double> metrics;
    Snapshot captured_snapshot{};
};

struct SnapshotDiff {
    struct FieldDiff {
        std::string baseline;
        std::string actual;
    };

    bool has_differences{false};
    std::map<std::string, FieldDiff> environment_diffs;
    std::map<std::string, FieldDiff> gpu_diffs;
    std::map<std::string, FieldDiff> api_data_diffs;
    std::map<std::string, FieldDiff> capability_diffs;
    std::map<std::string, FieldDiff> metadata_diffs;
};

struct Assertion {
    std::string field;
    std::string expected_value;
};

struct AssertionResult {
    bool passed{false};
    std::string message;
};

struct TestCase {
    std::string name;
    VerificationConfig config;
    std::chrono::milliseconds timeout{0};
    std::vector<Assertion> assertions;
    std::optional<Snapshot> baseline_snapshot;
};

struct TestResult {
    std::string test_name;
    TestStatus status{TestStatus::Fail};
    std::string failure_reason;
    Snapshot actual_snapshot;
};

struct TestSuite {
    std::string name;
    std::vector<TestCase> test_cases;
};

struct TestSuiteResult {
    std::string suite_name;
    std::vector<TestResult> results;
    std::size_t passed_count{0};
    std::size_t failed_count{0};
    std::size_t regression_count{0};
    std::size_t no_change_count{0};
    std::size_t unknown_count{0};
    std::size_t timeout_count{0};
};

} // namespace isb::verification

#endif // ISB_VERIFICATION_TYPES_HPP
