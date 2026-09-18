#include "isb/graphics/upscaler_types.hpp"

#include <cassert>
#include <iostream>
#include <sstream>

using namespace isb::graphics;

int test_upscaler_type_strings() {
    // Test to_string conversions
    assert(std::string(to_string(UpscalerType::Native)) == "Native");
    assert(std::string(to_string(UpscalerType::FSR)) == "FSR");
    assert(std::string(to_string(UpscalerType::XeSS)) == "XeSS");
    assert(std::string(to_string(UpscalerType::OptiScaler)) == "OptiScaler");
    assert(std::string(to_string(UpscalerType::Auto)) == "Auto");
    assert(std::string(to_string(UpscalerType::Unsupported)) == "Unsupported");
    
    // Test from_string conversions
    assert(from_string("Native") == UpscalerType::Native);
    assert(from_string("fsr") == UpscalerType::FSR);
    assert(from_string("XeSS") == UpscalerType::XeSS);
    assert(from_string("optiscaler") == UpscalerType::OptiScaler);
    assert(from_string("Auto") == UpscalerType::Auto);
    assert(from_string("unsupported") == UpscalerType::Unsupported);
    assert(from_string("invalid") == UpscalerType::Unsupported);  // Invalid returns Unsupported
    
    std::cout << "PASS: upscaler_type_strings\n";
    return 0;
}

int test_quality_preset_strings() {
    assert(std::string(to_string(QualityPreset::UltraQuality)) == "Ultra Quality");
    assert(std::string(to_string(QualityPreset::Quality)) == "Quality");
    assert(std::string(to_string(QualityPreset::Balanced)) == "Balanced");
    assert(std::string(to_string(QualityPreset::Performance)) == "Performance");
    assert(std::string(to_string(QualityPreset::UltraPerformance)) == "Ultra Performance");
    assert(std::string(to_string(QualityPreset::Custom)) == "Custom");
    
    std::cout << "PASS: quality_preset_strings\n";
    return 0;
}

int test_dlss_compatibility_strings() {
    assert(std::string(to_string(DLSSCompatibilityMode::Off)) == "Off");
    assert(std::string(to_string(DLSSCompatibilityMode::Auto)) == "Auto");
    assert(std::string(to_string(DLSSCompatibilityMode::OptiScaler)) == "OptiScaler");
    assert(std::string(to_string(DLSSCompatibilityMode::Experimental)) == "Experimental");
    assert(std::string(to_string(DLSSCompatibilityMode::Unsupported)) == "Unsupported");
    
    std::cout << "PASS: dlss_compatibility_strings\n";
    return 0;
}

int test_frame_generation_strings() {
    assert(std::string(to_string(FrameGenerationMode::Off)) == "Off");
    assert(std::string(to_string(FrameGenerationMode::Native)) == "Native");
    assert(std::string(to_string(FrameGenerationMode::OptiFG)) == "OptiFG");
    assert(std::string(to_string(FrameGenerationMode::Unsupported)) == "Unsupported");
    
    std::cout << "PASS: frame_generation_strings\n";
    return 0;
}

int test_manager_creation() {
    auto manager = create_upscaler_manager(true);
    assert(manager != nullptr);
    
    std::cout << "PASS: manager_creation\n";
    return 0;
}

int test_list_backends() {
    auto manager = create_upscaler_manager(true);
    auto backends = manager->list_backends();
    
    // Should have FSR, XeSS, OptiScaler, Native
    assert(backends.size() >= 4);
    
    bool found_fsr = false, found_xess = false, found_optiscaler = false;
    for (const auto& b : backends) {
        if (b.type == UpscalerType::FSR) {
            found_fsr = true;
            assert(b.requires_external);
            assert(b.external_component == "AMD FidelityFX SDK");
            assert(b.provenance.synthetic);
        }
        if (b.type == UpscalerType::XeSS) {
            found_xess = true;
            assert(!b.supported);  // V100 has limited XeSS support
        }
        if (b.type == UpscalerType::OptiScaler) {
            found_optiscaler = true;
            assert(b.supported);
            assert(b.requires_external);
        }
    }
    
    assert(found_fsr && "FSR backend should be listed");
    assert(found_xess && "XeSS backend should be listed");
    assert(found_optiscaler && "OptiScaler backend should be listed");
    
    std::cout << "PASS: list_backends\n";
    return 0;
}

int test_detect_backend() {
    auto manager = create_upscaler_manager(true);
    
    auto result = manager->detect_backend(UpscalerType::FSR);
    assert(result.status == UpscalerStatus::Ok);
    assert(result.capability.has_value());
    assert(result.capability->type == UpscalerType::FSR);
    assert(result.provenance.synthetic);
    
    // Test unsupported backend detection
    result = manager->detect_backend(UpscalerType::Unsupported);
    assert(result.status == UpscalerStatus::Unsupported);
    assert(result.error_code == "BACKEND_NOT_FOUND");
    
    std::cout << "PASS: detect_backend\n";
    return 0;
}

int test_plan_config_dry_run() {
    auto manager = create_upscaler_manager(true);
    
    UpscalerConfig config;
    config.backend = UpscalerType::FSR;
    config.quality = QualityPreset::Quality;
    config.sharpening.enabled = true;
    config.sharpening.strength = 0.2f;
    config.dlss_compat = DLSSCompatibilityMode::Off;
    config.frame_gen = FrameGenerationMode::Off;
    
    auto result = manager->plan_config(config);
    assert(result.status == UpscalerStatus::Ok);
    assert(result.message.find("Backend: FSR") != std::string::npos);
    assert(result.message.find("Quality: Quality") != std::string::npos);
    
    std::cout << "PASS: plan_config_dry_run\n";
    return 0;
}

int test_plan_config_with_dlss_spoof_warning() {
    auto manager = create_upscaler_manager(true);
    
    UpscalerConfig config;
    config.backend = UpscalerType::OptiScaler;
    config.dlss_compat = DLSSCompatibilityMode::OptiScaler;
    
    auto result = manager->plan_config(config);
    assert(result.status == UpscalerStatus::Ok);
    assert(result.message.find("DLSS Compatibility uses spoofing") != std::string::npos);
    assert(result.message.find("NOT native NVIDIA DLSS") != std::string::npos);
    
    std::cout << "PASS: plan_config_with_dlss_spoof_warning\n";
    return 0;
}

int test_apply_requires_approval() {
    auto manager = create_upscaler_manager(true);
    
    UpscalerConfig config;
    config.backend = UpscalerType::FSR;
    
    // Without approval
    auto result = manager->apply_config(config, false);
    assert(result.status == UpscalerStatus::Failed);
    assert(result.error_code == "APPROVAL_REQUIRED");
    
    // With approval (still fails because deployment not implemented)
    result = manager->apply_config(config, true);
    assert(result.status == UpscalerStatus::Failed);
    assert(result.error_code == "DEPLOYMENT_NOT_IMPLEMENTED");
    
    std::cout << "PASS: apply_requires_approval\n";
    return 0;
}

int test_backup_not_implemented() {
    auto manager = create_upscaler_manager(true);
    
    auto result = manager->create_backup("/mock/game/path");
    assert(result.status == UpscalerStatus::Failed);
    assert(result.error_code == "BACKUP_NOT_IMPLEMENTED");
    
    // Empty path should fail with invalid argument
    result = manager->create_backup("");
    assert(result.status == UpscalerStatus::InvalidArgument);
    assert(result.error_code == "INVALID_PATH");
    
    std::cout << "PASS: backup_not_implemented\n";
    return 0;
}

int test_rollback_not_implemented() {
    auto manager = create_upscaler_manager(true);
    
    auto result = manager->rollback("/mock/game/path");
    assert(result.status == UpscalerStatus::Failed);
    assert(result.error_code == "ROLLBACK_NOT_IMPLEMENTED");
    
    std::cout << "PASS: rollback_not_implemented\n";
    return 0;
}

int test_verify_deployment() {
    auto manager = create_upscaler_manager(true);
    
    auto result = manager->verify_deployment("/mock/game/path");
    assert(result.status == UpscalerStatus::Ok);
    assert(result.deployment.has_value());
    assert(!result.deployment->installed);
    assert(!result.deployment->verified);
    assert(!result.deployment->backup_available);
    
    std::cout << "PASS: verify_deployment\n";
    return 0;
}

int test_game_detection_boundary() {
    auto manager = create_upscaler_manager(true);
    
    // Non-existent path
    auto result = manager->detect_game("/nonexistent/path");
    assert(result.status == UpscalerStatus::InvalidArgument);
    assert(result.error_code == "GAME_PATH_INVALID");
    
    std::cout << "PASS: game_detection_boundary\n";
    return 0;
}

int test_recommendation_unavailable() {
    auto manager = create_upscaler_manager(true);
    
    GameDetectionResult game;
    game.game_id = "test-game";
    game.compatible = false;
    
    auto result = manager->recommend_backend(game);
    assert(result.status == UpscalerStatus::Unsupported);
    assert(result.error_code == "RECOMMENDATION_UNAVAILABLE");
    
    std::cout << "PASS: recommendation_unavailable\n";
    return 0;
}

int main() {
    int failures = 0;
    
    failures += test_upscaler_type_strings();
    failures += test_quality_preset_strings();
    failures += test_dlss_compatibility_strings();
    failures += test_frame_generation_strings();
    failures += test_manager_creation();
    failures += test_list_backends();
    failures += test_detect_backend();
    failures += test_plan_config_dry_run();
    failures += test_plan_config_with_dlss_spoof_warning();
    failures += test_apply_requires_approval();
    failures += test_backup_not_implemented();
    failures += test_rollback_not_implemented();
    failures += test_verify_deployment();
    failures += test_game_detection_boundary();
    failures += test_recommendation_unavailable();
    
    if (failures == 0) {
        std::cout << "\n=== All tests PASSED ===\n";
        return 0;
    } else {
        std::cerr << "\n=== " << failures << " tests FAILED ===\n";
        return 1;
    }
}
