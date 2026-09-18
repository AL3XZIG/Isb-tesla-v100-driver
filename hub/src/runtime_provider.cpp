#include "isb/hub/runtime_provider.hpp"

#include "isb/providers/cuda/provider.hpp"
#include "isb/providers/nvml/hub_provider.hpp"
#include "isb/providers/vulkan/provider.hpp"

#include <algorithm>
#include <cctype>
#include <cstdint>
#include <string>

namespace isb::hub {
namespace {

std::string host_os() {
#if defined(_WIN32)
    return "Windows";
#elif defined(__linux__)
    return "Linux";
#elif defined(__APPLE__)
    return "macOS";
#else
    return "Unknown";
#endif
}

bool has_real_telemetry(const TelemetrySnapshot& t) {
    return t.temperature_c.has_value() ||
           t.gpu_utilization_percent.has_value() ||
           t.memory_utilization_percent.has_value() ||
           t.power_w.has_value() ||
           t.gpu_clock_mhz.has_value() ||
           t.vram_total_mib.has_value();
}

cal::Version vulkan_version(std::uint32_t version) noexcept {\n    // Vulkan API versions are packed as variant:3 | major:7 | minor:10 | patch:12.\n    return cal::Version{\n        static_cast<int>((version >> 22U) & 0x7FU),\n        static_cast<int>((version >> 12U) & 0x3FFU),\n        static_cast<int>(version & 0xFFFU)};\n}\n\nbool is_v100_name(const std::string& name) {
    std::string lower = name;
    std::transform(lower.begin(), lower.end(), lower.begin(),
                   [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return lower.find("tesla v100") != std::string::npos;
}

cal::GpuCapabilities merge_cuda(cal::GpuCapabilities caps,
                                 const providers::cuda::RuntimeObservation& observation) {
    caps.compute.cuda_state = common::CapabilityState::Available;

    if (!observation.devices.value || observation.devices.value->empty()) {
        return caps;
    }

    const auto& device = observation.devices.value->front();
    if (device.name.value && caps.identity.exact_hardware_variant.empty()) {
        caps.identity.vendor = "NVIDIA";
        caps.identity.model_name = *device.name.value;
        caps.identity.exact_hardware_variant = *device.name.value;
        caps.identity.architecture = "Unknown";
        if (is_v100_name(*device.name.value)) {
            caps.identity.architecture = "Volta";
            caps.identity.variant =
                device.name.value->find("SXM2") != std::string::npos
                    ? cal::GpuVariant::V100_SXM2
                    : cal::GpuVariant::V100_PCIe;
        }
    }

    if (device.compute_capability_major.value &&
        device.compute_capability_minor.value) {
        caps.compute.compute_capability = cal::ComputeCapability{
            *device.compute_capability_major.value,
            *device.compute_capability_minor.value};
        caps.identity.compute_capability = caps.compute.compute_capability;
        if (*device.compute_capability_major.value == 7 &&
            *device.compute_capability_minor.value == 0) {
            caps.compute.tensor_cores.state = common::CapabilityState::Available;
            caps.compute.tensor_cores.generation = 1;
            caps.compute.tensor_precisions.state = common::CapabilityState::Available;
            caps.compute.tensor_precisions.values = {cal::TensorPrecision::FP16};
            caps.hardware.tensor_cores.state = common::CapabilityState::Available;
            caps.hardware.tensor_cores.generation = 1;
            caps.hardware.hbm2.state = common::CapabilityState::Available;
            caps.hardware.ecc.state = common::CapabilityState::Available;
            caps.hardware.rt_cores.state = common::CapabilityState::Unavailable;
            caps.hardware.optical_flow_accelerator.state =
                common::CapabilityState::Unavailable;
            caps.hardware.mig.state = common::CapabilityState::Unavailable;
            caps.hardware.display_outputs.state =
                common::CapabilityState::Unavailable;
        }
    }

    return caps;
}

} // namespace

Environment RuntimeProvider::environment() const {
    providers::nvml::HubProvider nvml;
    const auto nvml_environment = nvml.environment();

    auto cuda = providers::cuda::make_cuda_provider();
    auto vulkan = providers::vulkan::make_vulkan_provider();

    bool cuda_available = false;
    bool vulkan_available = false;
    if (cuda) {
        const auto result = cuda->observe();
        cuda_available = result.ok() && result.value().devices.value.has_value();
    }
    if (vulkan) {
        const auto result = vulkan->observe();
        vulkan_available = result.ok() && result.value().devices.value.has_value();
    }

    if (nvml_environment.mode == ProviderMode::Real) {
        auto result = nvml_environment;
        result.os = host_os();
        result.provenance.detail +=
            " | aggregated with CUDA/Vulkan probes";
        return result;
    }

    if (cuda_available || vulkan_available) {
        return {
            host_os(),
            "unknown",
            ProviderMode::Real,
            {"runtime-aggregator",
             std::string("NVML unavailable; CUDA=") +
                 (cuda_available ? "available" : "unavailable") +
                 ", Vulkan=" + (vulkan_available ? "available" : "unavailable"),
             false}};
    }

    return {
        host_os(),
        "unknown",
        ProviderMode::Unavailable,
        {"runtime-aggregator",
         "No real NVIDIA runtime provider produced usable device evidence",
         false}};
}

cal::GpuCapabilities RuntimeProvider::capabilities() const {
    providers::nvml::HubProvider nvml;
    auto caps = nvml.capabilities();

    if (auto cuda = providers::cuda::make_cuda_provider()) {
        const auto result = cuda->observe();
        if (result.ok()) {
            caps = merge_cuda(std::move(caps), result.value());
        }
    }

    if (auto vulkan = providers::vulkan::make_vulkan_provider()) {
        const auto result = vulkan->observe();
        if (result.ok() && result.value().devices.value &&
            !result.value().devices.value->empty()) {
            caps.graphics.vulkan_state = common::CapabilityState::Available;
            if (result.value().instance_api_version.value) {
                const auto version = *result.value().instance_api_version.value;
                caps.graphics.vulkan_api_version = cal::Version{
                    VK_VERSION_MAJOR(version),
                    VK_VERSION_MINOR(version),
                    VK_VERSION_PATCH(version)};
            }
        }
    }

    return caps;
}

TelemetrySnapshot RuntimeProvider::telemetry() const {
    providers::nvml::HubProvider nvml;
    const auto nvml_telemetry = nvml.telemetry();
    if (has_real_telemetry(nvml_telemetry)) {
        return nvml_telemetry;
    }

    TelemetrySnapshot telemetry;
    telemetry.provenance = {
        "runtime-aggregator",
        "NVML telemetry unavailable; CUDA-only observation",
        false};
    telemetry.driver = "unknown";

    if (auto cuda = providers::cuda::make_cuda_provider()) {
        const auto result = cuda->observe();
        if (result.ok() && result.value().devices.value &&
            !result.value().devices.value->empty()) {
            const auto& device = result.value().devices.value->front();
            if (device.total_global_memory_bytes.value) {
                telemetry.vram_total_mib = static_cast<int>(
                    *device.total_global_memory_bytes.value / (1024ULL * 1024ULL));
            }
            if (device.name.value) {
                telemetry.provenance.detail =
                    "CUDA runtime: " + *device.name.value;
            }
        }
    }

    return telemetry;
}

std::vector<Control> RuntimeProvider::controls() const {
    providers::nvml::HubProvider nvml;
    return nvml.controls();
}

std::unique_ptr<Provider> make_runtime_provider() {
    return std::make_unique<RuntimeProvider>();
}

} // namespace isb::hub
