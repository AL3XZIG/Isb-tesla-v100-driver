#include "isb/providers/nvml/observed.hpp"
#include "isb/providers/nvml/pci_ids.hpp"
#include "isb/providers/nvml/provider.hpp"

#include <cassert>
#include <cstdint>

int main() {
    const auto unknown = isb::providers::nvml::Observed<bool>::unknown();
    assert(!unknown.value.has_value());

    const auto reported = isb::providers::nvml::Observed<bool>::reported(false);
    assert(reported.value.has_value());
    assert(!*reported.value);

    const auto pci = isb::providers::nvml::decode_pci_device_id(0x10de1db6U);
    assert(pci.vendor_id == 0x10deU);
    assert(pci.device_id == 0x1db6U);

#ifndef ISB_HAS_NVML
    assert(isb::providers::nvml::make_nvml_provider() == nullptr);
#endif
}
