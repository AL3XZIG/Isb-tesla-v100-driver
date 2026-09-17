#include "isb/compute/context.hpp"
#include "isb/compute/device.hpp"
#include "isb/compute/kernel.hpp"


namespace isb::compute {

ComputeDevice ComputeDevice::from_capabilities(const cal::GpuCapabilities& capabilities,
                                                std::optional<DeviceMemoryInfo> memory) {
    ComputeDevice device;
    device.name_ = capabilities.identity.model_name;
    device.sm_version_ = capabilities.compute.compute_capability
        ? capabilities.compute.compute_capability
        : capabilities.identity.compute_capability;
    device.memory_ = memory;
    device.cuda_availability_ = capabilities.compute.cuda_state;
    device.ray_tracing_capability_ = capabilities.hardware.rt_cores.state;

    // The hardware observation is authoritative for physical Tensor Core
    // presence. Retain an unavailable runtime observation when hardware is
    // otherwise unknown, so reports do not claim usable tensor execution.
    device.tensor_capability_ = capabilities.hardware.tensor_cores.state;
    if (device.tensor_capability_ == cal::CapabilityState::Unknown) {
        device.tensor_capability_ = capabilities.compute.tensor_cores.state;
    }
    return device;
}

const std::string& ComputeDevice::name() const noexcept { return name_; }
const std::optional<cal::ComputeCapability>& ComputeDevice::sm_version() const noexcept { return sm_version_; }
const std::optional<DeviceMemoryInfo>& ComputeDevice::memory() const noexcept { return memory_; }
cal::CapabilityState ComputeDevice::tensor_capability() const noexcept { return tensor_capability_; }
cal::CapabilityState ComputeDevice::cuda_availability() const noexcept { return cuda_availability_; }
cal::CapabilityState ComputeDevice::ray_tracing_capability() const noexcept { return ray_tracing_capability_; }

ComputeContext::ComputeContext(Backend backend, cal::CapabilityState availability) noexcept
    : backend_(backend), availability_(availability) {}

bool ComputeContext::initialize() noexcept {
    initialized_ = availability_ == cal::CapabilityState::Available;
    return initialized_;
}

bool ComputeContext::available() const noexcept {
    return initialized_;
}

void ComputeContext::shutdown() noexcept {
    initialized_ = false;
}

Backend ComputeContext::backend() const noexcept { return backend_; }

KernelCapability kernel_capability(const ComputeDevice& device) noexcept {
    KernelCapability result;
    const auto cuda = device.cuda_availability();
    if (cuda != cal::CapabilityState::Available) {
        result.fp32 = cuda;
        result.fp16 = cuda;
        result.int8 = cuda;
        result.tensor_fp16 = cuda;
        return result;
    }

    result.fp32 = cal::CapabilityState::Available;
    result.fp16 = cal::CapabilityState::Available;
    // Volta exposes INT8 CUDA instructions, but this intentionally does not
    // claim an INT8 Tensor Core execution mode.
    result.int8 = cal::CapabilityState::Available;
    result.tensor_fp16 = device.tensor_capability();
    return result;
}

} // namespace isb::compute
