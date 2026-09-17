#pragma once

#include "isb/compute/device.hpp"
#include "isb/compute/kernel.hpp"

#include <string>

namespace isb::compute {

/// Produce a stable, human-readable capability summary.
std::string make_report(const ComputeDevice& device);

/// Serialize the compute capability view as deterministic UTF-8 JSON.
std::string to_json(const ComputeDevice& device);

} // namespace isb::compute
