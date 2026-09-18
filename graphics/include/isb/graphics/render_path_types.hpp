#pragma once

#include "isb/common/provenance.hpp"
#include "isb/common/capability_state.hpp"

#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <cstdint>

namespace isb::graphics {

using isb::common::Provenance;
using isb::common::CapabilityState;

/// GPU role in multi-GPU configuration
enum class GPURole {
    Unknown,
    ComputeOnly,      ///< Compute/render GPU without display outputs (e.g., Tesla V100)
    DisplayOnly,      ///< Display GPU (e.g., iGPU)
    ComputeAndDisplay ///< Full-featured GPU with both compute and display
};

const char* to_string(GPURole role) noexcept;
GPURole from_gpu_role_string(const std::string& str) noexcept;
/// Alias for from_gpu_role_string for convenience
inline GPURole from_string(const std::string& str) noexcept { return from_gpu_role_string(str); }

/// Graphics API type
enum class GraphicsAPI {
    Unknown,
    Vulkan,
    OpenGL,
    DirectX11,
    DirectX12,
    CUDA,
    OpenCL
};

const char* to_string(GraphicsAPI api) noexcept;
GraphicsAPI from_api_string(const std::string& str) noexcept;

/// Render offload method
enum class RenderOffloadMethod {
    Unknown,
    None,             ///< No offload - single GPU
    PRIME,            ///< Linux PRIME offload
    VulkanOffload,    ///< Vulkan device offload
    Optimus,          ///< NVIDIA Optimus
    Manual,           ///< Manual configuration required
    Unsupported       ///< Offload not supported
};

const char* to_string(RenderOffloadMethod method) noexcept;

/// Individual GPU description
struct GPUDescription {
    std::string name;
    std::string vendor;
    std::string pci_bus;          ///< PCI bus ID (e.g., "0000:04:00.0")
    uint32_t device_id = 0;
    uint32_t vendor_id = 0;
    
    GPURole role = GPURole::Unknown;
    
    // Capabilities
    bool has_display_outputs = false;
    bool vulkan_capable = false;
    bool opengl_capable = false;
    bool cuda_capable = false;
    bool opencl_capable = false;
    bool dx11_capable = false;
    bool dx12_capable = false;
    
    // Driver information
    std::string driver_version;
    std::string driver_model;     ///< WDDM/TCC/MCDM on Windows
    
    // Current state
    bool is_active_display_adapter = false;
    bool is_render_target = false;
    
    Provenance provenance;
};

/// Multi-GPU configuration plan
struct MultiGPUConfig {
    /// Detected GPUs
    std::vector<GPUDescription> gpus;
    
    /// Selected render GPU index (into gpus vector)
    std::optional<size_t> render_gpu_index;
    
    /// Selected display GPU index (into gpus vector)
    std::optional<size_t> display_gpu_index;
    
    /// Recommended offload method
    RenderOffloadMethod offload_method = RenderOffloadMethod::Unknown;
    
    /// Graphics APIs available for offload
    std::vector<GraphicsAPI> available_apis;
    
    /// Configuration status
    CapabilityState status = CapabilityState::Unknown;
    std::string status_reason;
    
    /// Required environment variables (Linux)
    struct EnvVar {
        std::string name;
        std::string value;
        std::string description;
    };
    std::vector<EnvVar> required_env_vars;
    
    /// Per-application configuration (Windows)
    struct AppPreference {
        std::string app_name;
        std::string app_path;
        std::string preferred_gpu;  ///< GPU preference setting
    };
    std::vector<AppPreference> app_preferences;
    
    Provenance provenance;
};

/// Render path detection result
struct RenderPathResult {
    CapabilityState status = CapabilityState::Unknown;
    std::string message;
    std::string error_code;
    
    std::optional<MultiGPUConfig> config;
    Provenance provenance;
};

/// Deployment state for render path configuration
struct RenderPathDeployment {
    bool configured = false;
    std::string config_path;
    bool env_vars_set = false;
    bool app_preferences_set = false;
    bool verified = false;
    std::string verification_message;
    bool rollback_available = false;
    std::string rollback_info;
};

/// Render path operation result
struct RenderPathOperationResult {
    CapabilityState status = CapabilityState::Unknown;
    std::string message;
    std::string error_code;
    
    std::optional<RenderPathDeployment> deployment;
    Provenance provenance;
};

/// Manager interface for render path configuration
class IRenderPathManager {
public:
    virtual ~IRenderPathManager() = default;
    
    /// Detect all GPUs and their capabilities
    virtual RenderPathResult detect_gpus() const = 0;
    
    /// Analyze multi-GPU configuration and recommend setup
    virtual RenderPathResult analyze_config(const MultiGPUConfig& config) const = 0;
    
    /// Get recommended configuration for current system
    virtual RenderPathResult get_recommended_config() const = 0;
    
    /// Plan configuration changes (dry-run)
    virtual RenderPathOperationResult plan_config() const = 0;
    
    /// Apply render path configuration
    virtual RenderPathOperationResult apply_config(bool approved) = 0;
    
    /// Verify current render path configuration
    virtual RenderPathOperationResult verify_config() const = 0;
    
    /// Get current deployment state
    virtual RenderPathOperationResult get_deployment_state() const = 0;
    
    /// Rollback to previous configuration
    virtual RenderPathOperationResult rollback() = 0;
    
    /// Get per-application render preference
    virtual RenderPathOperationResult get_app_preference(const std::string& app_path) const = 0;
    
    /// Set per-application render preference
    virtual RenderPathOperationResult set_app_preference(
        const std::string& app_path,
        const std::string& gpu_id,
        bool approved
    ) = 0;
};

/// Factory function to create render path manager
std::unique_ptr<IRenderPathManager> create_render_path_manager(bool mock_mode = false);

} // namespace isb::graphics
