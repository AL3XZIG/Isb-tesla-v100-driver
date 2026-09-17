#include "isb/graphics/report.hpp"

#include <sstream>

namespace isb::graphics {
namespace {

void add_if_not_available(std::vector<UnsupportedFeature>& features,
                          cal::CapabilityState state,
                          const char* feature,
                          const char* unavailable_reason) {
    if (state != cal::CapabilityState::Available) {
        features.push_back({feature, state == cal::CapabilityState::Unknown
            ? "No provider or backend observation is available."
            : unavailable_reason});
    }
}

} // namespace

GraphicsReport::GraphicsReport(const GraphicsAdapter& adapter)
    : gpu_identity_(adapter.gpu_identity()), capability_(adapter.capability()), backends_(adapter.backends()) {
    add_if_not_available(unsupported_features_, capability_.rt_cores, "Ray Tracing",
                         "No RT Core capability was reported.");
    add_if_not_available(unsupported_features_, capability_.optical_flow, "Optical Flow",
                         "No NVIDIA Optical Flow Accelerator capability was reported.");
    if (capability_.optical_flow != cal::CapabilityState::Available) {
        unsupported_features_.push_back({"DLSS Frame Generation",
            "No NVIDIA Optical Flow Accelerator capability was reported."});
    }
}

const std::string& GraphicsReport::gpu_identity() const noexcept { return gpu_identity_; }
const std::vector<ObservedGraphicsBackend>& GraphicsReport::backends() const noexcept { return backends_; }
const std::vector<UnsupportedFeature>& GraphicsReport::unsupported_features() const noexcept { return unsupported_features_; }

std::string GraphicsReport::to_text() const {
    std::ostringstream out;
    out << gpu_identity_ << "\n\n";
    for (const auto& backend : backends_) {
        out << backend.name() << ": " << (backend.available() ? "AVAILABLE" : "UNAVAILABLE") << "\n";
    }
    out << "Tensor Core: " << cal::to_string(capability_.tensor_cores) << "\n";
    for (const auto& feature : unsupported_features_) {
        out << feature.feature << ": NOT SUPPORTED\nReason: " << feature.reason << "\n";
    }
    return out.str();
}

std::string GraphicsReport::to_json() const {
    std::ostringstream out;
    out << "{\"gpu_identity\":\"" << gpu_identity_ << "\",\"capabilities\":"
        << isb::graphics::to_json(capability_) << ",\"backends\":[";
    for (std::size_t i = 0; i < backends_.size(); ++i) {
        if (i != 0) out << ',';
        out << "{\"type\":\"" << to_string(backends_[i].type()) << "\",\"available\":"
            << (backends_[i].available() ? "true" : "false") << '}';
    }
    out << "]}";
    return out.str();
}

} // namespace isb::graphics
