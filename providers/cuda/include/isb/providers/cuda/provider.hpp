#pragma once

#include "isb/common/error.hpp"
#include "isb/providers/cuda/observation.hpp"

#include <memory>

namespace isb::providers::cuda {

class Provider {
public:
    virtual ~Provider() = default;
    virtual common::Result<RuntimeObservation> observe() = 0;
};

std::unique_ptr<Provider> make_cuda_provider();

} // namespace isb::providers::cuda
