#pragma once

#include "isb/cal/capabilities.hpp"

namespace isb::cal {

/// Build the provider-neutral hardware capability baseline for a Tesla V100.
///
/// This function is declarative only: it performs no device probing and makes
/// no claims about the capabilities exposed by a particular driver/runtime.
GpuCapabilities make_v100_capabilities(GpuVariant variant);

} // namespace isb::cal
