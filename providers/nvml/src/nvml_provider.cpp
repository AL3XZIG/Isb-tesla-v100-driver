#include "isb/providers/nvml/provider.hpp"
#include "isb/providers/nvml/pci_ids.hpp"

#ifdef ISB_HAS_NVML
#include <nvml.h>

#include <array>
#include <string>
#include <utility>

namespace isb::providers::nvml {
namespace {

class NvmlProvider final : public Provider {
public:
    common::Result<std::vector<RawGpuObservation>> observe() override {
        const nvmlReturn_t initialization = nvmlInit_v2();
        if (initialization != NVML_SUCCESS) {
            return common::Status::failure(common::ErrorCode::InitializationFailed,
                                           nvmlErrorString(initialization));
        }

        unsigned int device_count = 0;
        const nvmlReturn_t count_status = nvmlDeviceGetCount_v2(&device_count);
        if (count_status != NVML_SUCCESS) {
            nvmlShutdown();
            return common::Status::failure(common::ErrorCode::QueryFailed,
                                           nvmlErrorString(count_status));
        }

        std::vector<RawGpuObservation> observations;
        observations.reserve(device_count);
        for (unsigned int index = 0; index < device_count; ++index) {
            observations.push_back(observe_device(index));
        }
        nvmlShutdown();
        return observations;
    }

private:
    static RawGpuObservation observe_device(unsigned int index) {
        RawGpuObservation observation;
        observation.index = Observed<std::uint32_t>::reported(index);

        nvmlDevice_t device{};
        if (nvmlDeviceGetHandleByIndex_v2(index, &device) != NVML_SUCCESS) {
            return observation;
        }

        std::array<char, NVML_DEVICE_NAME_BUFFER_SIZE> name{};
        if (nvmlDeviceGetName(device, name.data(), static_cast<unsigned int>(name.size())) == NVML_SUCCESS) {
            observation.name = Observed<std::string>::reported(name.data());
        }

        nvmlPciInfo_t pci_info{};
        if (nvmlDeviceGetPciInfo(device, &pci_info) == NVML_SUCCESS) {
            PciAddress address;
            address.bus_id = pci_info.busId;
            address.domain = pci_info.domain;
            address.bus = pci_info.bus;
            address.device = pci_info.device;
            observation.pci_address = Observed<PciAddress>::reported(std::move(address));
            observation.pci_device_id =
                Observed<PciDeviceId>::reported(decode_pci_device_id(pci_info.pciDeviceId));
        }

        std::vector<NvLinkLinkObservation> links;
        links.reserve(NVML_NVLINK_MAX_LINKS);
        for (unsigned int link = 0; link < NVML_NVLINK_MAX_LINKS; ++link) {
            nvmlEnableState_t state = NVML_FEATURE_DISABLED;
            if (nvmlDeviceGetNvLinkState(device, link, &state) == NVML_SUCCESS) {
                links.push_back({link, Observed<bool>::reported(state == NVML_FEATURE_ENABLED)});
            } else {
                links.push_back({link, Observed<bool>::unknown()});
            }
        }
        observation.nvlink_links = Observed<std::vector<NvLinkLinkObservation>>::reported(std::move(links));
        return observation;
    }
};

} // namespace

std::unique_ptr<Provider> make_nvml_provider() {
    return std::make_unique<NvmlProvider>();
}

} // namespace isb::providers::nvml
#else
namespace isb::providers::nvml {

std::unique_ptr<Provider> make_nvml_provider() {
    return nullptr;
}

} // namespace isb::providers::nvml
#endif
