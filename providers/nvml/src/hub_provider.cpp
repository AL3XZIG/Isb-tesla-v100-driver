#include "isb/providers/nvml/hub_provider.hpp"

#include <string>
#include <utility>

namespace isb::providers::nvml {
namespace {

template <typename T>
bool has(const Observed<T>& value) {
    return value.value.has_value();
}

template <typename T>
const T* get(const Observed<T>& value) {
    return value.value ? &*value.value : nullptr;
}

const RawGpuObservation* first_observation(std::vector<RawGpuObservation>& observations) {
    return observations.empty() ? nullptr : &observations.front();
}

hub::TelemetrySnapshot telemetry_from(const RawGpuObservation& o) {
    hub::TelemetrySnapshot t;
    t.synthetic = false;
    t.provenance = {"nvml", "read-only NVML observation", false};

    if (const auto* value = get(o.temperature_celsius)) t.temperature_c = static_cast<int>(*value);
    if (const auto* value = get(o.gpu_utilization_percent)) t.gpu_utilization_percent = static_cast<int>(*value);
    if (const auto* value = get(o.memory_utilization_percent)) t.memory_utilization_percent = static_cast<int>(*value);
    if (const auto* value = get(o.power_draw_milliwatts)) t.power_w = static_cast<int>(*value / 1000U);
    if (const auto* value = get(o.power_limit_milliwatts)) t.power_limit_w = static_cast<int>(*value / 1000U);
    if (const auto* value = get(o.sm_clock_mhz)) t.gpu_clock_mhz = static_cast<int>(*value);
    if (const auto* value = get(o.memory_clock_mhz)) t.memory_clock_mhz = static_cast<int>(*value);
    if (const auto* value = get(o.memory_used_bytes)) t.vram_used_mib = static_cast<int>(*value / (1024ULL * 1024ULL));
    if (const auto* value = get(o.memory_total_bytes)) t.vram_total_mib = static_cast<int>(*value / (1024ULL * 1024ULL));
    if (const auto* value = get(o.performance_state)) t.performance_state = "P" + std::to_string(*value);
    if (has(o.ecc_enabled)) t.ecc = *get(o.ecc_enabled) ? "enabled" : "disabled";
    if (const auto* value = get(o.pcie_generation)) {
        t.pcie = "Gen" + std::to_string(*value);
        if (const auto* width = get(o.pcie_link_width)) {
            t.pcie += " x" + std::to_string(*width);
        }
    }
    if (has(o.nvlink_links)) {
        const auto& links = *get(o.nvlink_links);
        std::size_t active = 0;
        for (const auto& link : links) {
            if (link.active.value && *link.active.value) ++active;
        }
        t.nvlink = std::to_string(active) + "/" + std::to_string(links.size()) + " active";
    }
    t.driver = "NVML provider";
    return t;
}

cal::GpuCapabilities capabilities_from(const RawGpuObservation& o) {
    cal::GpuCapabilities c;
    c.identity.vendor = "NVIDIA";
    if (const auto* name = get(o.name)) {
        c.identity.model_name = *name;
        c.identity.exact_hardware_variant = *name;
        if (name->find("Tesla V100-SXM2") != std::string::npos ||
            name->find("Tesla V100 SXM2") != std::string::npos) {
            c.identity.variant = cal::GpuVariant::V100_SXM2;
        } else if (name->find("Tesla V100") != std::string::npos) {
            c.identity.variant = cal::GpuVariant::Unknown;
        }
    }
    c.identity.architecture = "Volta";
    c.compute.compute_capability = cal::ComputeCapability{7, 0};

    if (has(o.ecc_enabled)) {
        c.hardware.ecc.state = cal::CapabilityState::Available;
    }
    if (has(o.nvlink_links)) {
        c.hardware.nvlink.state = cal::CapabilityState::Available;
    }
    return c;
}

} // namespace

hub::Environment HubProvider::environment() const {
    return {"unknown", "unknown", hub::ProviderMode::Real,
            {"nvml", "NVML provider compiled; runtime state is queried on demand", false}};
}

cal::GpuCapabilities HubProvider::capabilities() const {
    auto provider = make_nvml_provider();
    if (!provider) return {};

    auto result = provider->observe();
    if (!result.ok() || result.value().empty()) return {};
    return capabilities_from(result.value().front());
}

hub::TelemetrySnapshot HubProvider::telemetry() const {
    auto provider = make_nvml_provider();
    if (!provider) {
        hub::TelemetrySnapshot t;
        t.provenance = {"nvml", "NVML provider unavailable", false};
        t.driver = "unknown";
        t.performance_state = "unknown";
        t.ecc = "unknown";
        t.pcie = "unknown";
        t.nvlink = "unknown";
        return t;
    }

    auto result = provider->observe();
    if (!result.ok() || result.value().empty()) {
        hub::TelemetrySnapshot t;
        t.provenance = {"nvml", result.ok() ? "no NVIDIA GPU reported" : result.status().message(), false};
        t.driver = "unknown";
        t.performance_state = "unknown";
        t.ecc = "unknown";
        t.pcie = "unknown";
        t.nvlink = "unknown";
        return t;
    }
    return telemetry_from(result.value().front());
}

std::vector<hub::Control> HubProvider::controls() const {
    return {
        {"persistence_mode", hub::ControlState::Unsupported,
         "NVML integration is read-only in this phase"},
        {"power_limit", hub::ControlState::Unsupported,
         "NVML integration is read-only in this phase"},
        {"application_clocks", hub::ControlState::Unsupported,
         "NVML integration is read-only in this phase"},
        {"compute_mode", hub::ControlState::Unsupported,
         "NVML integration is read-only in this phase"},
    };
}

} // namespace isb::providers::nvml
