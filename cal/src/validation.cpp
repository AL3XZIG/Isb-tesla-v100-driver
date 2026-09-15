#include "isb/cal/validation.hpp"

#include <algorithm>
#include <utility>

namespace isb::cal {
namespace {

void add(ValidationResult& result, std::string code, std::string message) {
    result.issues.push_back(ValidationIssue{std::move(code), ValidationSeverity::Error, std::move(message)});
}

bool has_precision(const TensorPrecisionSet& set, TensorPrecision precision) {
    return std::find(set.values.begin(), set.values.end(), precision) != set.values.end();
}

bool is_v100(GpuVariant variant) {
    return variant == GpuVariant::V100_SXM2 || variant == GpuVariant::V100_PCIe;
}

} // namespace

ValidationResult validate(const GpuCapabilities& c) {
    ValidationResult result;

    if (c.identity.compute_capability && c.compute.compute_capability &&
        (c.identity.compute_capability->major != c.compute.compute_capability->major ||
         c.identity.compute_capability->minor != c.compute.compute_capability->minor)) {
        add(result, "CAL-VAL-001", "identity and compute compute-capability values disagree");
    }

    if (c.compute.tensor_cores.state == CapabilityState::Unavailable &&
        c.hardware.tensor_cores.state == CapabilityState::Available) {
        add(result, "CAL-VAL-002", "compute Tensor Cores cannot be unavailable when hardware Tensor Cores are available");
    }

    if (c.hardware.tensor_cores.state == CapabilityState::Unavailable &&
        c.hardware.tensor_cores.generation.has_value()) {
        add(result, "CAL-VAL-003", "an unavailable Tensor Core capability must not specify a generation");
    }

    if (c.compute.tensor_precisions.state == CapabilityState::Unavailable &&
        !c.compute.tensor_precisions.values.empty()) {
        add(result, "CAL-VAL-004", "an unavailable tensor-precision set must not contain supported precisions");
    }

    if (!is_v100(c.identity.variant)) {
        return result;
    }

    if (!c.identity.architecture.empty() && c.identity.architecture != "Volta") {
        add(result, "CAL-V100-001", "V100 must identify the Volta architecture when architecture is populated");
    }

    if (c.identity.compute_capability &&
        (c.identity.compute_capability->major != 7 || c.identity.compute_capability->minor != 0)) {
        add(result, "CAL-V100-002", "V100 compute capability must be 7.0");
    }

    if (c.compute.compute_capability &&
        (c.compute.compute_capability->major != 7 || c.compute.compute_capability->minor != 0)) {
        add(result, "CAL-V100-003", "V100 compute capability observation must be 7.0");
    }

    if (c.compute.cuda_cores && *c.compute.cuda_cores != 5120) {
        add(result, "CAL-V100-004", "V100 CUDA core count must be 5120 when populated");
    }

    if (c.hardware.rt_cores.state == CapabilityState::Available) {
        add(result, "CAL-V100-005", "V100 does not provide RT cores");
    }

    if (c.hardware.optical_flow_accelerator.state == CapabilityState::Available) {
        add(result, "CAL-V100-006", "V100 does not provide a dedicated Optical Flow Accelerator");
    }

    if (c.hardware.mig.state == CapabilityState::Available) {
        add(result, "CAL-V100-007", "V100 does not provide MIG");
    }

    if (c.hardware.tensor_cores.state == CapabilityState::Available &&
        c.hardware.tensor_cores.generation && *c.hardware.tensor_cores.generation != 1) {
        add(result, "CAL-V100-008", "V100 Tensor Cores must be generation 1 when generation is populated");
    }

    if (c.compute.tensor_cores.state == CapabilityState::Available &&
        c.compute.tensor_cores.generation && *c.compute.tensor_cores.generation != 1) {
        add(result, "CAL-V100-009", "V100 compute Tensor Cores must be generation 1 when generation is populated");
    }

    // Volta Tensor Cores are FP16-oriented. Do not infer INT8/INT4 Tensor Core
    // support from their presence; those precisions belong to later generations.
    if (has_precision(c.compute.tensor_precisions, TensorPrecision::INT4)) {
        add(result, "CAL-V100-010", "V100 Tensor Core precision set must not claim INT4 support");
    }

    if (has_precision(c.compute.tensor_precisions, TensorPrecision::INT8)) {
        add(result, "CAL-V100-011", "V100 Tensor Core precision set must not claim INT8 support");
    }

    return result;
}

} // namespace isb::cal
