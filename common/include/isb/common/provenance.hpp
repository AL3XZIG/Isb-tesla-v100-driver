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
    /// Concrete API/source that produced the fact, e.g. "cudaDeviceGetAttribute"
    /// or "NVML nvmlDeviceGetMemoryInfo". Empty when no single API owns the fact.
    std::string source;

    Provenance() = default;
    Provenance(std::string provider_, std::string detail_, bool synthetic_)
        : provider(std::move(provider_)), detail(std::move(detail_)),
          synthetic(synthetic_) {}
    Provenance(std::string provider_, std::string detail_, bool synthetic_,
               std::string source_)
        : provider(std::move(provider_)), detail(std::move(detail_)),
          synthetic(synthetic_), source(std::move(source_)) {}
};

} // namespace isb::common
