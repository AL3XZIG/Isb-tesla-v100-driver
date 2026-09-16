#include "isb/verification/types.hpp"

namespace isb::verification {

std::string capabilityStateToString(CapabilityState state) {
    switch (state) {
        case CapabilityState::Supported: return "supported";
        case CapabilityState::Unsupported: return "unsupported";
        case CapabilityState::Unknown: return "unknown";
        case CapabilityState::Disabled: return "disabled";
    }
    return "unknown";
}

std::string testStatusToString(TestStatus status) {
    switch (status) {
        case TestStatus::Pass: return "pass";
        case TestStatus::Fail: return "fail";
        case TestStatus::Regression: return "regression";
        case TestStatus::NoChange: return "no_change";
        case TestStatus::Unknown: return "unknown";
        case TestStatus::Timeout: return "timeout";
    }
    return "unknown";
}

} // namespace isb::verification
