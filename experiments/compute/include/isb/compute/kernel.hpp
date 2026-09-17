#pragma once

#include "isb/cal/capabilities.hpp"
#include "isb/compute/device.hpp"

namespace isb::compute {

/// Capability-only kernel precision matrix. No kernels are compiled or run.
struct KernelCapability {
    cal::CapabilityState fp32 = cal::CapabilityState::Unknown;
    cal::CapabilityState fp16 = cal::CapabilityState::Unknown;
    cal::CapabilityState int8 = cal::CapabilityState::Unknown;
    cal::CapabilityState tensor_fp16 = cal::CapabilityState::Unknown;
};

KernelCapability kernel_capability(const ComputeDevice& device) noexcept;

} // namespace isb::compute
