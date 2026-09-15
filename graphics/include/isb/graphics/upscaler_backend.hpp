#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace isb::graphics {

enum class UpscalerStatus {
    Ok,
    Unsupported,
    InvalidArgument,
    NotInitialized,
    Failed
};

enum class UpscalerQuality {
    UltraQuality,
    Quality,
    Balanced,
    Performance,
    UltraPerformance
};

struct Resolution {
    std::uint32_t width = 0;
    std::uint32_t height = 0;
};

struct UpscalerCapabilities {
    std::string backend_name;
    std::string backend_version;
    std::vector<Resolution> supported_input_resolutions;
    std::vector<Resolution> supported_output_resolutions;
    std::vector<UpscalerQuality> supported_quality_modes;
    bool supports_hdr = false;
    bool supports_frame_generation = false;
};

struct UpscalerFrame {
    std::uintptr_t color_resource = 0;
    std::uintptr_t motion_vectors = 0;
    std::uintptr_t depth = 0;
    std::uintptr_t exposure = 0;
    std::uint64_t frame_index = 0;
};

struct UpscalerOutput {
    std::uintptr_t resource = 0;
    std::uint64_t frame_index = 0;
};

class UpscalerBackend {
public:
    virtual ~UpscalerBackend() = default;

    virtual const char* name() const noexcept = 0;
    virtual const char* version() const noexcept = 0;
    virtual UpscalerCapabilities capabilities() const = 0;

    virtual UpscalerStatus initialize(Resolution input,
                                      Resolution output,
                                      UpscalerQuality quality) = 0;

    virtual UpscalerStatus submit_frame(const UpscalerFrame& frame) = 0;
    virtual UpscalerStatus get_output(UpscalerOutput& output) = 0;
    virtual UpscalerStatus synchronize() = 0;
    virtual void shutdown() noexcept = 0;
};

} // namespace isb::graphics
