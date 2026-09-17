#include "isb/benchmarks/benchmark.hpp"
#include "isb/benchmarks/cuda_check.hpp"

#include <chrono>
#include <utility>

#if defined(ISB_BENCHMARKS_HAS_CUDA)
#include <cuda_runtime_api.h>
#endif

namespace isb::benchmarks {
namespace {

class CapabilityCheck final : public IBenchmark {
public:
    enum class Kind { FP32, FP16, Tensor };

    CapabilityCheck(std::string name, Kind kind)
        : kind_(kind) {
        result_.name = std::move(name);
        result_.backend = "CUDA";
    }

    void initialize() override {
        cuda_ = check_cuda_runtime();
        initialized_ = true;
    }

    void run() override {
        const auto start = std::chrono::steady_clock::now();
        if (!initialized_) {
            result_.available = VerificationState::Failed;
            result_.error = "initialize() must be called before run()";
        } else {
            set_result();
        }
        result_.duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::steady_clock::now() - start);
    }

    const BenchmarkResult& result() const override { return result_; }

    void shutdown() override { initialized_ = false; }

private:
    void set_result() {
        result_.metadata["device_count"] = std::to_string(cuda_.device_count);
        if (!cuda_.device_name.empty()) result_.metadata["device_name"] = cuda_.device_name;
        if (cuda_.state != VerificationState::Verified) {
            result_.available = cuda_.state;
            result_.error = cuda_.error;
            return;
        }

        const int capability = cuda_.compute_capability_major * 10 + cuda_.compute_capability_minor;
        result_.metadata["compute_capability"] = std::to_string(cuda_.compute_capability_major) + "." +
                                                    std::to_string(cuda_.compute_capability_minor);
        const bool supported = (kind_ == Kind::FP32) ? capability >= 10
                             : (kind_ == Kind::FP16) ? capability >= 53
                                                      : capability >= 70;
        result_.available = supported ? VerificationState::Verified : VerificationState::Unavailable;
        if (!supported) result_.error = "not supported by this compute capability";
    }

    Kind kind_;
    bool initialized_ = false;
    CudaCheck cuda_;
    BenchmarkResult result_;
};

std::unique_ptr<IBenchmark> make_check(const char* name, CapabilityCheck::Kind kind) {
    return std::make_unique<CapabilityCheck>(name, kind);
}

} // namespace

const char* to_string(VerificationState state) noexcept {
    switch (state) {
    case VerificationState::Unknown: return "UNKNOWN";
    case VerificationState::Verified: return "VERIFIED";
    case VerificationState::Unavailable: return "UNAVAILABLE";
    case VerificationState::Failed: return "FAILED";
    }
    return "UNKNOWN";
}

CudaCheck check_cuda_runtime() {
#if defined(ISB_BENCHMARKS_HAS_CUDA)
    int count = 0;
    const cudaError_t count_status = cudaGetDeviceCount(&count);
    if (count_status != cudaSuccess) {
        return {VerificationState::Unavailable, {}, 0, 0, 0, cudaGetErrorString(count_status)};
    }
    if (count == 0) return {VerificationState::Unavailable, {}, 0, 0, 0, "no CUDA devices found"};

    cudaDeviceProp device{};
    const cudaError_t property_status = cudaGetDeviceProperties(&device, 0);
    if (property_status != cudaSuccess) {
        return {VerificationState::Failed, {}, 0, 0, static_cast<std::size_t>(count),
                cudaGetErrorString(property_status)};
    }
    return {VerificationState::Verified, device.name, device.major, device.minor,
            static_cast<std::size_t>(count), {}};
#else
    return {VerificationState::Unknown, {}, 0, 0, 0,
            "CUDA backend was not built"};
#endif
}

std::unique_ptr<IBenchmark> make_fp32_check() { return make_check("FP32", CapabilityCheck::Kind::FP32); }
std::unique_ptr<IBenchmark> make_fp16_check() { return make_check("FP16", CapabilityCheck::Kind::FP16); }
std::unique_ptr<IBenchmark> make_tensor_capability_check() {
    return make_check("Tensor", CapabilityCheck::Kind::Tensor);
}

} // namespace isb::benchmarks
