#include "isb/graphics/adapter.hpp"

#include <sstream>
#include <utility>

#if defined(_WIN32)
#include <windows.h>
#include <d3d12.h>
#include <dxgi.h>
#else
#include <dlfcn.h>
#endif

namespace isb::graphics {
namespace {

bool is_available(cal::CapabilityState state) noexcept {
    return state == cal::CapabilityState::Available;
}

GraphicsCapability normalize(const cal::GpuCapabilities& source) {
    GraphicsCapability result;
    result.cuda = source.compute.cuda_state;
    result.vulkan = source.graphics.vulkan_state;
    result.tensor_cores = source.hardware.tensor_cores.state;
    result.rt_cores = source.hardware.rt_cores.state;
    result.optical_flow = source.hardware.optical_flow_accelerator.state;
    result.sm_version = source.compute.compute_capability;
    if (!result.sm_version) result.sm_version = source.identity.compute_capability;
    return result;
}

cal::CapabilityState probe_cuda(std::optional<cal::ComputeCapability>& sm_version) {
#if defined(_WIN32)
    HMODULE library = LoadLibraryA("cudart64_110.dll");
    if (library == nullptr) library = LoadLibraryA("cudart64_101.dll");
    if (library == nullptr) return cal::CapabilityState::Unavailable;
    const auto get_count = reinterpret_cast<int (*)(int*)>(GetProcAddress(library, "cudaGetDeviceCount"));
    const auto get_attribute = reinterpret_cast<int (*)(int*, int, int)>(GetProcAddress(library, "cudaDeviceGetAttribute"));
#else
    void* library = dlopen("libcudart.so", RTLD_LAZY | RTLD_LOCAL);
    if (library == nullptr) return cal::CapabilityState::Unavailable;
    const auto get_count = reinterpret_cast<int (*)(int*)>(dlsym(library, "cudaGetDeviceCount"));
    const auto get_attribute = reinterpret_cast<int (*)(int*, int, int)>(dlsym(library, "cudaDeviceGetAttribute"));
#endif
    int count = 0;
    const int status = get_count == nullptr ? 1 : get_count(&count);
    if (status == 0 && count > 0) {
        int major = 0;
        int minor = 0;
        // cudaDevAttrComputeCapabilityMajor/Minor are stable CUDA runtime ABI values.
        if (get_attribute != nullptr && get_attribute(&major, 75, 0) == 0 &&
            get_attribute(&minor, 76, 0) == 0) {
            sm_version = cal::ComputeCapability{static_cast<std::uint32_t>(major),
                                                static_cast<std::uint32_t>(minor)};
        }
#if defined(_WIN32)
        FreeLibrary(library);
#else
        dlclose(library);
#endif
        return cal::CapabilityState::Available;
    }
#if defined(_WIN32)
    FreeLibrary(library);
#else
    dlclose(library);
#endif
    return cal::CapabilityState::Unavailable;
}

cal::CapabilityState probe_vulkan() {
    using VkInstance = void*;
    using VkPhysicalDevice = void*;
    using VkResult = std::int32_t;
    constexpr VkResult vk_success = 0;
    struct VkApplicationInfo {
        std::uint32_t structure_type; const void* next; const char* application_name;
        std::uint32_t application_version; const char* engine_name;
        std::uint32_t engine_version; std::uint32_t api_version;
    };
    struct VkInstanceCreateInfo {
        std::uint32_t structure_type; const void* next; std::uint32_t flags;
        const VkApplicationInfo* application_info; std::uint32_t layer_count;
        const char* const* layers; std::uint32_t extension_count; const char* const* extensions;
    };
#if defined(_WIN32)
    HMODULE library = LoadLibraryA("vulkan-1.dll");
    if (library == nullptr) return cal::CapabilityState::Unavailable;
    const auto create_instance = reinterpret_cast<VkResult (*)(const VkInstanceCreateInfo*, const void*, VkInstance*)>(
        GetProcAddress(library, "vkCreateInstance"));
    const auto destroy_instance = reinterpret_cast<void (*)(VkInstance, const void*)>(GetProcAddress(library, "vkDestroyInstance"));
    const auto enumerate_devices = reinterpret_cast<VkResult (*)(VkInstance, std::uint32_t*, VkPhysicalDevice*)>(
        GetProcAddress(library, "vkEnumeratePhysicalDevices"));
    const auto enumerate_extensions = reinterpret_cast<VkResult (*)(VkPhysicalDevice, const char*, std::uint32_t*, void*)>(
        GetProcAddress(library, "vkEnumerateDeviceExtensionProperties"));
#else
    void* library = dlopen("libvulkan.so.1", RTLD_LAZY | RTLD_LOCAL);
    if (library == nullptr) return cal::CapabilityState::Unavailable;
    const auto create_instance = reinterpret_cast<VkResult (*)(const VkInstanceCreateInfo*, const void*, VkInstance*)>(
        dlsym(library, "vkCreateInstance"));
    const auto destroy_instance = reinterpret_cast<void (*)(VkInstance, const void*)>(dlsym(library, "vkDestroyInstance"));
    const auto enumerate_devices = reinterpret_cast<VkResult (*)(VkInstance, std::uint32_t*, VkPhysicalDevice*)>(
        dlsym(library, "vkEnumeratePhysicalDevices"));
    const auto enumerate_extensions = reinterpret_cast<VkResult (*)(VkPhysicalDevice, const char*, std::uint32_t*, void*)>(
        dlsym(library, "vkEnumerateDeviceExtensionProperties"));
#endif
    VkApplicationInfo app{0, nullptr, "isb-graphics-capabilities", 1, "isb", 1, 0};
    VkInstanceCreateInfo create{1, nullptr, 0, &app, 0, nullptr, 0, nullptr};
    VkInstance instance = nullptr;
    std::uint32_t count = 0;
    VkPhysicalDevice device = nullptr;
    std::uint32_t extension_count = 0;
    const bool available = create_instance != nullptr && destroy_instance != nullptr &&
        enumerate_devices != nullptr && enumerate_extensions != nullptr &&
        create_instance(&create, nullptr, &instance) == vk_success &&
        enumerate_devices(instance, &count, nullptr) == vk_success && count > 0 &&
        enumerate_devices(instance, &count, &device) == vk_success &&
        enumerate_extensions(device, nullptr, &extension_count, nullptr) == vk_success;
    if (instance != nullptr && destroy_instance != nullptr) destroy_instance(instance, nullptr);
#if defined(_WIN32)
    FreeLibrary(library);
#else
    dlclose(library);
#endif
    // The probe enumerates a physical device and its extension count. Extension
    // names are provider data because the normalized model has no list field yet.
    return available ? cal::CapabilityState::Available : cal::CapabilityState::Unavailable;
}

cal::CapabilityState probe_directx12() {
#if defined(_WIN32)
    HMODULE dxgi = LoadLibraryA("dxgi.dll");
    HMODULE d3d12 = LoadLibraryA("d3d12.dll");
    if (dxgi == nullptr || d3d12 == nullptr) {
        if (dxgi != nullptr) FreeLibrary(dxgi);
        if (d3d12 != nullptr) FreeLibrary(d3d12);
        return cal::CapabilityState::Unavailable;
    }
    const auto create_factory = reinterpret_cast<HRESULT (WINAPI *)(REFIID, void**)>(
        GetProcAddress(dxgi, "CreateDXGIFactory1"));
    const auto create_device = reinterpret_cast<HRESULT (WINAPI *)(IUnknown*, D3D_FEATURE_LEVEL, REFIID, void**)>(
        GetProcAddress(d3d12, "D3D12CreateDevice"));
    IDXGIFactory1* factory = nullptr;
    IDXGIAdapter1* adapter = nullptr;
    IUnknown* device = nullptr;
    const bool available = create_factory != nullptr && create_device != nullptr &&
        SUCCEEDED(create_factory(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&factory))) &&
        SUCCEEDED(factory->EnumAdapters1(0, &adapter)) &&
        SUCCEEDED(create_device(adapter, D3D_FEATURE_LEVEL_11_0, __uuidof(IUnknown), reinterpret_cast<void**>(&device)));
    if (device != nullptr) device->Release();
    if (adapter != nullptr) adapter->Release();
    if (factory != nullptr) factory->Release();
    FreeLibrary(dxgi);
    FreeLibrary(d3d12);
    return available ? cal::CapabilityState::Available : cal::CapabilityState::Unavailable;
#else
    return cal::CapabilityState::Unknown;
#endif
}

std::string identity(const cal::GpuCapabilities& source) {
    if (!source.identity.model_name.empty()) return source.identity.model_name;
    if (!source.identity.exact_hardware_variant.empty()) return source.identity.exact_hardware_variant;
    return "Unknown GPU";
}

} // namespace

const char* to_string(GraphicsBackend backend) noexcept {
    switch (backend) {
    case GraphicsBackend::CUDA: return "CUDA";
    case GraphicsBackend::Vulkan: return "Vulkan";
    case GraphicsBackend::DirectX12: return "DirectX 12";
    case GraphicsBackend::OpenCL: return "OpenCL";
    case GraphicsBackend::Unknown: return "Unknown";
    }
    return "Unknown";
}

ObservedGraphicsBackend::ObservedGraphicsBackend(GraphicsBackend type, bool available, std::string name)
    : type_(type), available_(available), name_(std::move(name)) {}

bool ObservedGraphicsBackend::available() const noexcept { return available_; }
GraphicsBackend ObservedGraphicsBackend::type() const noexcept { return type_; }
std::string ObservedGraphicsBackend::name() const { return name_; }

GraphicsAdapter::GraphicsAdapter(cal::GpuCapabilities capabilities)
    : cal_capabilities_(std::move(capabilities)), capability_(normalize(cal_capabilities_)),
      gpu_identity_(identity(cal_capabilities_)) {}

void GraphicsAdapter::probe() {
    capability_ = normalize(cal_capabilities_);
    if (capability_.cuda == cal::CapabilityState::Unknown) {
        capability_.cuda = probe_cuda(capability_.sm_version);
    }
    if (capability_.vulkan == cal::CapabilityState::Unknown) {
        capability_.vulkan = probe_vulkan();
    }
    if (capability_.directx_compute == cal::CapabilityState::Unknown) {
        capability_.directx_compute = probe_directx12();
    }
}

const GraphicsCapability& GraphicsAdapter::capability() const noexcept { return capability_; }
const std::string& GraphicsAdapter::gpu_identity() const noexcept { return gpu_identity_; }

std::vector<ObservedGraphicsBackend> GraphicsAdapter::backends() const {
    return {
        {GraphicsBackend::CUDA, is_available(capability_.cuda), "CUDA runtime"},
        {GraphicsBackend::Vulkan, is_available(capability_.vulkan), "Vulkan loader/device"},
        {GraphicsBackend::DirectX12, is_available(capability_.directx_compute), "DirectX 12 compute"},
    };
}

std::string to_json(const GraphicsCapability& capability) {
    std::ostringstream out;
    out << "{\"cuda\":\"" << cal::to_string(capability.cuda)
        << "\",\"vulkan\":\"" << cal::to_string(capability.vulkan)
        << "\",\"directx_compute\":\"" << cal::to_string(capability.directx_compute)
        << "\",\"tensor_cores\":\"" << cal::to_string(capability.tensor_cores)
        << "\",\"rt_cores\":\"" << cal::to_string(capability.rt_cores)
        << "\",\"optical_flow\":\"" << cal::to_string(capability.optical_flow)
        << "\",\"sm_version\":";
    if (capability.sm_version) {
        out << "{\"major\":" << capability.sm_version->major
            << ",\"minor\":" << capability.sm_version->minor << '}';
    } else {
        out << "null";
    }
    return out << '}', out.str();
}

} // namespace isb::graphics
