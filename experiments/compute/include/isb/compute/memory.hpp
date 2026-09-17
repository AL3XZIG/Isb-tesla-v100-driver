#pragma once

#include <cstdint>

namespace isb::compute {

/// Observed device-memory quantities in bytes. This type does not allocate or
/// otherwise manage device memory.
struct DeviceMemoryInfo {
    std::uint64_t total = 0;
    std::uint64_t free = 0;
    std::uint64_t used = 0;
};

} // namespace isb::compute
