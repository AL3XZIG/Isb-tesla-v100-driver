#include "isb/providers/nvml/observation.hpp"

#include <cassert>
#include <cstdint>

int main() {
    isb::providers::nvml::RawGpuObservation observation;

    assert(!observation.name.value.has_value());
    assert(!observation.memory_total_bytes.value.has_value());
    assert(!observation.power_limit_milliwatts.value.has_value());
    assert(!observation.temperature_celsius.value.has_value());
    assert(!observation.ecc_enabled.value.has_value());
    assert(!observation.pcie_generation.value.has_value());
    assert(!observation.nvlink_links.value.has_value());

    observation.power_limit_milliwatts =
        isb::providers::nvml::Observed<std::uint32_t>::reported(250000);
    assert(observation.power_limit_milliwatts.value.has_value());
    assert(*observation.power_limit_milliwatts.value == 250000);

    return 0;
}
