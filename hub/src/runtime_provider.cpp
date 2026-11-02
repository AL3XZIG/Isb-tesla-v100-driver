#include "isb/hub/runtime_provider.hpp"

#include "isb/providers/cuda/cuda_provider.hpp"
#include "isb/providers/nvml/hub_provider.hpp"
#include "isb/providers/vulkan/vulkan_provider.hpp"

#include <memory>
#include <utility>

namespace isb::hub {
namespace {

void merge_cuda(cal::GpuCapabilities& target, const providers::cuda::ObservedDevice& device) {
    if (target.identity.model_name.empty()) target.identity.model_name = device.name;
    if (target.identity.exact_hardware_variant.empty()) target.identity.exact_hardware_variant = device.name;
    target.compute.compute_capability = cal::ComputeCapability{device.major, device.minor};
    target.compute.cuda_state = cal::CapabilityState::Available;
    target.compute.cuda_version = cal::CudaVersion{device.major, device.minor};
    if (device.major == 7 && device.minor == 0) {
        target.identity.architecture = "Volta";
        target.compute.tensor_cores = {cal::CapabilityState::Available, 1};
        target.hardware.tensor_cores.state = cal::CapabilityState::Available;
    }
}

} // namespace

class RuntimeProvider final : public Provider {
public:
    RuntimeProvider()
        : nvml_(std::make_unique<providers::nvml::HubProvider>()),
          vulkan_(providers::vulkan::make_vulkan_provider()) {}

    Environment environment() const override {
        auto nvml_env = nvml_->environment();
        auto cuda = providers::cuda::CudaProvider{}.observe();
        const bool has_cuda_device = cuda.ok() && !cuda.value().devices.empty();
        auto vulkan = vulkan_ ? vulkan_->enumerate() : providers::vulkan::VulkanResult{};
        const bool has_vulkan_device = vulkan_ && vulkan.status.state == common::CapabilityState::Available
            && !vulkan.devices.empty();

        if (nvml_env.mode == ProviderMode::Real || has_cuda_device || has_vulkan_device) {
            Environment env = nvml_env;
            env.mode = ProviderMode::Real;
            env.provenance.provider = "runtime";
            env.provenance.detail = "NVML/CUDA/Vulkan runtime observation; first correlated device scope";
            env.provenance.synthetic = false;
            if (has_cuda_device) {
                env.provenance.detail += "; CUDA runtime " + std::to_string(cuda.value().runtime_version);
            }
            if (has_vulkan_device) {
                env.provenance.detail += "; Vulkan devices " + std::to_string(vulkan.devices.size());
            }
            return env;
        }

        Environment env;
        env.os = nvml_env.os;
        env.driver_version = nvml_env.driver_version;
        env.mode = ProviderMode::Unavailable;
        env.provenance = {"runtime", "no usable NVML, CUDA, or Vulkan device", false};
        if (!cuda.ok()) env.provenance.detail += "; CUDA: " + cuda.status.message;
        if (vulkan_ && vulkan.status.state != common::CapabilityState::Available) {
            env.provenance.detail += "; Vulkan: " + vulkan.status.message;
        }
        return env;
    }

    cal::GpuCapabilities capabilities() const override {
        auto caps = nvml_->capabilities();
        auto cuda = providers::cuda::CudaProvider{}.observe();
        const bool has_cuda_device = cuda.ok() && !cuda.value().devices.empty();

        // CUDA observations do not yet carry UUID/PCI BDF. Merge only when the
        // device scope is unambiguous; never mix arbitrary first devices.
        const auto nvml_devices = nvml_->observe_devices();
        const bool unambiguous_scope = has_cuda_device && cuda.value().devices.size() == 1
            && nvml_devices.size() <= 1;
        const bool cuda_only_scope = has_cuda_device && caps.identity.model_name.empty()
            && cuda.value().devices.size() == 1;
        if (unambiguous_scope || cuda_only_scope) {
            merge_cuda(caps, cuda.value().devices.front());
        }

        if (vulkan_) {
            auto result = vulkan_->enumerate();
            if (result.status.state == common::CapabilityState::Available && !result.devices.empty()) {
                caps.graphics.vulkan_state = cal::CapabilityState::Available;
                caps.graphics.vulkan_api_version = cal::VulkanVersion{1, 0};
            }
        }
        return caps;
    }

    TelemetrySnapshot telemetry() const override {
        auto t = nvml_->telemetry();
        t.provenance.provider = "runtime";
        t.provenance.detail += "; NVML-first runtime telemetry";
        t.synthetic = false;
        return t;
    }

    std::vector<Control> controls() const override {
        auto controls = nvml_->controls();
        controls.push_back({"runtime_observation", ControlState::Available,
                            "NVML/CUDA/Vulkan aggregation; identity-limited merge"});
        return controls;
    }

private:
    std::unique_ptr<providers::nvml::HubProvider> nvml_;
    std::unique_ptr<providers::vulkan::VulkanProvider> vulkan_;
};

std::unique_ptr<Provider> make_runtime_provider() {
    return std::make_unique<RuntimeProvider>();
}

} // namespace isb::hub
