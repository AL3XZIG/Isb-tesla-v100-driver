#pragma once

#include "isb/providers/cuda/observed.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace isb::providers::cuda {

struct CudaDeviceObservation {
    Observed<std::uint32_t> index;
    Observed<std::string> name;
    Observed<std::uint32_t> compute_capability_major;
    Observed<std::uint32_t> compute_capability_minor;
    Observed<std::uint64_t> total_global_memory_bytes;
};

struct RuntimeObservation {
    Observed<std::uint32_t> runtime_version;
    Observed<std::uint32_t> driver_version;
    Observed<std::vector<CudaDeviceObservation>> devices;
};

} // namespace isb::providers::cuda
