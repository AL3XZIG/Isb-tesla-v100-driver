#include "isb/graphics/adapter.hpp"
#include "isb/graphics/report.hpp"

#include <cassert>

int main() {
    using isb::cal::CapabilityState;
    using namespace isb::graphics;

    const GraphicsCapability defaults{};
    assert(defaults.cuda == CapabilityState::Unknown);
    assert(!defaults.sm_version.has_value());

    const auto serialized = to_json(defaults);
    assert(serialized.find("\"cuda\":\"unknown\"") != std::string::npos);
    assert(serialized.find("\"sm_version\":null") != std::string::npos);

    isb::cal::GpuCapabilities source;
    source.identity.model_name = "Test V100";
    source.compute.cuda_state = CapabilityState::Available;
    source.compute.compute_capability = {7, 0};
    source.hardware.tensor_cores.state = CapabilityState::Available;
    source.hardware.rt_cores.state = CapabilityState::Unavailable;
    source.hardware.optical_flow_accelerator.state = CapabilityState::Unavailable;

    GraphicsAdapter adapter(source);
    const auto backends = adapter.backends();
    assert(backends.size() == 3);
    assert(backends[0].type() == GraphicsBackend::CUDA);
    assert(backends[0].available());
    assert(backends[1].type() == GraphicsBackend::Vulkan);
    assert(!backends[1].available()); // Unknown never becomes a false positive.

    GraphicsReport report(adapter);
    assert(report.to_text().find("DLSS Frame Generation: NOT SUPPORTED") != std::string::npos);
    assert(report.to_json().find("\"gpu_identity\":\"Test V100\"") != std::string::npos);
    return 0;
}
