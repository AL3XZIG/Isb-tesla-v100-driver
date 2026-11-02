#pragma once

#include "isb/hub/hub.hpp"
#include "isb/hub/runtime_provider.hpp"

#include <memory>

namespace isb::cli {

inline std::unique_ptr<hub::Provider> make_provider(bool mock) {
    if (mock) return std::make_unique<hub::MockProvider>();
    return hub::make_runtime_provider();
}

} // namespace isb::cli
