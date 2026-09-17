#include "isb/providers/cuda/provider.hpp"

#ifdef ISB_HAS_CUDA_RUNTIME
#include <cuda_runtime_api.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace isb::providers::cuda {
namespace {

Observed<std::uint32_t> query_runtime_version() {
    int version = 0;
    return cudaRuntimeGetVersion(&version) == cudaSuccess
        ? Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(version))
        : Observed<std::uint32_t>::unknown();
}

Observed<std::uint32_t> query_driver_version() {
    int version = 0;
    return cudaDriverGetVersion(&version) == cudaSuccess
        ? Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(version))
        : Observed<std::uint32_t>::unknown();
}

class CudaProvider final : public Provider {
public:
    common::Result<RuntimeObservation> observe() override {
        RuntimeObservation observation;
        observation.runtime_version = query_runtime_version();
        observation.driver_version = query_driver_version();

        int device_count = 0;
        const cudaError_t count_status = cudaGetDeviceCount(&device_count);
        if (count_status == cudaErrorNoDevice) {
            observation.devices = Observed<std::vector<CudaDeviceObservation>>::reported({});
            return observation;
        }
        if (count_status != cudaSuccess) {
            return common::Status::failure(common::ErrorCode::QueryFailed,
                                           cudaGetErrorString(count_status));
        }

        std::vector<CudaDeviceObservation> devices;
        devices.reserve(static_cast<std::size_t>(device_count));
        for (int index = 0; index < device_count; ++index) {
            cudaDeviceProp properties{};
            if (cudaGetDeviceProperties(&properties, index) != cudaSuccess) {
                devices.push_back({
                    Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(index)),
                    Observed<std::string>::unknown(),
                    Observed<std::uint32_t>::unknown(),
                    Observed<std::uint32_t>::unknown(),
                    Observed<std::uint64_t>::unknown(),
                });
                continue;
            }

            devices.push_back({
                Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(index)),
                Observed<std::string>::reported(properties.name),
                Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(properties.major)),
                Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(properties.minor)),
                Observed<std::uint64_t>::reported(static_cast<std::uint64_t>(properties.totalGlobalMem)),
            });
        }

        observation.devices = Observed<std::vector<CudaDeviceObservation>>::reported(std::move(devices));
        return observation;
    }
};

} // namespace

std::unique_ptr<Provider> make_cuda_provider() {
    return std::make_unique<CudaProvider>();
}

} // namespace isb::providers::cuda
#else
namespace isb::providers::cuda {

std::unique_ptr<Provider> make_cuda_provider() {
    return nullptr;
}

} // namespace isb::providers::cuda
#endif
