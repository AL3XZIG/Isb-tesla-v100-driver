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
    Observed<PciAddress> pci_address;
    Observed<PciDeviceId> pci_device_id;
    Observed<std::vector<NvLinkLinkObservation>> nvlink_links;
};

} // namespace isb::providers::nvml
