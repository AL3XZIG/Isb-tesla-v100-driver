#include "isb/providers/vulkan/observation.hpp"
#include "isb/providers/vulkan/provider.hpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    isb::providers::vulkan::RuntimeObservation observation;

    assert(!observation.instance_api_version.value.has_value());
    assert(!observation.devices.value.has_value());

    observation.devices =
        isb::providers::vulkan::Observed<std::vector<isb::providers::vulkan::PhysicalDeviceObservation>>::reported({});
    assert(observation.devices.value.has_value());
    assert(observation.devices.value->empty());

    const auto provider = isb::providers::vulkan::make_vulkan_provider();
#if defined(ISB_HAS_VULKAN)
    assert(provider != nullptr);
#else
    assert(provider == nullptr);
#endif

    return 0;
}
