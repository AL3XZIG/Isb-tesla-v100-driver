#include "isb/providers/vulkan/provider.hpp"

#ifdef ISB_HAS_VULKAN
#include <vulkan/vulkan.h>

#include <cstdint>
#include <limits>
#include <string>
#include <vector>

namespace isb::providers::vulkan {
namespace {

Observed<std::uint32_t> query_instance_api_version() {
    std::uint32_t version = VK_API_VERSION_1_0;
    const auto enumerate_version = vkEnumerateInstanceVersion;
    if (enumerate_version != nullptr && enumerate_version(&version) == VK_SUCCESS) {
        return Observed<std::uint32_t>::reported(version);
    }
    return Observed<std::uint32_t>::reported(VK_API_VERSION_1_0);
}

class VulkanProvider final : public Provider {
public:
    common::Result<RuntimeObservation> observe() override {
        RuntimeObservation observation;
        observation.instance_api_version = query_instance_api_version();

        VkApplicationInfo application_info{};
        application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        application_info.pApplicationName = "ISB";
        application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.pEngineName = "ISB";
        application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        application_info.apiVersion = observation.instance_api_version.value.value_or(VK_API_VERSION_1_0);

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &application_info;

        VkInstance instance = VK_NULL_HANDLE;
        if (vkCreateInstance(&create_info, nullptr, &instance) != VK_SUCCESS) {
            return common::Status::failure(common::ErrorCode::QueryFailed,
                                           "vkCreateInstance failed");
        }

        std::uint32_t count = 0;
        VkResult result = vkEnumeratePhysicalDevices(instance, &count, nullptr);
        if (result != VK_SUCCESS) {
            vkDestroyInstance(instance, nullptr);
            return common::Status::failure(common::ErrorCode::QueryFailed,
                                           "vkEnumeratePhysicalDevices count failed");
        }

        std::vector<VkPhysicalDevice> physical_devices(count);
        if (count != 0) {
            result = vkEnumeratePhysicalDevices(instance, &count, physical_devices.data());
            if (result != VK_SUCCESS) {
                vkDestroyInstance(instance, nullptr);
                return common::Status::failure(common::ErrorCode::QueryFailed,
                                               "vkEnumeratePhysicalDevices failed");
            }
        }

        std::vector<PhysicalDeviceObservation> devices;
        devices.reserve(count);
        for (VkPhysicalDevice device : physical_devices) {
            VkPhysicalDeviceProperties properties{};
            vkGetPhysicalDeviceProperties(device, &properties);
            devices.push_back({
                Observed<std::string>::reported(properties.deviceName),
                Observed<std::uint32_t>::reported(properties.vendorID),
                Observed<std::uint32_t>::reported(properties.deviceID),
                Observed<std::uint32_t>::reported(properties.apiVersion),
                Observed<std::uint32_t>::reported(properties.driverVersion),
                Observed<std::uint32_t>::reported(static_cast<std::uint32_t>(properties.deviceType)),
            });
        }

        vkDestroyInstance(instance, nullptr);
        observation.devices = Observed<std::vector<PhysicalDeviceObservation>>::reported(std::move(devices));
        return observation;
    }
};

} // namespace

std::unique_ptr<Provider> make_vulkan_provider() {
    return std::make_unique<VulkanProvider>();
}

} // namespace isb::providers::vulkan
#else
namespace isb::providers::vulkan {

std::unique_ptr<Provider> make_vulkan_provider() {
    return nullptr;
}

} // namespace isb::providers::vulkan
#endif
