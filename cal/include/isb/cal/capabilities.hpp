#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

namespace isb::cal {

/// Three-state capability result. Unknown is deliberately distinct from Unavailable.
enum class CapabilityState {
    Unknown,
    Available,
    Unavailable,
};

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

struct ComputeCapabilities {
    CapabilityState cuda_state = CapabilityState::Unknown;
    std::optional<Version> cuda_version;
    std::optional<ComputeCapability> compute_capability;
    std::optional<std::uint32_t> cuda_cores;
    TensorCores tensor_cores;
    TensorPrecisionSet tensor_precisions;
};

struct GraphicsCapabilities {
    CapabilityState vulkan_state = CapabilityState::Unknown;
    std::optional<Version> vulkan_api_version;
    CapabilityState opengl_state = CapabilityState::Unknown;
    CapabilityState graphics_acceleration_state = CapabilityState::Unknown;
};

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

const char* to_string(CapabilityState state) noexcept;
const char* to_string(GpuVariant variant) noexcept;
const char* to_string(TensorPrecision precision) noexcept;

/// Serialize a capabilities object as deterministic UTF-8 JSON.
/// No external JSON library is required by CAL v1.
std::string to_json(const GpuCapabilities& capabilities);

} // namespace isb::cal
