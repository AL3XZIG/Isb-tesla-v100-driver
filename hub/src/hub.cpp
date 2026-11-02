#include "isb/hub/hub.hpp"

#include <cctype>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <stdexcept>

namespace isb::hub {
namespace {

std::string json_quote(const std::string& value) {
    std::ostringstream out;
    out << '"';
    for (const unsigned char ch : value) {
        switch (ch) {
            case '"': out << "\\\""; break;
            case '\\': out << "\\\\"; break;
            case '\b': out << "\\b"; break;
            case '\f': out << "\\f"; break;
            case '\n': out << "\\n"; break;
            case '\r': out << "\\r"; break;
            case '\t': out << "\\t"; break;
            default:
                if (ch < 0x20) {
                    out << "\\u00" << std::hex << std::setw(2) << std::setfill('0')
                        << static_cast<unsigned int>(ch) << std::dec;
                } else {
                    out << static_cast<char>(ch);
                }
        }
    }
    out << '"';
    return out.str();
}

std::string environment_hash(const Environment& env) {
    if (env.provenance.synthetic) return "deterministic-mock-v1";

    // Stable FNV-1a, rather than implementation-defined std::hash, so report
    // metadata can be compared across processes and standard libraries.
    const std::string evidence = env.provenance.provider + "|" + env.provenance.detail +
        "|" + env.driver_version + "|" + env.os + "|" + to_string(env.mode);
    std::uint64_t hash = 14695981039346656037ULL;
    for (const unsigned char ch : evidence) {
        hash ^= ch;
        hash *= 1099511628211ULL;
    }
    std::ostringstream out;
    out << "fnv1a64-" << std::hex << std::setw(16) << std::setfill('0') << hash;
    return out.str();
}

void write_report_file(const std::filesystem::path& path, const std::string& contents) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out) throw std::runtime_error("cannot open report file: " + path.string());
    out << contents;
    out.close();
    if (!out) throw std::runtime_error("cannot write report file: " + path.string());
}

} // namespace

const char* to_string(ProviderMode mode) {
    switch (mode) {
        case ProviderMode::Real: return "real";
        case ProviderMode::Mock: return "mock";
        case ProviderMode::Unavailable: return "unavailable";
    }
    return "unknown";
}

const char* to_string(OperationState state) {
    switch (state) {
        case OperationState::Planned: return "planned";
        case OperationState::Applied: return "applied";
        case OperationState::Verified: return "verified";
        case OperationState::Failed: return "failed";
        case OperationState::RolledBack: return "rolled_back";
    }
    return "unknown";
}

const char* to_string(ControlState state) {
    switch (state) {
        case ControlState::Available: return "available";
        case ControlState::Unsupported: return "unsupported";
        case ControlState::PermissionDenied: return "permission_denied";
        case ControlState::Error: return "error";
    }
    return "unknown";
}

Environment MockProvider::environment() const {
    return {"Linux x86-64 (mock)", "550.54.14-mock", ProviderMode::Mock,
            {"mock", "deterministic V100 SXM2 fixture; no hardware access", true}};
}

cal::GpuCapabilities MockProvider::capabilities() const {
    cal::GpuCapabilities c;
    c.identity.vendor = "NVIDIA";
    c.identity.model_name = "Tesla V100-SXM2-16GB (mock)";
    c.identity.architecture = "Volta";
    c.identity.variant = cal::GpuVariant::V100_SXM2;
    c.identity.exact_hardware_variant = "Tesla V100-SXM2-16GB";
    c.identity.compute_capability = cal::ComputeCapability{7, 0};
    c.compute.cuda_state = cal::CapabilityState::Available;
    c.compute.cuda_version = cal::CudaVersion{12, 4};
    c.compute.compute_capability = cal::ComputeCapability{7, 0};
    c.compute.tensor_cores = {cal::CapabilityState::Available, 1};
    c.graphics.vulkan_state = cal::CapabilityState::Available;
    c.graphics.vulkan_api_version = cal::VulkanVersion{1, 3};
    c.graphics.opengl_state = cal::CapabilityState::Unavailable;
    c.graphics.directx12_state = cal::CapabilityState::Unavailable;
    c.graphics.nvenc_state = cal::CapabilityState::Unavailable;
    c.graphics.nvdec_state = cal::CapabilityState::Available;
    c.hardware.tensor_cores.state = cal::CapabilityState::Available;
    c.hardware.rt_cores.state = cal::CapabilityState::Unavailable;
    c.hardware.hbm2.state = cal::CapabilityState::Available;
    c.hardware.ecc.state = cal::CapabilityState::Available;
    c.hardware.nvlink.state = cal::CapabilityState::Available;
    c.hardware.mig.state = cal::CapabilityState::Unavailable;
    c.hardware.display_outputs.state = cal::CapabilityState::Unavailable;
    c.hardware.optical_flow_accelerator.state = cal::CapabilityState::Unavailable;
    c.compatibility.native_v100 = cal::CapabilityState::Available;
    c.compatibility.dlss = cal::CapabilityState::Unavailable;
    c.compatibility.dlss_frame_generation = cal::CapabilityState::Unavailable;
    c.compatibility.native_frame_generation = cal::CapabilityState::Unavailable;
    c.compatibility.fsr = cal::CapabilityState::Unknown;
    c.compatibility.xess = cal::CapabilityState::Unknown;
    c.compatibility.optiscaler = cal::CapabilityState::Unknown;
    c.compatibility.cuda_legacy_apps = cal::CapabilityState::Unknown;
    return c;
}

TelemetrySnapshot MockProvider::telemetry() const {
    TelemetrySnapshot t;
    t.timestamp = "1970-01-01T00:00:00Z";
    t.synthetic = true;
    t.provenance = {"mock", "fixed telemetry fixture", true};
    t.temperature_c = 42;
    t.gpu_utilization_percent = 17;
    t.memory_utilization_percent = 11;
    t.power_w = 61;
    t.power_limit_w = 300;
    t.gpu_clock_mhz = 1350;
    t.memory_clock_mhz = 877;
    t.vram_used_mib = 512;
    t.vram_total_mib = 16384;
    t.performance_state = "P0";
    t.ecc = "enabled";
    t.pcie = "Gen3 x16";
    t.nvlink = "6 links (mock)";
    t.process_count = 1;
    t.throttling_hint = "unknown";
    t.driver = "550.54.14-mock";
    return t;
}

std::vector<Control> MockProvider::controls() const {
    return {
        {"persistence_mode", ControlState::Unsupported, "mock does not mutate hardware"},
        {"power_limit", ControlState::Unsupported, "requires real backend and explicit approval"},
        {"application_clocks", ControlState::Unsupported, "requires real backend and explicit approval"},
        {"compute_mode", ControlState::Unsupported, "requires real backend and explicit approval"},
    };
}

Environment UnavailableProvider::environment() const {
    return {"unknown", "unknown", ProviderMode::Unavailable,
            {"none", "no usable provider", false}};
}

cal::GpuCapabilities UnavailableProvider::capabilities() const { return {}; }
TelemetrySnapshot UnavailableProvider::telemetry() const {
    TelemetrySnapshot t;
    t.provenance = {"none", "no provider", false};
    t.driver = "unknown";
    return t;
}
std::vector<Control> UnavailableProvider::controls() const { return {}; }

Environment Hub::environment() const { return provider_.environment(); }
cal::CapabilitySnapshot Hub::capabilities() const {
    const auto env = provider_.environment();
    return {provider_.capabilities(), env.provenance};
}
TelemetrySnapshot Hub::telemetry() const { return provider_.telemetry(); }
std::vector<Control> Hub::controls() const { return provider_.controls(); }

std::string Hub::status_json() const {
    const auto env = environment();
    std::ostringstream out;
    out << "{\"schema_version\":\"1\",\"mode\":" << json_quote(to_string(env.mode))
        << ",\"driver\":" << json_quote(env.driver_version)
        << ",\"capabilities\":" << cal::to_json(capabilities())
        << ",\"telemetry\":" << json(telemetry()) << "}";
    return out.str();
}

ProfilePlan Hub::profile_plan(const std::string& name) const {
    ProfilePlan plan;
    plan.name = name;
    plan.operations = {
        {"set_persistence_mode", "1", "Optional stability baseline"},
        {"set_power_limit", "300", "V100 SXM2 nominal envelope; validate SKU first"},
        {"set_application_clocks", "877,1350", "Conservative memory/SM clock request"},
    };
    plan.unknown = {"provider-specific acceptance", "current power/clock state", "permission level"};
    plan.unsupported = {"write/mutation backend not connected in this phase"};
    plan.risks = {
        "Clocks and power limits can affect stability and thermals.",
        "A plan is not proof that a change was applied or verified.",
        "No native DLSS, RT-core, MIG, or display-output capability is implied.",
    };
    if (telemetry().synthetic) {
        plan.risks.push_back("Current telemetry is synthetic; no real-hardware claim is made.");
    }
    if (telemetry().throttling_hint == "throttling") {
        plan.risks.push_back("Throttling is observed; inspect thermal, power and ECC evidence before tuning.");
    }
    return plan;
}

OptimizationPlan Hub::optimize_plan() const {
    OptimizationPlan plan;
    plan.target = "V100 workload-oriented control plane";
    plan.operations = {
        {"inventory", "observe", "Collect capabilities and telemetry without mutation"},
        {"legacy_compute", "validate", "Compile/check a Volta-compatible workload separately"},
        {"graphics_compatibility", "unknown", "Do not infer native DLSS/FG on V100"},
    };
    plan.unknown = {"actual application list", "CUDA source compatibility", "frame-generation backend"};
    plan.unsupported = {"DLSS native", "DLSS Frame Generation native", "automatic mutation"};
    plan.risks = {"Benchmark fixtures are not workload performance measurements."};
    if (telemetry().synthetic) {
        plan.risks.push_back("Plan is based on synthetic telemetry; hardware qualification is still required.");
    }
    if (telemetry().throttling_hint == "throttling") {
        plan.risks.push_back("Throttling is observed; do not raise limits until the cause is understood.");
    }
    return plan;
}

OperationResult Hub::apply(const ProfilePlan& plan, bool approved) const {
    OperationResult result;
    result.state = OperationState::Failed;
    result.mutated = false;
    result.provenance = environment().provenance;
    if (!approved) {
        result.verification = {false, "ISB_APPROVAL_REQUIRED", "Explicit approval required", result.provenance};
        return result;
    }
    if (plan.operations.empty()) {
        result.verification = {false, "ISB_NO_OPERATIONS", "Nothing to apply", result.provenance};
        return result;
    }
    result.verification = {false, "ISB_MUTATION_BACKEND_UNAVAILABLE",
                           "Read-only phase: no hardware mutation was performed", result.provenance};
    return result;
}

VerificationResult Hub::verify() const {
    const auto env = environment();
    if (env.mode == ProviderMode::Unavailable) {
        return {false, "ISB_PROVIDER_UNAVAILABLE", "No provider to verify", env.provenance};
    }
    if (env.provenance.synthetic) {
        return {false, "ISB_SYNTHETIC_ONLY", "Mock state cannot verify real hardware", env.provenance};
    }
    return {false, "ISB_READBACK_NOT_IMPLEMENTED", "No mutation read-back was requested", env.provenance};
}

std::vector<Diagnostic> Hub::diagnose() const {
    std::vector<Diagnostic> out;
    const auto env = environment();
    if (env.mode == ProviderMode::Unavailable) {
        out.push_back({"ISB_PROVIDER_UNAVAILABLE", "No usable real or mock provider"});
    } else if (env.provenance.synthetic) {
        out.push_back({"ISB_SYNTHETIC_EVIDENCE", "Mock values must not be treated as hardware qualification"});
    }
    return out;
}

BenchmarkResult Hub::benchmark() const {
    const auto env = environment();
    return {"control-plane-fixture", "1", "fixture-ok", true, true, env.provenance};
}

ReportBundle Hub::report(const std::string& directory) const {
    ReportBundle bundle;
    bundle.directory = directory;
    const std::filesystem::path root(directory);
    std::error_code ec;
    std::filesystem::create_directories(root, ec);
    if (ec || !std::filesystem::is_directory(root)) {
        throw std::runtime_error("cannot create report directory: " + directory +
                                 (ec ? ": " + ec.message() : ""));
    }

    const auto env = environment();
    const auto caps = capabilities();
    const auto telem = telemetry();
    write_report_file(root / "capabilities.json", cal::to_json(caps));
    write_report_file(root / "telemetry.json", json(telem));
    std::ostringstream manifest;
    manifest << "{\"schema_version\":\"1\",\"environment_hash\":"
             << json_quote(environment_hash(env))
             << ",\"mode\":" << json_quote(to_string(env.mode))
             << ",\"synthetic\":" << (env.provenance.synthetic ? "true" : "false")
             << ",\"provider\":" << json_quote(env.provenance.provider)
             << ",\"files\":[\"capabilities.json\",\"telemetry.json\"]}";
    bundle.manifest = manifest.str();
    write_report_file(root / "manifest.json", bundle.manifest);
    return bundle;
}

std::string json(const TelemetrySnapshot& t) {
    auto opt_int = [](const auto& v) -> std::string {
        return v ? std::to_string(*v) : "null";
    };
    std::ostringstream out;
    out << "{\"timestamp\":" << json_quote(t.timestamp)
        << ",\"synthetic\":" << (t.synthetic ? "true" : "false")
        << ",\"temperature_c\":" << opt_int(t.temperature_c)
        << ",\"gpu_utilization_percent\":" << opt_int(t.gpu_utilization_percent)
        << ",\"memory_utilization_percent\":" << opt_int(t.memory_utilization_percent)
        << ",\"power_w\":" << opt_int(t.power_w)
        << ",\"power_limit_w\":" << opt_int(t.power_limit_w)
        << ",\"gpu_clock_mhz\":" << opt_int(t.gpu_clock_mhz)
        << ",\"memory_clock_mhz\":" << opt_int(t.memory_clock_mhz)
        << ",\"vram_used_mib\":" << opt_int(t.vram_used_mib)
        << ",\"vram_total_mib\":" << opt_int(t.vram_total_mib)
        << ",\"performance_state\":" << json_quote(t.performance_state)
        << ",\"ecc\":" << json_quote(t.ecc)
        << ",\"pcie\":" << json_quote(t.pcie)
        << ",\"nvlink\":" << json_quote(t.nvlink)
        << ",\"process_count\":" << opt_int(t.process_count)
        << ",\"throttling\":" << json_quote(t.throttling_hint)
        << ",\"driver\":" << json_quote(t.driver) << "}";
    return out.str();
}

std::string json(const ProfilePlan& p) {
    std::ostringstream out;
    out << "{\"profile\":" << json_quote(p.name) << ",\"dry_run\":" << (p.dry_run ? "true" : "false")
        << ",\"operations\":" << p.operations.size()
        << ",\"unknown\":" << p.unknown.size()
        << ",\"unsupported\":" << p.unsupported.size() << "}";
    return out.str();
}

std::string json(const OptimizationPlan& p) {
    std::ostringstream out;
    out << "{\"target\":" << json_quote(p.target) << ",\"dry_run\":" << (p.dry_run ? "true" : "false")
        << ",\"operations\":" << p.operations.size() << ",\"unknown\":" << p.unknown.size()
        << ",\"unsupported\":" << p.unsupported.size() << "}";
    return out.str();
}

std::string json(const VerificationResult& v) {
    return "{\"verified\":" + std::string(v.verified ? "true" : "false") +
           ",\"code\":" + json_quote(v.code) + ",\"message\":" + json_quote(v.message) + "}";
}

std::string json(const BenchmarkResult& b) {
    return "{\"name\":" + json_quote(b.name) + ",\"version\":" + json_quote(b.version) +
           ",\"synthetic\":" + (b.synthetic ? "true" : "false") +
           ",\"correctness_verified\":" + (b.correctness_verified ? "true" : "false") +
           ",\"result\":" + json_quote(b.result) + "}";
}

} // namespace isb::hub
