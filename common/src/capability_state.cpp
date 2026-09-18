#include "isb/common/capability_state.hpp"

namespace isb::common {

const char* to_string(CapabilityState state) noexcept {
    switch (state) {
    case CapabilityState::Unknown: return "unknown";
    case CapabilityState::Available: return "available";
    case CapabilityState::Unavailable: return "unavailable";
    case CapabilityState::PermissionDenied: return "permission_denied";
    case CapabilityState::Error: return "error";
    case CapabilityState::Partial: return "partial";
    case CapabilityState::Ready: return "ready";
    case CapabilityState::Unsupported: return "unsupported";
    }
    return "unknown";
}

} // namespace isb::common
