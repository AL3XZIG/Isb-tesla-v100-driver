#include "isb/common/capability_state.hpp"

#include <cassert>
#include <string>

int main() {
    using isb::common::CapabilityState;
    using isb::common::to_string;

    assert(std::string(to_string(CapabilityState::Unknown)) == "unknown");
    assert(std::string(to_string(CapabilityState::Available)) == "available");
    assert(std::string(to_string(CapabilityState::Unavailable)) == "unavailable");
    assert(std::string(to_string(CapabilityState::PermissionDenied)) == "permission_denied");
    assert(std::string(to_string(CapabilityState::Error)) == "error");

    // Defensive fallback for invalid enum values crossing an ABI or data boundary.
    assert(std::string(to_string(static_cast<CapabilityState>(0xffU))) == "unknown");
    return 0;
}
