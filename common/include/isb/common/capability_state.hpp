#pragma once

namespace isb::common {

enum class CapabilityState {
    Unknown,
    Available,
    Unavailable,
    PermissionDenied,
    Error,
    Partial,        ///< Partially available/functional
    Ready,          ///< Ready to apply/configure
    Unsupported     ///< Not supported on this system
};

const char* to_string(CapabilityState state) noexcept;

} // namespace isb::common
