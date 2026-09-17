#pragma once

#include "isb/providers/nvml/pci.hpp"

#include <cstdint>

namespace isb::providers::nvml {

inline PciDeviceId decode_pci_device_id(std::uint32_t encoded_device_id) noexcept {
    return {static_cast<std::uint16_t>(encoded_device_id >> 16U),
            static_cast<std::uint16_t>(encoded_device_id & 0xffffU)};
}

} // namespace isb::providers::nvml
