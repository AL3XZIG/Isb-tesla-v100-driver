#pragma once

#include "isb/common/capability_state.hpp"

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace isb::cal {

/// CAL compatibility alias for the provider-neutral capability state contract.
/// Unknown is deliberately distinct from Unavailable.
using CapabilityState = common::CapabilityState;

/// Hardware form factor / configuration. Variants are never implicitly equivalent.
enum class GpuVariant {
    Unknown,
    V100_SXM2,
    V100_PCIe,
};

enum class TensorPrecision {
    FP16,
    BF16,
    TF32,
    INT8,
    INT4,
};

struct Version {
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
    std::uint32_t patch = 0;
};

struct ComputeCapability {
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
};

struct Capability {
    CapabilityState state = CapabilityState::Unknown;
};

struct TensorCores {
    CapabilityState state = CapabilityState::Unknown;
    std::optional<std::uint32_t> generation;
};

struct TensorPrecisionSet {
    CapabilityState state = CapabilityState::Unknown;
    std::vector<TensorPrecision> values;
};

struct GpuIdentity {
    std::string vendor;
    std::string architecture;
    std::optional<ComputeCapability> compute_capability;
    GpuVariant variant = GpuVariant::Unknown;
    std::string exact_hardware_variant;
    std::string model_name;
};

/// Compute capabilities plus runtime/toolchain observations.
///
/// The CUDA fields are transitional CAL v1 observations: they describe the
/// availability/version reported by a Provider and do not perform CUDA probing.
/// They are intentionally kept here for v1 compatibility and may be generalized
/// into provider-neutral runtime capability descriptors in a later CAL revision.
struct ComputeCapabilities {
    CapabilityState cuda_state = CapabilityState::Unknown;
    std::optional<Version> cuda_version;
    std::optional<ComputeCapability> compute_capability;
    std::optional<std::uint32_t> cuda_cores;
    TensorCores tensor_cores;
    TensorPrecisionSet tensor_precisions;
};

/// Graphics capabilities plus runtime/toolchain observations.
///
/// The Vulkan/OpenGL fields are transitional CAL v1 observations: they describe
/// facts supplied by Providers and do not imply any Vulkan/OpenGL API dependency
/// or probing inside CAL. They may be generalized in a later CAL revision.
struct GraphicsCapabilities {
    CapabilityState vulkan_state = CapabilityState::Unknown;
    std::optional<Version> vulkan_api_version;
    CapabilityState opengl_state = CapabilityState::Unknown;
    CapabilityState graphics_acceleration_state = CapabilityState::Unknown;
};

/// Hardware-level feature facts.
///
/// hardware.tensor_cores is the physical hardware capability. In contrast,
/// compute.tensor_cores describes Tensor Core availability in the compute
/// execution context reported by a Provider. Providers must keep the two
/// semantically consistent when both are populated; hardware.tensor_cores is
/// the authoritative field for physical presence.
struct HardwareFeatures {
    Capability rt_cores;
    Capability optical_flow_accelerator;
    TensorCores tensor_cores;
    Capability hbm2;
    Capability ecc;
    Capability nvlink;
    Capability mig;
    Capability display_outputs;
};

/// Provider-neutral description of GPU and software-stack capabilities.
///
/// This type contains facts supplied by Providers. It performs no probing and
/// has no dependency on CUDA, NVML, Vulkan, Linux, Windows, or NVIDIA APIs.
struct GpuCapabilities {
    GpuIdentity identity;
    ComputeCapabilities compute;
    GraphicsCapabilities graphics;
    HardwareFeatures hardware;
};

/// Compatibility wrapper retained for the CAL public API.
const char* to_string(CapabilityState state) noexcept;
const char* to_string(GpuVariant variant) noexcept;
const char* to_string(TensorPrecision precision) noexcept;

/// Serialize a capabilities object as deterministic UTF-8 JSON.
/// No external JSON library is required by CAL v1.
std::string to_json(const GpuCapabilities& capabilities);

} // namespace isb::cal
