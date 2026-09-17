#pragma once

#include "isb/benchmarks/result.hpp"

#include <cstddef>
#include <string>

namespace isb::benchmarks {

/// Runtime facts needed by the small capability checks. No workload is run.
struct CudaCheck {
    VerificationState state = VerificationState::Unknown;
    std::string device_name;
    int compute_capability_major = 0;
    int compute_capability_minor = 0;
    std::size_t device_count = 0;
    std::string error;
};

/// Query CUDA Runtime availability and the first device's identity.
/// Returns Unknown when this build has no CUDA backend.
CudaCheck check_cuda_runtime();

} // namespace isb::benchmarks
