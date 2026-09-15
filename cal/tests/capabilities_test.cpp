#include "isb/cal/capabilities.hpp"

#include <cassert>
#include <string>

using namespace isb::cal;

namespace {

void test_empty_capabilities() {
    const GpuCapabilities capabilities;
    assert(capabilities.identity.variant == GpuVariant::Unknown);
    assert(capabilities.identity.vendor.empty());
    assert(capabilities.compute.cuda_state == CapabilityState::Unknown);
    assert(capabilities.graphics.vulkan_state == CapabilityState::Unknown);
    assert(capabilities.hardware.rt_cores.state == CapabilityState::Unknown);
}

void test_capability_states() {
    Capability capability;

    capability.state = CapabilityState::Unknown;
    assert(capability.state == CapabilityState::Unknown);

    capability.state = CapabilityState::Available;
    assert(capability.state == CapabilityState::Available);

    capability.state = CapabilityState::Unavailable;
    assert(capability.state == CapabilityState::Unavailable);
}

GpuCapabilities make_v100(GpuVariant variant) {
    GpuCapabilities result;
    result.identity.vendor = "NVIDIA";
    result.identity.architecture = "Volta";
    result.identity.compute_capability = ComputeCapability{7, 0};
    result.identity.variant = variant;
    result.identity.exact_hardware_variant = to_string(variant);
    result.identity.model_name = "Tesla V100";

    result.compute.cuda_state = CapabilityState::Available;
    result.compute.compute_capability = ComputeCapability{7, 0};
    result.compute.cuda_cores = 5120;
    result.compute.tensor_cores = TensorCores{CapabilityState::Available, 1};
    result.compute.tensor_precisions = TensorPrecisionSet{
        CapabilityState::Available,
        {TensorPrecision::FP16, TensorPrecision::INT8, TensorPrecision::INT4}
    };

    result.hardware.tensor_cores = TensorCores{CapabilityState::Available, 1};
    result.hardware.rt_cores.state = CapabilityState::Unavailable;
    result.hardware.optical_flow_accelerator.state = CapabilityState::Unavailable;
    result.hardware.mig.state = CapabilityState::Unavailable;
    result.hardware.hbm2.state = CapabilityState::Available;
    result.hardware.ecc.state = CapabilityState::Unknown;
    result.hardware.nvlink.state = CapabilityState::Unknown;
    result.hardware.display_outputs.state = CapabilityState::Unavailable;

    return result;
}

void test_v100_variants() {
    const auto sxm2 = make_v100(GpuVariant::V100_SXM2);
    const auto pcie = make_v100(GpuVariant::V100_PCIe);

    assert(sxm2.identity.variant == GpuVariant::V100_SXM2);
    assert(pcie.identity.variant == GpuVariant::V100_PCIe);
    assert(sxm2.identity.variant != pcie.identity.variant);
    assert(sxm2.identity.exact_hardware_variant != pcie.identity.exact_hardware_variant);
}

void test_v100_feature_states() {
    const auto capabilities = make_v100(GpuVariant::V100_SXM2);

    assert(capabilities.compute.tensor_cores.state == CapabilityState::Available);
    assert(capabilities.compute.tensor_cores.generation.has_value());
    assert(*capabilities.compute.tensor_cores.generation == 1);

    assert(capabilities.hardware.tensor_cores.state == CapabilityState::Available);
    assert(capabilities.hardware.tensor_cores.generation.has_value());
    assert(*capabilities.hardware.tensor_cores.generation == 1);

    assert(capabilities.hardware.rt_cores.state == CapabilityState::Unavailable);
    assert(capabilities.hardware.optical_flow_accelerator.state == CapabilityState::Unavailable);
    assert(capabilities.hardware.mig.state == CapabilityState::Unavailable);
}

void test_json_serialization() {
    auto capabilities = make_v100(GpuVariant::V100_SXM2);
    capabilities.identity.model_name = "Tesla V100 \"SXM2\"";

    const std::string json = to_json(capabilities);

    assert(!json.empty());
    assert(json.front() == '{');
    assert(json.back() == '}');
    assert(json.find("\"variant\":\"V100 SXM2\"") != std::string::npos);
    assert(json.find("\"tensor_cores\":{\"state\":\"available\",\"generation\":1}") != std::string::npos);
    assert(json.find("\"rt_cores\":{\"state\":\"unavailable\"}") != std::string::npos);
    assert(json.find("\"optical_flow_accelerator\":{\"state\":\"unavailable\"}") != std::string::npos);
    assert(json.find("\"mig\":{\"state\":\"unavailable\"}") != std::string::npos);
    assert(json.find("Tesla V100 \\\"SXM2\\\"") != std::string::npos);

    GpuCapabilities unknown;
    const std::string unknown_json = to_json(unknown);
    assert(unknown_json.find("\"state\":\"unknown\"") != std::string::npos);
    assert(unknown_json.find("\"variant\":\"unknown\"") != std::string::npos);
}

} // namespace

int main() {
    test_empty_capabilities();
    test_capability_states();
    test_v100_variants();
    test_v100_feature_states();
    test_json_serialization();
    return 0;
}
