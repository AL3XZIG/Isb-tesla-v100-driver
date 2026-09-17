#pragma once

#include "isb/common/observed.hpp"

namespace isb::providers::vulkan {

template <typename T>
using Observed = common::Observed<T, common::SourceKind::Vulkan>;

} // namespace isb::providers::vulkan
