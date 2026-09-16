#include "isb/verification/assertions.hpp"

namespace isb::verification {
namespace {

bool readSnapshotField(const Snapshot& snapshot, const std::string& field, std::string& value) {
    const auto dot = field.find('.');
    if (dot == std::string::npos) {
        return false;
    }

    const std::string category = field.substr(0, dot);
    const std::string key = field.substr(dot + 1);

    if (category == "environment_data") {
        const auto it = snapshot.environment_data.find(key);
        if (it == snapshot.environment_data.end()) return false;
        value = it->second;
        return true;
    }
    if (category == "gpu_data") {
        const auto it = snapshot.gpu_data.find(key);
        if (it == snapshot.gpu_data.end()) return false;
        value = it->second;
        return true;
    }
    if (category == "api_data") {
        const auto it = snapshot.api_data.find(key);
        if (it == snapshot.api_data.end()) return false;
        value = it->second;
        return true;
    }
    if (category == "capability_states") {
        const auto it = snapshot.capability_states.find(key);
        if (it == snapshot.capability_states.end()) return false;
        value = capabilityStateToString(it->second);
        return true;
    }
    if (category == "metadata") {
        const auto it = snapshot.metadata.find(key);
        if (it == snapshot.metadata.end()) return false;
        value = it->second;
        return true;
    }

    return false;
}

} // namespace

AssertionResult evaluateAssertion(
    const Assertion& assertion,
    const BackendExecutionResult& execution,
    const Snapshot& snapshot) {
    (void)execution;

    std::string actual;
    if (!readSnapshotField(snapshot, assertion.field, actual)) {
        return {false, "Snapshot field not found: " + assertion.field};
    }

    if (actual != assertion.expected_value) {
        return {
            false,
            "Expected " + assertion.field + " = " + assertion.expected_value
                + ", got " + actual};
    }

    return {true, "Assertion passed"};
}

} // namespace isb::verification
