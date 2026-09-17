#pragma once

#include "isb/common/observed.hpp"

namespace isb::providers::nvml {

template <typename T>
using Observed = common::Observed<T, common::SourceKind::Nvml>;

} // namespace isb::providers::nvml
