#include "isb/cal/capabilities.hpp"
#include "isb/cal/validation.hpp"

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
    result.compute.tensor_precisions =
        TensorPrecisionSet{CapabilityState::Available, {TensorPrecision::FP16}};

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

void test_validation_accepts_valid_v100() {
    const auto result = validate(make_v100(GpuVariant::V100_SXM2));
    assert(result.valid());
}

void test_validation_rejects_conflicting_compute_capability() {
    auto capabilities = make_v100(GpuVariant::V100_SXM2);
    capabilities.compute.compute_capability = ComputeCapability{8, 0};

    const auto result = validate(capabilities);
    assert(!result.valid());

    bool found = false;
    for (const auto& issue : result.issues) {
        if (issue.code == "CAL-VAL-001" || issue.code == "CAL-V100-003") {
            found = true;
        }
    }
    assert(found);
}

void test_validation_rejects_v100_rt_cores() {
    auto capabilities = make_v100(GpuVariant::V100_SXM2);
    capabilities.hardware.rt_cores.state = CapabilityState::Available;

    const auto result = validate(capabilities);
    assert(!result.valid());

    bool found = false;
    for (const auto& issue : result.issues) {
        if (issue.code == "CAL-V100-005") {
            found = true;
        }
    }
    assert(found);
}

void test_validation_rejects_inconsistent_tensor_state() {
    auto capabilities = make_v100(GpuVariant::V100_SXM2);
    capabilities.hardware.tensor_cores.state = CapabilityState::Available;
    capabilities.compute.tensor_cores.state = CapabilityState::Unavailable;

    const auto result = validate(capabilities);
    assert(!result.valid());
}

void test_validation_rejects_later_generation_precision_claims() {
    auto capabilities = make_v100(GpuVariant::V100_PCIe);
    capabilities.compute.tensor_precisions.values.push_back(TensorPrecision::INT8);

    const auto result = validate(capabilities);
    assert(!result.valid());
}

void test_validation_accepts_unknown_profile() {
    const GpuCapabilities capabilities;
    const auto result = validate(capabilities);
    assert(result.valid());
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

    const std::string unknown_json = to_json(GpuCapabilities{});
    assert(unknown_json.find("\"state\":\"unknown\"") != std::string::npos);
    assert(unknown_json.find("\"variant\":\"unknown\"") != std::string::npos);
}

} // namespace

int main() {
    test_empty_capabilities();
    test_capability_states();
    test_v100_variants();
    test_v100_feature_states();
    test_validation_accepts_valid_v100();
    test_validation_rejects_conflicting_compute_capability();
    test_validation_rejects_v100_rt_cores();
    test_validation_rejects_inconsistent_tensor_state();
    test_validation_rejects_later_generation_precision_claims();
    test_validation_accepts_unknown_profile();
    test_json_serialization();
    return 0;
}
