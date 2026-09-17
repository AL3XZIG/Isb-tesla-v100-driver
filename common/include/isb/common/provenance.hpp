#pragma once

namespace isb::common {

enum class SourceKind {
    Unknown,
    Nvml,
    Cuda,
};

enum class Confidence {
    Unknown,
    Reported,
};

} // namespace isb::common
