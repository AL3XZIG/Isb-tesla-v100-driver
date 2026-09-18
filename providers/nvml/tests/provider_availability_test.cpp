#include "isb/providers/nvml/hub_provider.hpp"

#include <cassert>

int main() {
    auto provider = isb::providers::nvml::make_nvml_provider();

#ifndef ISB_HAS_NVML
    assert(!provider);
#endif

    isb::providers::nvml::HubProvider hub_provider;
    const auto telemetry = hub_provider.telemetry();

#ifndef ISB_HAS_NVML
    assert(telemetry.driver == "unknown");
    assert(telemetry.performance_state == "unknown");
    assert(telemetry.ecc == "unknown");
    assert(telemetry.pcie == "unknown");
    assert(telemetry.nvlink == "unknown");
    assert(!telemetry.synthetic);
#endif

    const auto controls = hub_provider.controls();
    assert(!controls.empty());

    return 0;
}
