#pragma once

#include "isb/graphics/render_path_types.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace isb::graphics::platform {

struct DisplayAdapter {
    std::string name;
    std::string pci_bus;
    std::uint32_t vendor_id = 0;
    std::uint32_t device_id = 0;
    bool active = false;
};

struct PlatformSnapshot {
    bool supported = false;
    std::string os;
    std::string session;
    std::vector<DisplayAdapter> display_adapters;
    std::string driver_model_hint;
    std::string detail;
};

PlatformSnapshot detect_display_adapters();
std::vector<MultiGPUConfig::EnvVar> recommended_linux_environment(
    const MultiGPUConfig& config);

} // namespace isb::graphics::platform
