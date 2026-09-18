#include "isb/graphics/render_path_types.hpp"

namespace isb::graphics {

const char* to_string(GPURole role) noexcept {
    switch (role) {
        case GPURole::Unknown: return "Unknown";
        case GPURole::ComputeOnly: return "Compute Only";
        case GPURole::DisplayOnly: return "Display Only";
        case GPURole::ComputeAndDisplay: return "Compute and Display";
    }
    return "Unknown";
}

GPURole from_string(const std::string& str) noexcept {
    if (str == "Unknown" || str == "unknown") return GPURole::Unknown;
    if (str == "Compute Only" || str == "compute_only" || str == "ComputeOnly") 
        return GPURole::ComputeOnly;
    if (str == "Display Only" || str == "display_only" || str == "DisplayOnly") 
        return GPURole::DisplayOnly;
    if (str == "Compute and Display" || str == "compute_and_display" || str == "ComputeAndDisplay")
        return GPURole::ComputeAndDisplay;
    return GPURole::Unknown;
}

const char* to_string(GraphicsAPI api) noexcept {
    switch (api) {
        case GraphicsAPI::Unknown: return "Unknown";
        case GraphicsAPI::Vulkan: return "Vulkan";
        case GraphicsAPI::OpenGL: return "OpenGL";
        case GraphicsAPI::DirectX11: return "DirectX 11";
        case GraphicsAPI::DirectX12: return "DirectX 12";
        case GraphicsAPI::CUDA: return "CUDA";
        case GraphicsAPI::OpenCL: return "OpenCL";
    }
    return "Unknown";
}

GraphicsAPI from_api_string(const std::string& str) noexcept {
    if (str == "Vulkan" || str == "vulkan") return GraphicsAPI::Vulkan;
    if (str == "OpenGL" || str == "opengl") return GraphicsAPI::OpenGL;
    if (str == "DirectX 11" || str == "DX11" || str == "d3d11") return GraphicsAPI::DirectX11;
    if (str == "DirectX 12" || str == "DX12" || str == "d3d12") return GraphicsAPI::DirectX12;
    if (str == "CUDA" || str == "cuda") return GraphicsAPI::CUDA;
    if (str == "OpenCL" || str == "opencl") return GraphicsAPI::OpenCL;
    return GraphicsAPI::Unknown;
}

const char* to_string(RenderOffloadMethod method) noexcept {
    switch (method) {
        case RenderOffloadMethod::Unknown: return "Unknown";
        case RenderOffloadMethod::None: return "None";
        case RenderOffloadMethod::PRIME: return "PRIME";
        case RenderOffloadMethod::VulkanOffload: return "Vulkan Offload";
        case RenderOffloadMethod::Optimus: return "Optimus";
        case RenderOffloadMethod::Manual: return "Manual";
        case RenderOffloadMethod::Unsupported: return "Unsupported";
    }
    return "Unknown";
}

} // namespace isb::graphics
