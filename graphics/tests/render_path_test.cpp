#include "isb/graphics/render_path_types.hpp"
#include "isb/common/capability_state.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

using namespace isb::graphics;
using isb::common::CapabilityState;

int test_gpu_role_strings() {
    // Test to_string conversions
    assert(std::string(to_string(GPURole::Unknown)) == "Unknown");
    assert(std::string(to_string(GPURole::ComputeOnly)) == "Compute Only");
    assert(std::string(to_string(GPURole::DisplayOnly)) == "Display Only");
    assert(std::string(to_string(GPURole::ComputeAndDisplay)) == "Compute and Display");
    
    // Test from_string conversions
    assert(from_gpu_role_string("Unknown") == GPURole::Unknown);
    assert(from_gpu_role_string("ComputeOnly") == GPURole::ComputeOnly);
    assert(from_gpu_role_string("Display Only") == GPURole::DisplayOnly);
    assert(from_gpu_role_string("compute_and_display") == GPURole::ComputeAndDisplay);
    assert(from_gpu_role_string("invalid") == GPURole::Unknown);
    assert(from_gpu_role_string("ComputeOnly") == GPURole::ComputeOnly);
    assert(from_gpu_role_string("Display Only") == GPURole::DisplayOnly);
    assert(from_gpu_role_string("compute_and_display") == GPURole::ComputeAndDisplay);
    assert(from_gpu_role_string("invalid") == GPURole::Unknown);
    
    std::cout << "PASS: gpu_role_strings\n";
    return 0;
}

int test_graphics_api_strings() {
    assert(std::string(to_string(GraphicsAPI::Unknown)) == "Unknown");
    assert(std::string(to_string(GraphicsAPI::Vulkan)) == "Vulkan");
    assert(std::string(to_string(GraphicsAPI::OpenGL)) == "OpenGL");
    assert(std::string(to_string(GraphicsAPI::DirectX11)) == "DirectX 11");
    assert(std::string(to_string(GraphicsAPI::DirectX12)) == "DirectX 12");
    assert(std::string(to_string(GraphicsAPI::CUDA)) == "CUDA");
    assert(std::string(to_string(GraphicsAPI::OpenCL)) == "OpenCL");
    
    assert(from_api_string("vulkan") == GraphicsAPI::Vulkan);
    assert(from_api_string("DX12") == GraphicsAPI::DirectX12);
    assert(from_api_string("cuda") == GraphicsAPI::CUDA);
    
    std::cout << "PASS: graphics_api_strings\n";
    return 0;
}

int test_render_offload_strings() {
    assert(std::string(to_string(RenderOffloadMethod::Unknown)) == "Unknown");
    assert(std::string(to_string(RenderOffloadMethod::None)) == "None");
    assert(std::string(to_string(RenderOffloadMethod::PRIME)) == "PRIME");
    assert(std::string(to_string(RenderOffloadMethod::VulkanOffload)) == "Vulkan Offload");
    assert(std::string(to_string(RenderOffloadMethod::Unsupported)) == "Unsupported");
    
    std::cout << "PASS: render_offload_strings\n";
    return 0;
}

int test_manager_creation() {
    auto manager = create_render_path_manager(true);
    assert(manager != nullptr);
    
    std::cout << "PASS: manager_creation\n";
    return 0;
}

int test_detect_gpus() {
    auto manager = create_render_path_manager(true);
    auto result = manager->detect_gpus();
    
    // Should have mock data with provenance marked as synthetic
    assert(result.provenance.synthetic);
    assert(result.status == CapabilityState::Unavailable);
    
    if (result.config.has_value()) {
        const auto& config = result.config.value();
        
        // Should have at least 2 GPUs in mock (V100 + iGPU)
        assert(config.gpus.size() >= 2);
        
        // First GPU should be V100 (compute-only, no display outputs)
        bool found_v100 = false;
        bool found_igpu = false;
        
        for (const auto& gpu : config.gpus) {
            if (gpu.role == GPURole::ComputeOnly && !gpu.has_display_outputs) {
                found_v100 = true;
                assert(gpu.vendor == "NVIDIA");
                assert(gpu.cuda_capable);
            }
            if (gpu.role == GPURole::DisplayOnly || gpu.has_display_outputs) {
                found_igpu = true;
                assert(gpu.is_active_display_adapter);
            }
        }
        
        assert(found_v100 && "Should detect V100 as compute-only GPU");
        assert(found_igpu && "Should detect iGPU as display GPU");
        
        // Render GPU should be V100 (index 0)
        assert(config.render_gpu_index.has_value());
        assert(config.render_gpu_index.value() == 0);
        
        // Display GPU should be iGPU (index 1)
        assert(config.display_gpu_index.has_value());
        assert(config.display_gpu_index.value() == 1);
    } else {
        std::cerr << "FAIL: No config in detection result\n";
        return 1;
    }
    
    std::cout << "PASS: detect_gpus\n";
    return 0;
}

int test_analyze_config() {
    auto manager = create_render_path_manager(true);
    auto detection = manager->detect_gpus();
    
    if (!detection.config.has_value()) {
        std::cerr << "FAIL: Detection returned no config\n";
        return 1;
    }
    
    auto result = manager->analyze_config(detection.config.value());
    assert(result.provenance.synthetic);
    assert(result.message.find("Configuration analysis") != std::string::npos);
    
    std::cout << "PASS: analyze_config\n";
    return 0;
}

int test_plan_config_dry_run() {
    auto manager = create_render_path_manager(true);
    auto result = manager->plan_config();
    
    assert(result.provenance.synthetic);
    assert(result.message.find("Render Path Configuration Plan") != std::string::npos);
    assert(result.message.find("dry-run") != std::string::npos);
    assert(result.message.find("Tesla V100") != std::string::npos);
    
    std::cout << "PASS: plan_config_dry_run\n";
    return 0;
}

int test_apply_requires_approval() {
    auto manager = create_render_path_manager(true);
    
    // Without approval
    auto result = manager->apply_config(false);
    assert(result.status == CapabilityState::Error);
    assert(result.error_code == "APPROVAL_REQUIRED");
    
    // With approval (still fails because deployment not implemented)
    result = manager->apply_config(true);
    assert(result.status == CapabilityState::Error);
    assert(result.error_code == "DEPLOYMENT_NOT_IMPLEMENTED");
    
    std::cout << "PASS: apply_requires_approval\n";
    return 0;
}

int test_verify_config() {
    auto manager = create_render_path_manager(true);
    auto result = manager->verify_config();
    
    assert(result.provenance.synthetic);
    assert(result.deployment.has_value());
    assert(!result.deployment->configured);
    assert(!result.deployment->verified);
    assert(!result.deployment->rollback_available);
    
    std::cout << "PASS: verify_config\n";
    return 0;
}

int test_get_deployment_state() {
    auto manager = create_render_path_manager(true);
    auto result = manager->get_deployment_state();
    
    assert(result.status == CapabilityState::Unknown);
    
    std::cout << "PASS: get_deployment_state\n";
    return 0;
}

int test_rollback_not_implemented() {
    auto manager = create_render_path_manager(true);
    auto result = manager->rollback();
    
    assert(result.status == CapabilityState::Error);
    assert(result.error_code == "ROLLBACK_NOT_IMPLEMENTED");
    
    std::cout << "PASS: rollback_not_implemented\n";
    return 0;
}

int test_app_preference_invalid_args() {
    auto manager = create_render_path_manager(true);
    
    // Empty path
    auto result = manager->get_app_preference("");
    assert(result.status == CapabilityState::Error);
    assert(result.error_code == "INVALID_PATH");
    
    // Set preference without approval
    result = manager->set_app_preference("/mock/app", "gpu0", false);
    assert(result.status == CapabilityState::Error);
    assert(result.error_code == "APPROVAL_REQUIRED");
    
    // Set preference with approval (fails - not implemented)
    result = manager->set_app_preference("/mock/app", "gpu0", true);
    assert(result.status == CapabilityState::Error);
    assert(result.error_code == "NOT_IMPLEMENTED");
    
    std::cout << "PASS: app_preference_invalid_args\n";
    return 0;
}

int test_v100_no_display_outputs() {
    auto manager = create_render_path_manager(true);
    auto result = manager->detect_gpus();
    
    if (!result.config.has_value()) {
        std::cerr << "FAIL: No config in detection result\n";
        return 1;
    }
    
    const auto& config = result.config.value();
    
    // Verify V100 is correctly modeled as having no display outputs
    bool found_v100 = false;
    for (const auto& gpu : config.gpus) {
        if (gpu.role == GPURole::ComputeOnly) {
            found_v100 = true;
            assert(!gpu.has_display_outputs && "V100 should NOT have display outputs");
            assert(gpu.is_render_target && "V100 should be render target");
            assert(!gpu.is_active_display_adapter && "V100 should NOT be display adapter");
        }
    }
    
    assert(found_v100 && "Should have V100 in config");
    
    std::cout << "PASS: v100_no_display_outputs\n";
    return 0;
}

int test_multi_gpu_separation() {
    auto manager = create_render_path_manager(true);
    auto result = manager->detect_gpus();
    
    if (!result.config.has_value()) {
        std::cerr << "FAIL: No config in detection result\n";
        return 1;
    }
    
    const auto& config = result.config.value();
    
    // Verify render and display GPUs are different
    assert(config.render_gpu_index.has_value());
    assert(config.display_gpu_index.has_value());
    assert(config.render_gpu_index.value() != config.display_gpu_index.value() &&
           "Render and display GPUs should be different in multi-GPU config");
    
    // Verify the selected GPUs have correct roles
    const auto& render_gpu = config.gpus[config.render_gpu_index.value()];
    const auto& display_gpu = config.gpus[config.display_gpu_index.value()];
    
    assert((render_gpu.role == GPURole::ComputeOnly || 
            render_gpu.role == GPURole::ComputeAndDisplay));
    assert((display_gpu.has_display_outputs || 
            display_gpu.role == GPURole::DisplayOnly));
    
    std::cout << "PASS: multi_gpu_separation\n";
    return 0;
}


int test_classification_scenarios() {
    auto manager = create_render_path_manager(true);

    MultiGPUConfig v100_igpu;
    GPUDescription v100;
    v100.name = "Tesla V100";
    v100.vendor = "NVIDIA";
    v100.vendor_id = 0x10DE;
    v100.device_id = 0x1DB6;
    v100.role = GPURole::ComputeOnly;
    v100.cuda_capable = true;
    v100.vulkan_capable = true;
    v100.is_render_target = true;
    v100_igpu.gpus.push_back(v100);

    GPUDescription igpu;
    igpu.name = "Integrated GPU";
    igpu.vendor = "Generic";
    igpu.vendor_id = 0x8086;
    igpu.role = GPURole::DisplayOnly;
    igpu.has_display_outputs = true;
    igpu.is_active_display_adapter = true;
    v100_igpu.gpus.push_back(igpu);
    v100_igpu.render_gpu_index = 0;
    v100_igpu.display_gpu_index = 1;
    assert(manager->analyze_config(v100_igpu).status == CapabilityState::Ready);

    MultiGPUConfig only_v100;
    only_v100.gpus.push_back(v100);
    only_v100.render_gpu_index = 0;
    assert(manager->analyze_config(only_v100).status == CapabilityState::Partial);

    MultiGPUConfig tcc;
    GPUDescription tcc_v100 = v100;
    tcc_v100.driver_model = "TCC";
    tcc_v100.is_render_target = false;
    tcc.gpus.push_back(tcc_v100);
    tcc.render_gpu_index = 0;
    tcc.display_gpu_index = 0;
    assert(manager->analyze_config(tcc).status == CapabilityState::Partial);

    std::cout << "PASS: classification_scenarios\n";
    return 0;
}

int main() {
    int failures = 0;
    
    failures += test_gpu_role_strings();
    failures += test_graphics_api_strings();
    failures += test_render_offload_strings();
    failures += test_manager_creation();
    failures += test_detect_gpus();
    failures += test_analyze_config();
    failures += test_plan_config_dry_run();
    failures += test_apply_requires_approval();
    failures += test_verify_config();
    failures += test_get_deployment_state();
    failures += test_rollback_not_implemented();
    failures += test_app_preference_invalid_args();
    failures += test_v100_no_display_outputs();
    failures += test_multi_gpu_separation();
    failures += test_classification_scenarios();
    
    if (failures == 0) {
        std::cout << "\n=== All tests PASSED ===\n";
        return 0;
    } else {
        std::cerr << "\n=== " << failures << " tests FAILED ===\n";
        return 1;
    }
}
