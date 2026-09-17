#pragma once

namespace isb::common {

enum class SourceKind {
    Unknown,
    Nvml,
    Vulkan,
    Cuda,
};

enum class Confidence {
    Unknown,
    Reported,
};

} // namespace isb::common
