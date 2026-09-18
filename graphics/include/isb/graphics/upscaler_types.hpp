#pragma once

#include "isb/graphics/upscaler_backend.hpp"
#include "isb/common/provenance.hpp"

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace isb::graphics {

using isb::common::Provenance;

/// Supported upscaling backends for V100 compatibility layer
enum class UpscalerType {
    Native,      ///< Native game upscaler (if available)
    FSR,         ///< AMD FidelityFX Super Resolution
    XeSS,        ///< Intel Xe Super Sampling
    OptiScaler,  ///< OptiScaler compatibility layer
    Auto,        ///< Automatic selection based on capability detection
    Unsupported  ///< No compatible backend available
};

const char* to_string(UpscalerType type) noexcept;
UpscalerType from_string(const std::string& str) noexcept;

/// Quality preset mapping for upscalers
enum class QualityPreset {
    UltraQuality,
    Quality,
    Balanced,
    Performance,
    UltraPerformance,
    Custom
};

const char* to_string(QualityPreset preset) noexcept;

/// Sharpening level (0.0 - 1.0)
struct SharpeningConfig {
    float strength = 0.0f;  // 0.0 = off, 1.0 = maximum
    bool enabled = false;
};

/// DLSS compatibility/spoof mode
enum class DLSSCompatibilityMode {
    Off,        ///< No compatibility layer
    Auto,       ///< Automatic detection and enablement
    OptiScaler, ///< Use OptiScaler DLSS compatibility
    Experimental, ///< Experimental spoofing methods
    Unsupported ///< Not supported for this game/API
};

const char* to_string(DLSSCompatibilityMode mode) noexcept;

/// Frame generation mode
enum class FrameGenerationMode {
    Off,          ///< Disabled
    Native,       ///< Native game frame generation
    OptiFG,       ///< OptiScaler frame generation
    Unsupported   ///< Not supported
};

const char* to_string(FrameGenerationMode mode) noexcept;

/// Per-game upscaling configuration requested by user
struct UpscalerConfig {
    UpscalerType backend = UpscalerType::Auto;
    QualityPreset quality = QualityPreset::Quality;
    SharpeningConfig sharpening;
    DLSSCompatibilityMode dlss_compat = DLSSCompatibilityMode::Auto;
    FrameGenerationMode frame_gen = FrameGenerationMode::Off;
    
    /// Target resolution (0,0 = auto/game default)
    Resolution target_resolution{0, 0};
    
    /// Game/application identifier
    std::string game_id;
};

/// Backend capability information
struct BackendCapability {
    UpscalerType type;
    std::string name;
    std::string version;
    bool available = false;           ///< Backend is present
    bool supported = false;           ///< Supported on current hardware
    bool runtime_available = false;   ///< Runtime dependencies present
    bool requires_external = false;   ///< Needs external component
    bool experimental = false;        ///< Experimental/unsupported
    std::string external_component;   ///< Name of required external component
    Provenance provenance;            ///< Source of this capability info
};

/// Detection result for a specific game
struct GameDetectionResult {
    std::string game_id;
    std::string game_name;
    std::string executable_path;
    std::string graphics_api;       ///< D3D12, Vulkan, etc.
    
    /// Detected upscalers in game
    struct DetectedUpscaler {
        std::string name;
        std::string dll_path;
        std::string version;
    };
    std::vector<DetectedUpscaler> detected_upscalers;
    
    /// Detected anti-cheat
    bool anti_cheat_present = false;
    std::string anti_cheat_name;
    
    /// Compatibility assessment
    bool compatible = false;
    std::string compatibility_reason;
};

/// Deployment state for upscaler backend
struct DeploymentState {
    bool installed = false;
    std::string version;
    std::string path;
    bool backup_available = false;
    std::string backup_path;
    bool verified = false;
    std::string verification_message;
};

/// Result of upscaler operation
struct UpscalerResult {
    UpscalerStatus status;
    std::string message;
    std::string error_code;
    Provenance provenance;
    
    /// Detailed state after operation
    std::optional<BackendCapability> capability;
    std::optional<DeploymentState> deployment;
};

/// Manager interface for upscaler backends
class IUpscalerManager {
public:
    virtual ~IUpscalerManager() = default;
    
    /// Get list of available backends
    virtual std::vector<BackendCapability> list_backends() const = 0;
    
    /// Detect capabilities for a specific backend
    virtual UpscalerResult detect_backend(UpscalerType type) const = 0;
    
    /// Detect game and its upscaling requirements
    virtual UpscalerResult detect_game(const std::string& game_path) const = 0;
    
    /// Get recommended backend for a game
    virtual UpscalerResult recommend_backend(const GameDetectionResult& game) const = 0;
    
    /// Plan configuration changes (dry-run)
    virtual UpscalerResult plan_config(const UpscalerConfig& config) const = 0;
    
    /// Apply configuration
    virtual UpscalerResult apply_config(const UpscalerConfig& config, bool approved) = 0;
    
    /// Create backup before modification
    virtual UpscalerResult create_backup(const std::string& game_path) = 0;
    
    /// Rollback to previous state
    virtual UpscalerResult rollback(const std::string& game_path) = 0;
    
    /// Verify deployment state
    virtual UpscalerResult verify_deployment(const std::string& game_path) const = 0;
    
    /// Get deployment state
    virtual UpscalerResult get_deployment_state(const std::string& game_path) const = 0;
};

/// Factory function to create upscaler manager
std::unique_ptr<IUpscalerManager> create_upscaler_manager(bool mock_mode = false);

} // namespace isb::graphics
