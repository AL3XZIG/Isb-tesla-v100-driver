#include "isb/compute/context.hpp"
#include "isb/compute/device.hpp"
#include "isb/compute/kernel.hpp"
#include "isb/compute/report.hpp"

#include <cstdlib>
#include <string>

namespace {

#define CHECK(condition) do { if (!(condition)) std::abort(); } while (false)

isb::cal::GpuCapabilities v100_capabilities() {
    isb::cal::GpuCapabilities capabilities;
    capabilities.identity.model_name = "Tesla V100";
    capabilities.identity.compute_capability = {7, 0};
    capabilities.compute.cuda_state = isb::cal::CapabilityState::Available;
    capabilities.compute.tensor_cores.state = isb::cal::CapabilityState::Available;
    capabilities.hardware.tensor_cores.state = isb::cal::CapabilityState::Available;
    capabilities.hardware.rt_cores.state = isb::cal::CapabilityState::Unavailable;
    return capabilities;
}

void test_default_state() {
    const isb::compute::ComputeDevice device;
    CHECK(device.name().empty());
    CHECK(!device.sm_version());
    CHECK(!device.memory());
    CHECK(device.cuda_availability() == isb::cal::CapabilityState::Unknown);
    CHECK(device.tensor_capability() == isb::cal::CapabilityState::Unknown);

    const auto kernels = isb::compute::kernel_capability(device);
    CHECK(kernels.fp32 == isb::cal::CapabilityState::Unknown);
    CHECK(kernels.tensor_fp16 == isb::cal::CapabilityState::Unknown);
}

void test_unknown_state() {
    const auto device = isb::compute::ComputeDevice::from_capabilities({});
    const std::string report = isb::compute::make_report(device);
    CHECK(report.find("Unknown GPU\nCompute:\nUNKNOWN") == 0);
    CHECK(report.find("CUDA:\nUNKNOWN") != std::string::npos);
    CHECK(report.find("RayTracing:\nUNKNOWN") != std::string::npos);

    isb::compute::ComputeContext context;
    CHECK(!context.initialize());
    CHECK(!context.available());
}

void test_serialization_and_report() {
    const auto device = isb::compute::ComputeDevice::from_capabilities(
        v100_capabilities(), isb::compute::DeviceMemoryInfo{16ULL * 1024ULL * 1024ULL * 1024ULL,
                                                              12ULL * 1024ULL * 1024ULL * 1024ULL,
                                                              4ULL * 1024ULL * 1024ULL * 1024ULL});
    CHECK(device.sm_version() && device.sm_version()->major == 7);
    CHECK(device.memory() && device.memory()->total == 16ULL * 1024ULL * 1024ULL * 1024ULL);

    const std::string report = isb::compute::make_report(device);
    CHECK(report == "Tesla V100\nCompute:\nAVAILABLE\nCUDA:\nYES\nFP16:\nYES\n"
                     "Tensor:\nYES\nRayTracing:\nNO\n");

    const std::string json = isb::compute::to_json(device);
    CHECK(json.find("\"name\":\"Tesla V100\"") != std::string::npos);
    CHECK(json.find("\"sm_version\":{\"major\":7,\"minor\":0}") != std::string::npos);
    CHECK(json.find("\"memory\":{\"total\":17179869184,\"free\":12884901888,\"used\":4294967296}") != std::string::npos);
    CHECK(json.find("\"tensor_fp16\":\"available\"") != std::string::npos);
    CHECK(json == isb::compute::to_json(device));
}

void test_backend_enumeration() {
    using isb::compute::Backend;
    const Backend backends[] = {Backend::CUDA, Backend::VulkanCompute, Backend::OpenCL};
    CHECK(backends[0] != backends[1]);
    CHECK(backends[1] != backends[2]);

    isb::compute::ComputeContext cuda(Backend::CUDA, isb::cal::CapabilityState::Available);
    CHECK(cuda.initialize());
    CHECK(cuda.available());
    cuda.shutdown();
    CHECK(!cuda.available());
}

} // namespace

int main() {
    test_default_state();
    test_unknown_state();
    test_serialization_and_report();
    test_backend_enumeration();
    return 0;
}
