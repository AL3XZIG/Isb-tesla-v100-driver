#pragma once

#include "isb/cal/capabilities.hpp"

#include <optional>
#include <string>

namespace isb::graphics {

/// Normalized, observation-based graphics and compute capability snapshot.
struct GraphicsCapability {
    cal::CapabilityState cuda = cal::CapabilityState::Unknown;
    cal::CapabilityState vulkan = cal::CapabilityState::Unknown;
    cal::CapabilityState directx_compute = cal::CapabilityState::Unknown;
    cal::CapabilityState tensor_cores = cal::CapabilityState::Unknown;
    cal::CapabilityState rt_cores = cal::CapabilityState::Unknown;
    cal::CapabilityState optical_flow = cal::CapabilityState::Unknown;
    std::optional<cal::ComputeCapability> sm_version;
};

std::string to_json(const GraphicsCapability& capability);

} // namespace isb::graphics
