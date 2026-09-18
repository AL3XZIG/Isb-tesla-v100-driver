#include "isb/cal/v100.hpp"

namespace isb::cal {

GpuCapabilities make_v100_capabilities(GpuVariant variant) {
    GpuCapabilities result;

    result.identity.vendor = "NVIDIA";
    result.identity.architecture = "Volta";
    result.identity.compute_capability = ComputeCapability{7, 0};
    result.identity.variant = variant;
    result.identity.exact_hardware_variant =
        variant == GpuVariant::Unknown ? "" : to_string(variant);
    result.identity.model_name = "Tesla V100";

    result.compute.compute_capability = ComputeCapability{7, 0};
    result.compute.cuda_cores = 5120;

    // Volta V100 contains first-generation Tensor Cores. CAL records FP16 as
    // the baseline precision supported by this hardware generation here;
    // runtime/library exposure remains provider evidence.
    result.compute.tensor_cores = TensorCores{CapabilityState::Available, 1};
    result.compute.tensor_precisions =
        TensorPrecisionSet{CapabilityState::Available, {TensorPrecision::FP16}};

    result.hardware.tensor_cores = TensorCores{CapabilityState::Available, 1};
    result.hardware.hbm2.state = CapabilityState::Available;
    result.hardware.ecc.state = CapabilityState::Available;
    result.hardware.rt_cores.state = CapabilityState::Unavailable;
    result.hardware.optical_flow_accelerator.state = CapabilityState::Unavailable;
    result.hardware.mig.state = CapabilityState::Unavailable;
    result.hardware.display_outputs.state = CapabilityState::Unavailable;

    // NVLink topology is configuration-dependent and must be established by
    // the provider rather than inferred from the generic V100 baseline.
    result.hardware.nvlink.state = CapabilityState::Unknown;

    return result;
}

} // namespace isb::cal
