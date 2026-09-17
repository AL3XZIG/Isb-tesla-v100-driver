#pragma once

#include "isb/providers/vulkan/observed.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace isb::providers::vulkan {

struct PhysicalDeviceObservation {
    Observed<std::string> name;
    Observed<std::uint32_t> vendor_id;
    Observed<std::uint32_t> device_id;
    Observed<std::uint32_t> api_version;
    Observed<std::uint32_t> driver_version;
    Observed<std::uint32_t> device_type;
};

struct RuntimeObservation {
    Observed<std::uint32_t> instance_api_version;
    Observed<std::vector<PhysicalDeviceObservation>> devices;
};

} // namespace isb::providers::vulkan
