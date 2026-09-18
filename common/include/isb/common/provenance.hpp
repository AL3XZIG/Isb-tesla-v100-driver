#pragma once

#include <string>

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

/// Provenance metadata for capability/observation data
struct Provenance {
    std::string provider;      ///< Source of this data (e.g., \"mock\", \"nvml\", \"vulkan\")
    std::string detail;        ///< Additional context
    bool synthetic = false;    ///< true if this is mock/demo/boundary data
};

} // namespace isb::common
