#pragma once

#include "isb/cal/capabilities.hpp"
#include "isb/compute/memory.hpp"

#include <optional>
#include <string>

namespace isb::compute {

/// Provider-neutral compute view derived from CAL observations.
///
/// The optional memory observation is intentionally supplied separately: CAL
/// v1 does not contain memory telemetry, and this experimental layer does not
/// probe CUDA or NVML on its own.
class ComputeDevice {
public:
    ComputeDevice() = default;

    static ComputeDevice from_capabilities(
        const cal::GpuCapabilities& capabilities,
        std::optional<DeviceMemoryInfo> memory = std::nullopt);

    const std::string& name() const noexcept;
    const std::optional<cal::ComputeCapability>& sm_version() const noexcept;
    const std::optional<DeviceMemoryInfo>& memory() const noexcept;
    cal::CapabilityState tensor_capability() const noexcept;
    cal::CapabilityState cuda_availability() const noexcept;
    cal::CapabilityState ray_tracing_capability() const noexcept;

private:
    std::string name_;
    std::optional<cal::ComputeCapability> sm_version_;
    std::optional<DeviceMemoryInfo> memory_;
    cal::CapabilityState tensor_capability_ = cal::CapabilityState::Unknown;
    cal::CapabilityState cuda_availability_ = cal::CapabilityState::Unknown;
    cal::CapabilityState ray_tracing_capability_ = cal::CapabilityState::Unknown;
};

} // namespace isb::compute
