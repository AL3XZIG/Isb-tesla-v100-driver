#pragma once

#include "isb/benchmarks/cuda_check.hpp"
#include "isb/benchmarks/result.hpp"

#include <string>
#include <vector>

namespace isb::benchmarks {

/// Create a human-readable report from CUDA runtime facts and verification results.
std::string make_report(const CudaCheck& cuda, const std::vector<BenchmarkResult>& results);

} // namespace isb::benchmarks
