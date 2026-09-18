#include "isb/graphics/render_path_types.hpp"
#include "isb/common/provenance.hpp"

#include <algorithm>
#include <sstream>

namespace isb::graphics {
namespace {

/// Mock implementation for testing and environments without real GPU detection
class MockRenderPathManager final : public IRenderPathManager {
public:
    explicit MockRenderPathManager() = default;

    RenderPathResult detect_gpus() const override {
        RenderPathResult result;
        result.status = common::CapabilityState::Unavailable;
        result.message = "GPU detection is boundary-only - mock data";
        result.provenance = {"mock", "Render path detection requires real hardware/providers", true};

        // Create mock multi-GPU configuration representing typical V100 + iGPU scenario
        MultiGPUConfig config;
        
        // Mock Tesla V100 as compute-only GPU
        GPUDescription v100;
        v100.name = "Tesla V100 (Mock)";
        v100.vendor = "NVIDIA";
        v100.pci_bus = "0000:04:00.0";
        v100.device_id = 0x15B8;  // Tesla V100 PCIe device ID
        v100.vendor_id = 0x10DE;  // NVIDIA vendor ID
        v100.role = GPURole::ComputeOnly;
        v100.has_display_outputs = false;  // V100 has no display outputs
        v100.vulkan_capable = true;
        v100.opengl_capable = true;
        v100.cuda_capable = true;
        v100.opencl_capable = true;
        v100.dx11_capable = false;  // TCC mode typically doesn't expose DX
        v100.dx12_capable = false;
        v100.driver_version = "UNKNOWN";
        v100.driver_model = "TCC (Mock)";
        v100.is_active_display_adapter = false;
        v100.is_render_target = true;  // Intended as render target
        v100.provenance = {"mock", "V100 mock description", true};
        config.gpus.push_back(v100);

        // Mock iGPU as display-only GPU
        GPUDescription igpu;
        igpu.name = "Intel UHD Graphics (Mock)";
        igpu.vendor = "Intel";
        igpu.pci_bus = "0000:00:02.0";
        igpu.device_id = 0x3E98;
        igpu.vendor_id = 0x8086;
        igpu.role = GPURole::DisplayOnly;
        igpu.has_display_outputs = true;
        igpu.vulkan_capable = true;
        igpu.opengl_capable = true;
        igpu.cuda_capable = false;
        igpu.opencl_capable = true;
        igpu.dx11_capable = true;
        igpu.dx12_capable = true;
        igpu.driver_version = "UNKNOWN";
        igpu.driver_model = "WDDM (Mock)";
        igpu.is_active_display_adapter = true;
        igpu.is_render_target = false;
        igpu.provenance = {"mock", "iGPU mock description", true};
        config.gpus.push_back(igpu);

        // Recommend V100 as render GPU, iGPU as display GPU
        config.render_gpu_index = 0;  // V100
        config.display_gpu_index = 1; // iGPU
        config.offload_method = RenderOffloadMethod::VulkanOffload;
        config.available_apis = {GraphicsAPI::Vulkan, GraphicsAPI::OpenGL};
        config.status = common::CapabilityState::Partial;
        config.status_reason = "Multi-GPU configuration detected - offload required";
        config.provenance = {"mock", "Recommended configuration based on mock data", true};

        // Add Linux environment variables for PRIME/Vulkan offload
        config.required_env_vars.push_back({
            "__NV_PRIME_RENDER_OFFLOAD",
            "1",
            "Enable PRIME render offload to NVIDIA GPU"
        });
        config.required_env_vars.push_back({
            "__GLX_VENDOR_LIBRARY_NAME",
            "nvidia",
            "Use NVIDIA GLX library"
        });
        config.required_env_vars.push_back({
            "VK_LAYER_NV_optimus",
            "NVIDIA_only",
            "Use NVIDIA Vulkan layer only"
        });

        result.config = config;
        return result;
    }

    RenderPathResult analyze_config(const MultiGPUConfig& config) const override {
        RenderPathResult result;
        result.provenance = {"mock", "Configuration analysis is boundary-only", true};

        if (config.gpus.empty()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "NO_GPUS_DETECTED";
            result.message = "No GPUs detected in configuration";
            return result;
        }

        // Analyze the configuration
        std::ostringstream msg;
        msg << "Configuration analysis (dry-run):\n";
        msg << "  Total GPUs: " << config.gpus.size() << "\n";
        
        bool has_compute_gpu = false;
        bool has_display_gpu = false;
        for (const auto& gpu : config.gpus) {
            msg << "  - " << gpu.name << ": " << to_string(gpu.role) << "\n";
            if (gpu.role == GPURole::ComputeOnly || gpu.role == GPURole::ComputeAndDisplay) {
                has_compute_gpu = true;
            }
            if (gpu.has_display_outputs || gpu.role == GPURole::DisplayOnly) {
                has_display_gpu = true;
            }
        }

        if (has_compute_gpu && has_display_gpu) {
            result.status = common::CapabilityState::Ready;
            msg << "\nMulti-GPU configuration viable\n";
        } else if (!has_compute_gpu) {
            result.status = common::CapabilityState::Unsupported;
            msg << "\nNo compute-capable GPU detected\n";
        } else if (!has_display_gpu) {
            result.status = common::CapabilityState::Partial;
            msg << "\nNo display GPU detected - rendering possible but no output\n";
        }

        result.message = msg.str();
        result.config = config;
        return result;
    }

    RenderPathResult get_recommended_config() const override {
        return detect_gpus();
    }

    RenderPathOperationResult plan_config() const override {
        RenderPathOperationResult result;
        result.provenance = {"mock", "Configuration planning is boundary-only", true};

        auto detection = detect_gpus();
        if (!detection.config.has_value()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "DETECTION_FAILED";
            result.message = "Cannot plan configuration - GPU detection failed";
            return result;
        }

        const auto& config = detection.config.value();
        
        std::ostringstream msg;
        msg << "Render Path Configuration Plan (dry-run):\n\n";
        msg << "Detected GPUs:\n";
        for (size_t i = 0; i < config.gpus.size(); ++i) {
            const auto& gpu = config.gpus[i];
            msg << "  [" << i << "] " << gpu.name << "\n";
            msg << "      Role: " << to_string(gpu.role) << "\n";
            msg << "      Display Outputs: " << (gpu.has_display_outputs ? "Yes" : "No") << "\n";
            msg << "      PCI Bus: " << gpu.pci_bus << "\n";
        }

        if (config.render_gpu_index.has_value() && config.display_gpu_index.has_value()) {
            msg << "\nRecommended Configuration:\n";
            msg << "  Render GPU: [" << config.render_gpu_index.value() << "] " 
                << config.gpus[config.render_gpu_index.value()].name << "\n";
            msg << "  Display GPU: [" << config.display_gpu_index.value() << "] "
                << config.gpus[config.display_gpu_index.value()].name << "\n";
            msg << "  Offload Method: " << to_string(config.offload_method) << "\n";
            
            if (!config.required_env_vars.empty()) {
                msg << "\nRequired Environment Variables (Linux):\n";
                for (const auto& env : config.required_env_vars) {
                    msg << "  " << env.name << "=" << env.value << "\n";
                    msg << "    Description: " << env.description << "\n";
                }
            }

            msg << "\nNote: This is a dry-run. Use --apply to configure.\n";
            msg << "Warning: Changes require explicit approval before application.\n";
        }

        result.status = common::CapabilityState::Ready;
        result.message = msg.str();
        return result;
    }

    RenderPathOperationResult apply_config(bool approved) override {
        RenderPathOperationResult result;
        result.provenance = {"mock", "Configuration application is boundary-only", true};

        if (!approved) {
            result.status = common::CapabilityState::Error;
            result.error_code = "APPROVAL_REQUIRED";
            result.message = "Explicit approval required before applying render path configuration";
            return result;
        }

        // In real implementation:
        // 1. Create backup of current configuration
        // 2. Set environment variables (Linux) or app preferences (Windows)
        // 3. Write configuration files
        // 4. Verify changes
        // 5. Record rollback information

        result.status = common::CapabilityState::Error;
        result.error_code = "DEPLOYMENT_NOT_IMPLEMENTED";
        result.message = "Render path deployment not implemented - boundary only";
        return result;
    }

    RenderPathOperationResult verify_config() const override {
        RenderPathOperationResult result;
        result.provenance = {"mock", "Configuration verification is boundary-only", true};

        RenderPathDeployment state;
        state.configured = false;
        state.env_vars_set = false;
        state.app_preferences_set = false;
        state.verified = false;
        state.rollback_available = false;
        state.verification_message = "Render path verification not implemented - boundary only";

        result.deployment = state;
        result.status = common::CapabilityState::Unknown;
        result.message = "Verification completed (no configuration found)";
        return result;
    }

    RenderPathOperationResult get_deployment_state() const override {
        return verify_config();
    }

    RenderPathOperationResult rollback() override {
        RenderPathOperationResult result;
        result.provenance = {"mock", "Rollback is boundary-only", true};

        result.status = common::CapabilityState::Error;
        result.error_code = "ROLLBACK_NOT_IMPLEMENTED";
        result.message = "Rollback not implemented - boundary only";
        return result;
    }

    RenderPathOperationResult get_app_preference(const std::string& app_path) const override {
        RenderPathOperationResult result;
        result.provenance = {"mock", "App preference retrieval is boundary-only", true};

        if (app_path.empty()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "INVALID_PATH";
            result.message = "Application path required";
            return result;
        }

        result.status = common::CapabilityState::Unknown;
        result.message = "Application preference retrieval not implemented - boundary only";
        return result;
    }

    RenderPathOperationResult set_app_preference(
        const std::string& app_path,
        const std::string& gpu_id,
        bool approved
    ) override {
        RenderPathOperationResult result;
        result.provenance = {"mock", "App preference setting is boundary-only", true};

        if (app_path.empty() || gpu_id.empty()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "INVALID_ARGUMENT";
            result.message = "Application path and GPU ID required";
            return result;
        }

        if (!approved) {
            result.status = common::CapabilityState::Error;
            result.error_code = "APPROVAL_REQUIRED";
            result.message = "Explicit approval required before changing app preference";
            return result;
        }

        // In real implementation:
        // Windows: Set GPU preference via registry or graphics settings
        // Linux: Create wrapper script or desktop file with environment variables

        result.status = common::CapabilityState::Error;
        result.error_code = "NOT_IMPLEMENTED";
        result.message = "Application preference setting not implemented - boundary only";
        return result;
    }
};

} // namespace

std::unique_ptr<IRenderPathManager> create_render_path_manager(bool /*mock_mode*/) {
    // Always use mock for now - real implementation requires:
    // 1. NVML provider for GPU detection
    // 2. Vulkan provider for device enumeration
    // 3. OS-specific configuration mechanisms
    // 4. Real deployment/backup system
    
    return std::make_unique<MockRenderPathManager>();
}

} // namespace isb::graphics
