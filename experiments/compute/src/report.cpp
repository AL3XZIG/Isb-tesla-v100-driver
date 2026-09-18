#include "isb/compute/report.hpp"

#include <sstream>
#include <string_view>

namespace isb::compute {
namespace {

const char* yes_no_unknown(cal::CapabilityState state) noexcept {
    switch (state) {
    case cal::CapabilityState::Available: return "YES";
    case cal::CapabilityState::Unavailable: return "NO";
    case cal::CapabilityState::Unknown:
    case cal::CapabilityState::PermissionDenied:
    case cal::CapabilityState::Error:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

const char* availability(cal::CapabilityState state) noexcept {
    switch (state) {
    case cal::CapabilityState::Available: return "AVAILABLE";
    case cal::CapabilityState::Unavailable: return "UNAVAILABLE";
    case cal::CapabilityState::Unknown:
    case cal::CapabilityState::PermissionDenied:
    case cal::CapabilityState::Error:
        return "UNKNOWN";
    }
    return "UNKNOWN";
}

void write_json_string(std::ostringstream& out, std::string_view value) {
    out << '"';
    for (const char c : value) {
        if (c == '"' || c == '\\') out << '\\';
        out << c;
    }
    out << '"';
}

} // namespace

std::string make_report(const ComputeDevice& device) {
    const KernelCapability kernels = kernel_capability(device);
    std::ostringstream out;
    out << (device.name().empty() ? "Unknown GPU" : device.name()) << '\n'
        << "Compute:\n" << availability(device.cuda_availability()) << '\n'
        << "CUDA:\n" << yes_no_unknown(device.cuda_availability()) << '\n'
        << "FP16:\n" << yes_no_unknown(kernels.fp16) << '\n'
        << "Tensor:\n" << yes_no_unknown(device.tensor_capability()) << '\n'
        << "RayTracing:\n" << yes_no_unknown(device.ray_tracing_capability()) << '\n';
    return out.str();
}

std::string to_json(const ComputeDevice& device) {
    const KernelCapability kernels = kernel_capability(device);
    std::ostringstream out;
    out << "{\"name\":";
    write_json_string(out, device.name());
    out << ",\"sm_version\":";
    if (device.sm_version()) {
        out << "{\"major\":" << device.sm_version()->major
            << ",\"minor\":" << device.sm_version()->minor << '}';
    } else {
        out << "null";
    }
    out << ",\"memory\":";
    if (device.memory()) {
        out << "{\"total\":" << device.memory()->total
            << ",\"free\":" << device.memory()->free
            << ",\"used\":" << device.memory()->used << '}';
    } else {
        out << "null";
    }
    out << ",\"tensor\":";
    write_json_string(out, cal::to_string(device.tensor_capability()));
    out << ",\"cuda\":";
    write_json_string(out, cal::to_string(device.cuda_availability()));
    out << ",\"fp32\":";
    write_json_string(out, cal::to_string(kernels.fp32));
    out << ",\"fp16\":";
    write_json_string(out, cal::to_string(kernels.fp16));
    out << ",\"int8\":";
    write_json_string(out, cal::to_string(kernels.int8));
    out << ",\"tensor_fp16\":";
    write_json_string(out, cal::to_string(kernels.tensor_fp16));
    out << ",\"ray_tracing\":";
    write_json_string(out, cal::to_string(device.ray_tracing_capability()));
    out << '}';
    return out.str();
}

} // namespace isb::compute
