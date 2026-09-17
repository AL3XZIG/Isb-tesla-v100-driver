#include "isb/cal/capabilities.hpp"

#include <iomanip>
#include <sstream>
#include <string_view>

namespace isb::cal {
namespace {

std::string escape_json(std::string_view value) {
    std::string result;
    result.reserve(value.size() + 2);
    for (const unsigned char c : value) {
        switch (c) {
        case '"': result += "\\\""; break;
        case '\\': result += "\\\\"; break;
        case '\b': result += "\\b"; break;
        case '\f': result += "\\f"; break;
        case '\n': result += "\\n"; break;
        case '\r': result += "\\r"; break;
        case '\t': result += "\\t"; break;
        default:
            if (c < 0x20U) {
                std::ostringstream hex;
                hex << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                    << static_cast<unsigned int>(c);
                result += hex.str();
            } else {
                result += static_cast<char>(c);
            }
        }
    }
    return result;
}

void write_string(std::ostringstream& out, std::string_view value) {
    out << '"' << escape_json(value) << '"';
}

void write_version(std::ostringstream& out, const std::optional<Version>& version) {
    if (!version) {
        out << "null";
        return;
    }
    out << '{'
        << "\"major\":" << version->major << ','
        << "\"minor\":" << version->minor << ','
        << "\"patch\":" << version->patch
        << '}';
}

void write_compute_capability(std::ostringstream& out,
                              const std::optional<ComputeCapability>& capability) {
    if (!capability) {
        out << "null";
        return;
    }
    out << '{'
        << "\"major\":" << capability->major << ','
        << "\"minor\":" << capability->minor
        << '}';
}

void write_state(std::ostringstream& out, CapabilityState state) {
    out << '"' << common::to_string(state) << '"';
}

void write_capability(std::ostringstream& out, const Capability& capability) {
    out << "{\"state\":";
    write_state(out, capability.state);
    out << '}';
}

void write_tensor_cores(std::ostringstream& out, const TensorCores& cores) {
    out << "{\"state\":";
    write_state(out, cores.state);
    out << ",\"generation\":";
    if (cores.generation) {
        out << *cores.generation;
    } else {
        out << "null";
    }
    out << '}';
}

void write_tensor_precisions(std::ostringstream& out, const TensorPrecisionSet& set) {
    out << "{\"state\":";
    write_state(out, set.state);
    out << ",\"values\":[";
    for (std::size_t i = 0; i < set.values.size(); ++i) {
        if (i != 0) out << ',';
        write_string(out, to_string(set.values[i]));
    }
    out << "]}";
}

} // namespace

const char* to_string(CapabilityState state) noexcept {
    return common::to_string(state);
}

const char* to_string(GpuVariant variant) noexcept {
    switch (variant) {
    case GpuVariant::Unknown: return "unknown";
    case GpuVariant::V100_SXM2: return "V100 SXM2";
    case GpuVariant::V100_PCIe: return "V100 PCIe";
    }
    return "unknown";
}

const char* to_string(TensorPrecision precision) noexcept {
    switch (precision) {
    case TensorPrecision::FP16: return "fp16";
    case TensorPrecision::BF16: return "bf16";
    case TensorPrecision::TF32: return "tf32";
    case TensorPrecision::INT8: return "int8";
    case TensorPrecision::INT4: return "int4";
    }
    return "unknown";
}

std::string to_json(const GpuCapabilities& capabilities) {
    std::ostringstream out;
    out << '{';

    out << "\"identity\":{";
    out << "\"vendor\":";
    write_string(out, capabilities.identity.vendor);
    out << ",\"architecture\":";
    write_string(out, capabilities.identity.architecture);
    out << ",\"compute_capability\":";
    write_compute_capability(out, capabilities.identity.compute_capability);
    out << ",\"variant\":";
    write_string(out, to_string(capabilities.identity.variant));
    out << ",\"exact_hardware_variant\":";
    write_string(out, capabilities.identity.exact_hardware_variant);
    out << ",\"model_name\":";
    write_string(out, capabilities.identity.model_name);
    out << "},";

    out << "\"compute\":{";
    out << "\"cuda_state\":";
    write_state(out, capabilities.compute.cuda_state);
    out << ",\"cuda_version\":";
    write_version(out, capabilities.compute.cuda_version);
    out << ",\"compute_capability\":";
    write_compute_capability(out, capabilities.compute.compute_capability);
    out << ",\"cuda_cores\":";
    if (capabilities.compute.cuda_cores) {
        out << *capabilities.compute.cuda_cores;
    } else {
        out << "null";
    }
    out << ",\"tensor_cores\":";
    write_tensor_cores(out, capabilities.compute.tensor_cores);
    out << ",\"tensor_precisions\":";
    write_tensor_precisions(out, capabilities.compute.tensor_precisions);
    out << "},";

    out << "\"graphics\":{";
    out << "\"vulkan_state\":";
    write_state(out, capabilities.graphics.vulkan_state);
    out << ",\"vulkan_api_version\":";
    write_version(out, capabilities.graphics.vulkan_api_version);
    out << ",\"opengl_state\":";
    write_state(out, capabilities.graphics.opengl_state);
    out << ",\"graphics_acceleration_state\":";
    write_state(out, capabilities.graphics.graphics_acceleration_state);
    out << "},";

    out << "\"hardware\":{";
    out << "\"rt_cores\":";
    write_capability(out, capabilities.hardware.rt_cores);
    out << ",\"optical_flow_accelerator\":";
    write_capability(out, capabilities.hardware.optical_flow_accelerator);
    out << ",\"tensor_cores\":";
    write_tensor_cores(out, capabilities.hardware.tensor_cores);
    out << ",\"hbm2\":";
    write_capability(out, capabilities.hardware.hbm2);
    out << ",\"ecc\":";
    write_capability(out, capabilities.hardware.ecc);
    out << ",\"nvlink\":";
    write_capability(out, capabilities.hardware.nvlink);
    out << ",\"mig\":";
    write_capability(out, capabilities.hardware.mig);
    out << ",\"display_outputs\":";
    write_capability(out, capabilities.hardware.display_outputs);
    out << "}";

    out << '}';
    return out.str();
}

} // namespace isb::cal
