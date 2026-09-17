#pragma once

#include "isb/common/error.hpp"
#include "isb/providers/vulkan/observation.hpp"

#include <memory>

namespace isb::providers::vulkan {

class Provider {
public:
    virtual ~Provider() = default;
    virtual common::Result<RuntimeObservation> observe() = 0;
};

std::unique_ptr<Provider> make_vulkan_provider();

} // namespace isb::providers::vulkan
