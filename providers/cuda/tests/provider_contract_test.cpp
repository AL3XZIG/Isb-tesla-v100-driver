#include "isb/providers/cuda/observation.hpp"
#include "isb/providers/cuda/provider.hpp"

#include <cassert>
#include <cstdint>

int main() {
    isb::providers::cuda::RuntimeObservation observation;

    assert(!observation.runtime_version.value.has_value());
    assert(!observation.driver_version.value.has_value());
    assert(!observation.devices.value.has_value());

    observation.runtime_version =
        isb::providers::cuda::Observed<std::uint32_t>::reported(12080);
    assert(observation.runtime_version.value.has_value());
    assert(*observation.runtime_version.value == 12080);

    observation.devices =
        isb::providers::cuda::Observed<std::vector<isb::providers::cuda::CudaDeviceObservation>>::reported({});
    assert(observation.devices.value.has_value());
    assert(observation.devices.value->empty());

    const auto provider = isb::providers::cuda::make_cuda_provider();
#if defined(ISB_HAS_CUDA_RUNTIME)
    assert(provider != nullptr);
#else
    assert(provider == nullptr);
#endif

    return 0;
}
