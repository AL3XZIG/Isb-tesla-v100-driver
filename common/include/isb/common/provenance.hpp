#pragma once

namespace isb::common {

enum class SourceKind {
    Unknown,
    Nvml,
    Vulkan,
};

enum class Confidence {
    Unknown,
    Reported,
};

} // namespace isb::common
