#pragma once

#include <cstdint>
#include <string>

namespace isb::providers::nvml {

struct PciAddress {
    std::string bus_id;
    std::uint32_t domain = 0;
    std::uint32_t bus = 0;
    std::uint32_t device = 0;
    std::uint32_t function = 0;
};

struct PciDeviceId {
    std::uint16_t vendor_id = 0;
    std::uint16_t device_id = 0;
};

} // namespace isb::providers::nvml
