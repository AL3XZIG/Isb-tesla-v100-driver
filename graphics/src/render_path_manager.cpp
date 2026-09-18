#include "isb/graphics/render_path_types.hpp"
#include "isb/providers/nvml/provider.hpp"
#include "isb/providers/vulkan/provider.hpp"
#include "render_path_platform.hpp"

#include <algorithm>
#include <cctype>
#include <memory>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

namespace isb::graphics {
namespace {

bool contains_ci(const std::string& text, const std::string& needle) {
    if (needle.empty()) return true;
    auto it = text.begin();
    for (;;) {
        it = std::search(it, text.end(), needle.begin(), needle.end(),
                          [](char a, char b) {
                              return std::tolower(static_cast<unsigned char>(a)) ==
                                     std::tolower(static_cast<unsigned char>(b));
                          });
        if (it == text.end()) return false;
        return true;
    }
}

bool matches_platform_display(const GPUDescription& gpu,
                              const platform::PlatformSnapshot& platform_snapshot) {
    for (const auto& adapter : platform_snapshot.display_adapters) {
        if (!adapter.active) continue;
        if (!gpu.pci_bus.empty() && !adapter.pci_bus.empty() &&
            gpu.pci_bus == adapter.pci_bus) {
            return true;
        }
        if (gpu.vendor_id != 0 && gpu.device_id != 0 &&
            adapter.vendor_id == gpu.vendor_id &&
            adapter.device_id == gpu.device_id) {
            return true;
        }
    }
    return false;
}

class MockRenderPathManager final : public IRenderPathManager {
public:
    RenderPathResult detect_gpus() const override {
        RenderPathResult result;
        result.status = common::CapabilityState::Unavailable;
        result.message = "GPU detection is running in explicit mock mode.";
        result.provenance = {"mock", "Synthetic V100 + iGPU configuration", true};

        MultiGPUConfig config;
        GPUDescription v100;
        v100.name = "Tesla V100 (Mock)";
        v100.vendor = "NVIDIA";
        v100.pci_bus = "0000:04:00.0";
        v100.device_id = 0x15B8;
        v100.vendor_id = 0x10DE;
        v100.role = GPURole::ComputeOnly;
        v100.has_display_outputs = false;
        v100.vulkan_capable = true;
        v100.opengl_capable = true;
        v100.cuda_capable = true;
        v100.opencl_capable = true;
        v100.driver_model = "TCC (Mock)";
        v100.is_render_target = true;
        v100.provenance = {"mock", "Synthetic V100", true};
        config.gpus.push_back(v100);

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
        igpu.opencl_capable = true;
        igpu.dx11_capable = true;
        igpu.dx12_capable = true;
        igpu.driver_model = "WDDM (Mock)";
        igpu.is_active_display_adapter = true;
        igpu.provenance = {"mock", "Synthetic iGPU", true};
        config.gpus.push_back(igpu);

        config.render_gpu_index = 0;
        config.display_gpu_index = 1;
        config.offload_method = RenderOffloadMethod::VulkanOffload;
        config.available_apis = {GraphicsAPI::Vulkan, GraphicsAPI::OpenGL};
        config.status = common::CapabilityState::Partial;
        config.status_reason = "Synthetic multi-GPU offload configuration.";
        config.required_env_vars = platform::recommended_linux_environment(config);
        config.provenance = {"mock", "Synthetic render-path recommendation", true};
        result.config = std::move(config);
        return result;
    }

    RenderPathResult analyze_config(const MultiGPUConfig& config) const override {
        return analyze(config, true);
    }

    RenderPathResult get_recommended_config() const override {
        return detect_gpus();
    }

    RenderPathOperationResult plan_config() const override {
        return plan_from_detection(detect_gpus());
    }

    RenderPathOperationResult apply_config(bool approved) override {
        return deployment_not_implemented(approved);
    }

    RenderPathOperationResult verify_config() const override {
        RenderPathOperationResult result;
        result.status = common::CapabilityState::Unknown;
        result.message = "Mock render-path deployment is not persisted.";
        result.provenance = {"mock", "No real system mutation", true};
        result.deployment = RenderPathDeployment{};
        return result;
    }

    RenderPathOperationResult get_deployment_state() const override {
        return verify_config();
    }

    RenderPathOperationResult rollback() override {
        return not_implemented("ROLLBACK_NOT_IMPLEMENTED",
                               "Mock render-path rollback is not persisted.");
    }

    RenderPathOperationResult get_app_preference(const std::string& app_path) const override {
        if (app_path.empty()) return not_implemented("INVALID_PATH", "Application path required.");
        return not_implemented("NOT_IMPLEMENTED",
                               "Mock per-application preference is not persisted.");
    }

    RenderPathOperationResult set_app_preference(
        const std::string& app_path, const std::string& gpu_id, bool approved) override {
        if (app_path.empty() || gpu_id.empty())
            return not_implemented("INVALID_ARGUMENT", "Application path and GPU ID required.");
        if (!approved)
            return not_implemented("APPROVAL_REQUIRED",
                                   "Explicit approval required before changing app preference.");
        return not_implemented("NOT_IMPLEMENTED",
                               "Mock per-application preference is not persisted.");
    }

private:
    static RenderPathResult analyze(const MultiGPUConfig& config, bool synthetic) {
        RenderPathResult result;
        result.provenance = {synthetic ? "mock" : "real",
                             "Role analysis derived from detected observations",
                             synthetic};
        if (config.gpus.empty()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "NO_GPUS_DETECTED";
            result.message = "No GPUs detected.";
            return result;
        }

        bool compute = false;
        bool display = false;
        bool render = false;
        bool tcc_block = false;
        for (const auto& gpu : config.gpus) {
            compute = compute || gpu.cuda_capable || gpu.role == GPURole::ComputeOnly ||
                      gpu.role == GPURole::ComputeAndDisplay;
            display = display || gpu.has_display_outputs ||
                      gpu.is_active_display_adapter || gpu.role == GPURole::DisplayOnly;
            render = render || gpu.is_render_target;
            tcc_block = tcc_block || (gpu.driver_model == "TCC" && gpu.vendor_id == 0x10DE);
        }

        if (!compute) {
            result.status = common::CapabilityState::Unsupported;
            result.message = "No compute-capable GPU detected.";
        } else if (tcc_block && config.render_gpu_index.has_value()) {
            result.status = common::CapabilityState::Partial;
            result.message = "NVIDIA render target is in TCC mode; graphics presentation is not asserted.";
        } else if (compute && display && render) {
            result.status = common::CapabilityState::Ready;
            result.message = "A render-capable multi-GPU path is available.";
        } else if (compute && !display) {
            result.status = common::CapabilityState::Partial;
            result.message = "Compute GPU detected without an active display adapter.";
        } else {
            result.status = common::CapabilityState::Partial;
            result.message = "GPU roles were detected, but a complete render path could not be established.";
        }

        result.config = config;
        return result;
    }

    static RenderPathOperationResult plan_from_detection(const RenderPathResult& detection) {
        RenderPathOperationResult result;
        result.provenance = detection.provenance;
        if (!detection.config.has_value()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "DETECTION_FAILED";
            result.message = "Cannot create render-path plan without detection data.";
            return result;
        }

        const auto& config = *detection.config;
        std::ostringstream msg;
        msg << "Render Path Configuration Plan (dry-run):\n";
        msg << "Detected GPUs: " << config.gpus.size() << "\n";
        for (std::size_t i = 0; i < config.gpus.size(); ++i) {
            const auto& gpu = config.gpus[i];
            msg << "  [" << i << "] " << gpu.name
                << " | role=" << to_string(gpu.role)
                << " | PCI=" << (gpu.pci_bus.empty() ? "Unknown" : gpu.pci_bus)
                << " | driver=" << (gpu.driver_model.empty() ? "Unknown" : gpu.driver_model)
                << "\n";
        }

        if (config.render_gpu_index.has_value())
            msg << "Render GPU: [" << *config.render_gpu_index << "] "
                << config.gpus[*config.render_gpu_index].name << "\n";
        if (config.display_gpu_index.has_value())
            msg << "Display GPU: [" << *config.display_gpu_index << "] "
                << config.gpus[*config.display_gpu_index].name << "\n";
        msg << "Offload method: " << to_string(config.offload_method) << "\n";
        if (!config.required_env_vars.empty()) {
            msg << "Linux environment plan:\n";
            for (const auto& env : config.required_env_vars)
                msg << "  " << env.name << "=" << env.value << "\n";
        }
        if (!config.status_reason.empty())
            msg << "Status: " << config.status_reason << "\n";
        msg << "No system mutation is performed by this operation.\n";

        result.status = detection.status;
        result.message = msg.str();
        return result;
    }

    static RenderPathOperationResult deployment_not_implemented(bool approved) {
        if (!approved) return not_implemented("APPROVAL_REQUIRED",
                                               "Explicit approval required before applying render-path configuration.");
        return not_implemented("DEPLOYMENT_NOT_IMPLEMENTED",
                               "Mock mode never mutates the host render configuration.");
    }

    static RenderPathOperationResult not_implemented(const char* code, const char* message) {
        RenderPathOperationResult result;
        result.status = common::CapabilityState::Error;
        result.error_code = code;
        result.message = message;
        result.provenance = {"mock", "Synthetic provider", true};
        return result;
    }
};

class RealRenderPathManager final : public IRenderPathManager {
public:
    RenderPathResult detect_gpus() const override {
        RenderPathResult result;
        result.provenance = {"real", "NVML + Vulkan + OS display-adapter providers", false};

        auto nvml = providers::nvml::make_nvml_provider();
        if (!nvml) {
            result.status = common::CapabilityState::Unavailable;
            result.error_code = "NVML_UNAVAILABLE";
            result.message = "NVML provider is not available on this build.";
            return result;
        }

        const auto nvml_result = nvml->observe();
        if (!nvml_result.ok()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "NVML_QUERY_FAILED";
            result.message = nvml_result.status().message();
            return result;
        }

        const auto platform_snapshot = platform::detect_display_adapters();
        auto vulkan = providers::vulkan::make_vulkan_provider();
        providers::vulkan::RuntimeObservation vulkan_observation{};
        bool vulkan_ok = false;
        if (vulkan) {
            const auto vulkan_result = vulkan->observe();
            if (vulkan_result.ok()) {
                vulkan_observation = vulkan_result.value();
                vulkan_ok = true;
            }
        }

        MultiGPUConfig config;
        for (const auto& raw : nvml_result.value()) {
            GPUDescription gpu;
            gpu.name = raw.name.value.value_or("Unknown NVIDIA GPU");
            gpu.vendor = "NVIDIA";
            gpu.pci_bus = raw.pci_address.value ? raw.pci_address.value->bus_id : "";
            if (raw.pci_device_id.value) {
                gpu.vendor_id = raw.pci_device_id.value->vendor_id;
                gpu.device_id = raw.pci_device_id.value->device_id;
            } else {
                gpu.vendor_id = 0x10DE;
            }
            gpu.cuda_capable = true;
            gpu.opencl_capable = true;
            gpu.vulkan_capable = false;
            gpu.opengl_capable = false;
            gpu.driver_model = raw.driver_model.value.value_or("");
            gpu.has_display_outputs = raw.display_active.value.value_or(false);
            gpu.is_active_display_adapter = gpu.has_display_outputs;
            gpu.provenance = {"nvml", "NVML device observation", false};

            if (vulkan_ok && vulkan_observation.devices.value) {
                for (const auto& vk : *vulkan_observation.devices.value) {
                    if (vk.vendor_id.value.value_or(0) == gpu.vendor_id &&
                        vk.device_id.value.value_or(0) == gpu.device_id) {
                        gpu.vulkan_capable = true;
                        break;
                    }
                }
            }

            gpu.opengl_capable = gpu.vendor_id == 0x10DE;
            gpu.is_render_target = gpu.vulkan_capable || gpu.opengl_capable;

            const bool display_from_platform = matches_platform_display(gpu, platform_snapshot);
            gpu.has_display_outputs = gpu.has_display_outputs || display_from_platform;
            gpu.is_active_display_adapter = gpu.is_active_display_adapter || display_from_platform;

            if (gpu.has_display_outputs) {
                gpu.role = GPURole::ComputeAndDisplay;
            } else {
                gpu.role = GPURole::ComputeOnly;
            }

            if (gpu.driver_model == "TCC") {
                gpu.is_render_target = false;
            }

            config.gpus.push_back(std::move(gpu));
        }

        for (std::size_t i = 0; i < config.gpus.size(); ++i) {
            if (config.gpus[i].has_display_outputs ||
                config.gpus[i].is_active_display_adapter) {
                config.display_gpu_index = i;
                break;
            }
        }

        for (std::size_t i = 0; i < config.gpus.size(); ++i) {
            const auto& gpu = config.gpus[i];
            if (gpu.is_render_target && !gpu.has_display_outputs) {
                config.render_gpu_index = i;
                break;
            }
        }

        if (!config.render_gpu_index.has_value()) {
            for (std::size_t i = 0; i < config.gpus.size(); ++i) {
                if (config.gpus[i].is_render_target) {
                    config.render_gpu_index = i;
                    break;
                }
            }
        }

        if (config.render_gpu_index.has_value()) {
            const auto& render = config.gpus[*config.render_gpu_index];
            if (config.display_gpu_index.has_value() &&
                *config.display_gpu_index != *config.render_gpu_index) {
#if defined(__linux__)
                config.offload_method = RenderOffloadMethod::VulkanOffload;
                if (!render.vulkan_capable && render.opengl_capable)
                    config.offload_method = RenderOffloadMethod::PRIME;
#elif defined(_WIN32)
                if (render.driver_model == "TCC") {
                    config.offload_method = RenderOffloadMethod::Unsupported;
                } else {
                    config.offload_method = RenderOffloadMethod::Manual;
                }
#else
                config.offload_method = RenderOffloadMethod::Manual;
#endif
            } else {
                config.offload_method = RenderOffloadMethod::None;
            }
        }

        for (const auto& gpu : config.gpus) {
            if (gpu.vulkan_capable) config.available_apis.push_back(GraphicsAPI::Vulkan);
            if (gpu.opengl_capable) config.available_apis.push_back(GraphicsAPI::OpenGL);
            if (gpu.cuda_capable) config.available_apis.push_back(GraphicsAPI::CUDA);
            if (gpu.opencl_capable) config.available_apis.push_back(GraphicsAPI::OpenCL);
            if (gpu.dx11_capable) config.available_apis.push_back(GraphicsAPI::DirectX11);
            if (gpu.dx12_capable) config.available_apis.push_back(GraphicsAPI::DirectX12);
        }
        std::sort(config.available_apis.begin(), config.available_apis.end(),
                  [](GraphicsAPI a, GraphicsAPI b) {
                      return static_cast<int>(a) < static_cast<int>(b);
                  });
        config.available_apis.erase(
            std::unique(config.available_apis.begin(), config.available_apis.end()),
            config.available_apis.end());

        config.required_env_vars = platform::recommended_linux_environment(config);
        const auto analyzed = analyze_config(config);
        analyzed.config;
        result = analyzed;
        result.provenance = {"real", "NVML + Vulkan + OS display-adapter providers", false};
        return result;
    }

    RenderPathResult analyze_config(const MultiGPUConfig& config) const override {
        bool compute = false;
        bool display = false;
        bool render = false;
        bool tcc = false;
        for (const auto& gpu : config.gpus) {
            compute = compute || gpu.cuda_capable;
            display = display || gpu.has_display_outputs || gpu.is_active_display_adapter;
            render = render || gpu.is_render_target;
            tcc = tcc || (gpu.vendor_id == 0x10DE && gpu.driver_model == "TCC");
        }

        RenderPathResult result;
        result.provenance = {"real", "Role analysis over provider observations", false};
        result.config = config;
        if (!compute) {
            result.status = common::CapabilityState::Unsupported;
            result.message = "No compute-capable GPU was observed.";
        } else if (tcc && config.render_gpu_index.has_value()) {
            result.status = common::CapabilityState::Partial;
            result.message = "The selected NVIDIA GPU is in TCC mode; graphics rendering is not asserted.";
        } else if (compute && display && render) {
            result.status = common::CapabilityState::Ready;
            result.message = "Compute, display and render capabilities were observed.";
        } else if (compute && !display) {
            result.status = common::CapabilityState::Partial;
            result.message = "Compute GPU observed, but no active display adapter was identified.";
        } else {
            result.status = common::CapabilityState::Partial;
            result.message = "The available provider observations do not establish a complete render path.";
        }
        return result;
    }

    RenderPathResult get_recommended_config() const override { return detect_gpus(); }

    RenderPathOperationResult plan_config() const override {
        return plan_from_detection(detect_gpus());
    }

    RenderPathOperationResult apply_config(bool approved) override {
        if (!approved) {
            RenderPathOperationResult result;
            result.status = common::CapabilityState::Error;
            result.error_code = "APPROVAL_REQUIRED";
            result.message = "Explicit approval required before applying render-path configuration.";
            result.provenance = {"real", "No mutation performed", false};
            return result;
        }

        RenderPathOperationResult result;
        result.status = common::CapabilityState::Unsupported;
        result.error_code = "PLATFORM_DEPLOYMENT_NOT_IMPLEMENTED";
        result.message = "Detection and planning are real; host configuration deployment is intentionally not enabled until platform-specific persistence and rollback are implemented.";
        result.provenance = {"real", "Read-only real providers; no host mutation", false};
        return result;
    }

    RenderPathOperationResult verify_config() const override {
        RenderPathOperationResult result;
        const auto detection = detect_gpus();
        result.provenance = detection.provenance;
        if (!detection.config.has_value()) {
            result.status = detection.status;
            result.message = detection.message;
            return result;
        }
        result.status = common::CapabilityState::Unknown;
        result.message = "Current provider state was detected, but persistent render-path deployment cannot yet be verified.";
        RenderPathDeployment deployment;
        deployment.verification_message = result.message;
        result.deployment = deployment;
        return result;
    }

    RenderPathOperationResult get_deployment_state() const override {
        return verify_config();
    }

    RenderPathOperationResult rollback() override {
        RenderPathOperationResult result;
        result.status = common::CapabilityState::Unsupported;
        result.error_code = "ROLLBACK_NOT_IMPLEMENTED";
        result.message = "No persistent render-path mutation is performed by the real provider yet.";
        result.provenance = {"real", "Read-only real providers", false};
        return result;
    }

    RenderPathOperationResult get_app_preference(const std::string& app_path) const override {
        RenderPathOperationResult result;
        if (app_path.empty()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "INVALID_PATH";
            result.message = "Application path required.";
        } else {
            result.status = common::CapabilityState::Unknown;
            result.message = "Per-application preference detection is not yet persisted by a platform adapter.";
        }
        result.provenance = {"real", "Platform persistence boundary", false};
        return result;
    }

    RenderPathOperationResult set_app_preference(
        const std::string& app_path, const std::string& gpu_id, bool approved) override {
        RenderPathOperationResult result;
        result.provenance = {"real", "No host mutation performed", false};
        if (app_path.empty() || gpu_id.empty()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "INVALID_ARGUMENT";
            result.message = "Application path and GPU ID required.";
        } else if (!approved) {
            result.status = common::CapabilityState::Error;
            result.error_code = "APPROVAL_REQUIRED";
            result.message = "Explicit approval required before changing app preference.";
        } else {
            result.status = common::CapabilityState::Unsupported;
            result.error_code = "PLATFORM_DEPLOYMENT_NOT_IMPLEMENTED";
            result.message = "Per-application render preference persistence is not implemented yet.";
        }
        return result;
    }

private:
    static RenderPathOperationResult plan_from_detection(const RenderPathResult& detection) {
        RenderPathOperationResult result;
        result.provenance = detection.provenance;
        if (!detection.config.has_value()) {
            result.status = common::CapabilityState::Error;
            result.error_code = "DETECTION_FAILED";
            result.message = "Cannot create render-path plan without detection data.";
            return result;
        }
        const auto& config = *detection.config;
        std::ostringstream msg;
        msg << "Render Path Configuration Plan (dry-run):\n";
        for (std::size_t i = 0; i < config.gpus.size(); ++i) {
            const auto& gpu = config.gpus[i];
            msg << "[" << i << "] " << gpu.name
                << " | " << to_string(gpu.role)
                << " | PCI " << (gpu.pci_bus.empty() ? "Unknown" : gpu.pci_bus)
                << " | driver " << (gpu.driver_model.empty() ? "Unknown" : gpu.driver_model)
                << "\n";
        }
        if (config.render_gpu_index)
            msg << "Render GPU: [" << *config.render_gpu_index << "]\n";
        if (config.display_gpu_index)
            msg << "Display GPU: [" << *config.display_gpu_index << "]\n";
        msg << "Offload: " << to_string(config.offload_method) << "\n";
        for (const auto& env : config.required_env_vars)
            msg << env.name << "=" << env.value << "\n";
        msg << "Dry-run only; no host mutation performed.\n";
        result.status = detection.status;
        result.message = msg.str();
        return result;
    }
};

} // namespace

std::unique_ptr<IRenderPathManager> create_render_path_manager(bool mock_mode) {
    if (mock_mode) return std::make_unique<MockRenderPathManager>();
    return std::make_unique<RealRenderPathManager>();
}

} // namespace isb::graphics
