#pragma once

namespace isb::common {

enum class CapabilityState {
    Unknown,
    Available,
    Unavailable,
    PermissionDenied,
    Error,
};

const char* to_string(CapabilityState state) noexcept;

} // namespace isb::common
