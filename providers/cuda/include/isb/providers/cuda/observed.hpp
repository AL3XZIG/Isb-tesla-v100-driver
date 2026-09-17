#pragma once

#include "isb/common/observed.hpp"

namespace isb::providers::cuda {

template <typename T>
using Observed = common::Observed<T, common::SourceKind::Cuda>;

} // namespace isb::providers::cuda
