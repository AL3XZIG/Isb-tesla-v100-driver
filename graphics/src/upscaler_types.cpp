#include "isb/graphics/upscaler_types.hpp"

namespace isb::graphics {

const char* to_string(UpscalerType type) noexcept {
    switch (type) {
        case UpscalerType::Native: return "Native";
        case UpscalerType::FSR: return "FSR";
        case UpscalerType::XeSS: return "XeSS";
        case UpscalerType::OptiScaler: return "OptiScaler";
        case UpscalerType::Auto: return "Auto";
        case UpscalerType::Unsupported: return "Unsupported";
    }
    return "Unknown";
}

UpscalerType from_string(const std::string& str) noexcept {
    if (str == "Native" || str == "native") return UpscalerType::Native;
    if (str == "FSR" || str == "fsr") return UpscalerType::FSR;
    if (str == "XeSS" || str == "xess") return UpscalerType::XeSS;
    if (str == "OptiScaler" || str == "optiscaler") return UpscalerType::OptiScaler;
    if (str == "Auto" || str == "auto") return UpscalerType::Auto;
    if (str == "Unsupported" || str == "unsupported") return UpscalerType::Unsupported;
    return UpscalerType::Unsupported;
}

const char* to_string(QualityPreset preset) noexcept {
    switch (preset) {
        case QualityPreset::UltraQuality: return "Ultra Quality";
        case QualityPreset::Quality: return "Quality";
        case QualityPreset::Balanced: return "Balanced";
        case QualityPreset::Performance: return "Performance";
        case QualityPreset::UltraPerformance: return "Ultra Performance";
        case QualityPreset::Custom: return "Custom";
    }
    return "Unknown";
}

const char* to_string(DLSSCompatibilityMode mode) noexcept {
    switch (mode) {
        case DLSSCompatibilityMode::Off: return "Off";
        case DLSSCompatibilityMode::Auto: return "Auto";
        case DLSSCompatibilityMode::OptiScaler: return "OptiScaler";
        case DLSSCompatibilityMode::Experimental: return "Experimental";
        case DLSSCompatibilityMode::Unsupported: return "Unsupported";
    }
    return "Unknown";
}

const char* to_string(FrameGenerationMode mode) noexcept {
    switch (mode) {
        case FrameGenerationMode::Off: return "Off";
        case FrameGenerationMode::Native: return "Native";
        case FrameGenerationMode::OptiFG: return "OptiFG";
        case FrameGenerationMode::Unsupported: return "Unsupported";
    }
    return "Unknown";
}

} // namespace isb::graphics
