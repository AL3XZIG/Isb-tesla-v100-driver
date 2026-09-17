#pragma once

#include "isb/common/error.hpp"
#include "isb/providers/nvml/observation.hpp"

#include <memory>
#include <vector>

namespace isb::providers::nvml {

class Provider {
public:
    virtual ~Provider() = default;
    virtual common::Result<std::vector<RawGpuObservation>> observe() = 0;
};

/// Returns nullptr when NVML was not available when ISB was built.
std::unique_ptr<Provider> make_nvml_provider();

} // namespace isb::providers::nvml
