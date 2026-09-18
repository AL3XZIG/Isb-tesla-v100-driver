#include "isb/providers/nvml/provider.hpp"
#include "isb/providers/nvml/pci_ids.hpp"

#ifdef ISB_HAS_NVML
#include <nvml.h>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

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

        std::array<char, NVML_DEVICE_UUID_BUFFER_SIZE> uuid{};
        if (nvmlDeviceGetUUID(device, uuid.data(), static_cast<unsigned int>(uuid.size())) == NVML_SUCCESS) {
            observation.uuid = Observed<std::string>::reported(uuid.data());
        }

        std::array<char, NVML_DEVICE_VBIOS_VERSION_BUFFER_SIZE> vbios{};
        if (nvmlDeviceGetVbiosVersion(device, vbios.data(), static_cast<unsigned int>(vbios.size())) == NVML_SUCCESS) {
            observation.vbios_version = Observed<std::string>::reported(vbios.data());
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

            unsigned int generation = 0;
            if (nvmlDeviceGetCurrPcieLinkGeneration(device, &generation) == NVML_SUCCESS) {
                observation.pcie_generation = Observed<std::uint32_t>::reported(generation);
            }
            unsigned int width = 0;
            if (nvmlDeviceGetCurrPcieLinkWidth(device, &width) == NVML_SUCCESS) {
                observation.pcie_link_width = Observed<std::uint32_t>::reported(width);
            }
        }

        nvmlMemory_t memory{};
        if (nvmlDeviceGetMemoryInfo(device, &memory) == NVML_SUCCESS) {
            observation.memory_total_bytes = Observed<std::uint64_t>::reported(memory.total);
            observation.memory_used_bytes = Observed<std::uint64_t>::reported(memory.used);
            observation.memory_free_bytes = Observed<std::uint64_t>::reported(memory.free);
        }

        unsigned int temperature = 0;
        if (nvmlDeviceGetTemperature(device, NVML_TEMPERATURE_GPU, &temperature) == NVML_SUCCESS) {
            observation.temperature_celsius = Observed<std::uint32_t>::reported(temperature);
        }

        unsigned int power = 0;
        if (nvmlDeviceGetPowerUsage(device, &power) == NVML_SUCCESS) {
            observation.power_draw_milliwatts = Observed<std::uint32_t>::reported(power);
        }
        unsigned int power_limit = 0;
        if (nvmlDeviceGetEnforcedPowerLimit(device, &power_limit) == NVML_SUCCESS) {
            observation.power_limit_milliwatts = Observed<std::uint32_t>::reported(power_limit);
        }

        unsigned int clock = 0;
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_GRAPHICS, &clock) == NVML_SUCCESS) {
            observation.graphics_clock_mhz = Observed<std::uint32_t>::reported(clock);
        }
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_MEM, &clock) == NVML_SUCCESS) {
            observation.memory_clock_mhz = Observed<std::uint32_t>::reported(clock);
        }
        if (nvmlDeviceGetClockInfo(device, NVML_CLOCK_SM, &clock) == NVML_SUCCESS) {
            observation.sm_clock_mhz = Observed<std::uint32_t>::reported(clock);
        }

        nvmlUtilization_t utilization{};
        if (nvmlDeviceGetUtilizationRates(device, &utilization) == NVML_SUCCESS) {
            observation.gpu_utilization_percent = Observed<std::uint32_t>::reported(utilization.gpu);
            observation.memory_utilization_percent = Observed<std::uint32_t>::reported(utilization.memory);
        }

        nvmlEnableState_t display_active = NVML_FEATURE_DISABLED;
        if (nvmlDeviceGetDisplayActive(device, &display_active) == NVML_SUCCESS) {
            observation.display_active =
                Observed<bool>::reported(display_active == NVML_FEATURE_ENABLED);
        }

        nvmlEnableState_t persistence = NVML_FEATURE_DISABLED;
        if (nvmlDeviceGetPersistenceMode(device, &persistence) == NVML_SUCCESS) {
            observation.persistence_mode =
                Observed<bool>::reported(persistence == NVML_FEATURE_ENABLED);
        }

        nvmlComputeMode_t compute_mode = NVML_COMPUTEMODE_DEFAULT;
        if (nvmlDeviceGetComputeMode(device, &compute_mode) == NVML_SUCCESS) {
            observation.compute_mode =
                Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(compute_mode));
        }

        nvmlDriverModel_t current_driver_model = NVML_DRIVER_WDDM;
        nvmlDriverModel_t pending_driver_model = NVML_DRIVER_WDDM;
        if (nvmlDeviceGetDriverModel(device, &current_driver_model, &pending_driver_model) == NVML_SUCCESS) {
            const char* model = current_driver_model == NVML_DRIVER_TCC ? "TCC" :
                                current_driver_model == NVML_DRIVER_WDDM ? "WDDM" : "UNKNOWN";
            observation.driver_model = Observed<std::string>::reported(model);
        }

        nvmlPstates_t performance_state = NVML_PSTATE_UNKNOWN;
        if (nvmlDeviceGetPerformanceState(device, &performance_state) == NVML_SUCCESS) {
            observation.performance_state =
                Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(performance_state));
        }

        nvmlEnableState_t ecc_current = NVML_FEATURE_DISABLED;
        nvmlEnableState_t ecc_pending = NVML_FEATURE_DISABLED;
        if (nvmlDeviceGetEccMode(device, &ecc_current, &ecc_pending) == NVML_SUCCESS) {
            observation.ecc_enabled =
                Observed<bool>::reported(ecc_current == NVML_FEATURE_ENABLED);
        }

        unsigned long long ecc = 0;
        if (nvmlDeviceGetTotalEccErrors(device, NVML_MEMORY_ERROR_TYPE_CORRECTED,
                                        NVML_VOLATILE_ECC, &ecc) == NVML_SUCCESS) {
            observation.ecc_corrected_errors = Observed<std::uint64_t>::reported(ecc);
        }
        ecc = 0;
        if (nvmlDeviceGetTotalEccErrors(device, NVML_MEMORY_ERROR_TYPE_UNCORRECTED,
                                        NVML_VOLATILE_ECC, &ecc) == NVML_SUCCESS) {
            observation.ecc_uncorrected_errors = Observed<std::uint64_t>::reported(ecc);
        }

        std::vector<NvLinkLinkObservation> links;
        links.reserve(NVML_NVLINK_MAX_LINKS);
        for (unsigned int link = 0; link < NVML_NVLINK_MAX_LINKS; ++link) {
            nvmlEnableState_t state = NVML_FEATURE_DISABLED;
            const nvmlReturn_t result = nvmlDeviceGetNvLinkState(device, link, &state);
            if (result == NVML_ERROR_INVALID_ARGUMENT) {
                break;
            }
            if (result == NVML_SUCCESS) {
                links.push_back({link, Observed<bool>::reported(state == NVML_FEATURE_ENABLED)});
            }
        }
        if (!links.empty()) {
            observation.nvlink_links =
                Observed<std::vector<NvLinkLinkObservation>>::reported(std::move(links));
        }

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
