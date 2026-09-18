#include "render_path_platform.hpp"

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <utility>

#ifdef _WIN32
#include <windows.h>
#include <dxgi.h>
#endif

namespace isb::graphics::platform {
namespace {

#ifdef __linux__
bool connected_connector(const std::filesystem::path& status_path) {
    std::ifstream input(status_path);
    std::string status;
    std::getline(input, status);
    return status == "connected";
}

std::string pci_bus_from_device_path(const std::filesystem::path& device_path) {
    std::error_code ec;
    const auto canonical = std::filesystem::weakly_canonical(device_path, ec);
    if (ec) return {};

    for (const auto& part : canonical) {
        const std::string value = part.string();
        if (value.size() == 12 && value.rfind("0000:", 0) == 0 &&
            value[7] == ':' && value[10] == '.') {
            return value;
        }
    }
    return {};
}
#endif

} // namespace

PlatformSnapshot detect_display_adapters() {
    PlatformSnapshot snapshot;

#ifdef _WIN32
    snapshot.supported = true;
    snapshot.os = "Windows";
    snapshot.session = "Windows desktop";
    IDXGIFactory* factory = nullptr;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory)))) {
        snapshot.detail = "DXGI factory creation failed.";
        return snapshot;
    }

    for (UINT index = 0;; ++index) {
        IDXGIAdapter* adapter = nullptr;
        if (factory->EnumAdapters(index, &adapter) == DXGI_ERROR_NOT_FOUND) break;
        if (!adapter) continue;

        DXGI_ADAPTER_DESC desc{};
        if (SUCCEEDED(adapter->GetDesc(&desc))) {
            DisplayAdapter display;
            display.vendor_id = desc.VendorId;
            display.device_id = desc.DeviceId;
            display.active = true;
            display.name = "DXGI adapter";
            snapshot.display_adapters.push_back(std::move(display));
        }
        adapter->Release();
    }
    factory->Release();
    snapshot.driver_model_hint = "WDDM/DXGI";
    snapshot.detail = "DXGI hardware adapters are enumerated. DXGI adapter enumeration alone does not prove which physical output is currently presenting.";
#elif defined(__linux__)
    snapshot.supported = true;
    snapshot.os = "Linux";
    const char* wayland = std::getenv("WAYLAND_DISPLAY");
    const char* display = std::getenv("DISPLAY");
    snapshot.session = (wayland && *wayland) ? "Wayland" :
                       ((display && *display) ? "X11" : "headless/unknown");

    const std::filesystem::path drm_root{"/sys/class/drm"};
    std::error_code ec;
    if (std::filesystem::exists(drm_root, ec)) {
        for (const auto& entry : std::filesystem::directory_iterator(drm_root, ec)) {
            if (ec || !entry.is_directory()) continue;
            const auto status = entry.path() / "status";
            if (!std::filesystem::exists(status, ec) || !connected_connector(status)) continue;
            DisplayAdapter display_adapter;
            display_adapter.active = true;
            display_adapter.pci_bus = pci_bus_from_device_path(entry.path() / "device");
            display_adapter.name = entry.path().filename().string();
            snapshot.display_adapters.push_back(std::move(display_adapter));
        }
    }
    snapshot.detail = "DRM connector state identifies adapters driving a connected display; connector state is not a render-device preference.";
#else
    snapshot.os = "Unknown";
    snapshot.session = "Unknown";
    snapshot.detail = "No platform display-adapter provider is implemented for this OS.";
#endif

    return snapshot;
}

std::vector<MultiGPUConfig::EnvVar> recommended_linux_environment(
    const MultiGPUConfig& config) {
    std::vector<MultiGPUConfig::EnvVar> vars;
#if defined(__linux__)
    if (config.offload_method == RenderOffloadMethod::PRIME ||
        config.offload_method == RenderOffloadMethod::VulkanOffload) {
        vars.push_back({"__NV_PRIME_RENDER_OFFLOAD", "1",
                        "Enable NVIDIA PRIME render offload for the application."});
        bool vulkan = false;
        bool opengl = false;
        for (const auto api : config.available_apis) {
            vulkan = vulkan || api == GraphicsAPI::Vulkan;
            opengl = opengl || api == GraphicsAPI::OpenGL;
        }
        if (opengl) {
            vars.push_back({"__GLX_VENDOR_LIBRARY_NAME", "nvidia",
                            "Select the NVIDIA GLX vendor for OpenGL applications."});
        }
        if (vulkan) {
            vars.push_back({"__VK_LAYER_NV_optimus", "NVIDIA_only",
                            "Prefer NVIDIA in Vulkan device selection when the Optimus layer is available."});
        }
    }
#else
    (void)config;
#endif
    return vars;
}

} // namespace isb::graphics::platform
