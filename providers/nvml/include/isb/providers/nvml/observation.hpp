#pragma once

#include "isb/providers/nvml/observed.hpp"
#include "isb/providers/nvml/pci.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace isb::providers::nvml {

/// NVLink link state only. This is not CUDA P2P verification or a benchmark.
struct NvLinkLinkObservation {
    std::uint32_t link_index = 0;
    Observed<bool> active;
};

/// Raw facts reported by NVML; this type intentionally contains no inference or recommendation.
struct RawGpuObservation {
    Observed<std::uint32_t> index;
    Observed<std::string> name;
    Observed<std::string> uuid;
    Observed<std::string> vbios_version;
    Observed<PciAddress> pci_address;
    Observed<PciDeviceId> pci_device_id;

    Observed<std::uint64_t> memory_total_bytes;
    Observed<std::uint64_t> memory_used_bytes;
    Observed<std::uint64_t> memory_free_bytes;

    Observed<std::uint32_t> temperature_celsius;
    Observed<std::uint32_t> power_draw_milliwatts;
    Observed<std::uint32_t> power_limit_milliwatts;

    Observed<std::uint32_t> graphics_clock_mhz;
    Observed<std::uint32_t> memory_clock_mhz;
    Observed<std::uint32_t> sm_clock_mhz;
    Observed<std::uint32_t> gpu_utilization_percent;
    Observed<std::uint32_t> memory_utilization_percent;

    Observed<bool> persistence_mode;
    Observed<std::uint32_t> compute_mode;
    Observed<std::uint32_t> performance_state;

    Observed<bool> ecc_enabled;
    Observed<std::uint64_t> ecc_corrected_errors;
    Observed<std::uint64_t> ecc_uncorrected_errors;

    Observed<std::uint32_t> pcie_generation;
    Observed<std::uint32_t> pcie_link_width;

    Observed<PciAddress> pci_address;
    Observed<PciDeviceId> pci_device_id;
    Observed<std::vector<NvLinkLinkObservation>> nvlink_links;
};

} // namespace isb::providers::nvml
