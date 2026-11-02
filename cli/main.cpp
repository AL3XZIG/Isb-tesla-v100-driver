#include "isb/hub/hub.hpp"
#include "isb/cal/capabilities.hpp"
#include "isb/graphics/upscaler_types.hpp"
#include "isb/graphics/render_path_types.hpp"
#include "presentation/formatter.hpp"
#include "provider_selection.hpp"

#include <algorithm>
#include <atomic>
#include <cerrno>
#include <chrono>
#include <csignal>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>

using namespace isb::hub;
using namespace isb::cli::presentation;

namespace {

std::atomic<bool> g_monitoring_active{false};

void signal_handler(int) {
    g_monitoring_active.store(false);
}

void print_render_detection(const isb::graphics::RenderPathResult& result, const Formatter& fmt) {
    if (result.config) {
        std::cout << fmt.section("GPU Detection Result");
        std::cout << fmt.kv("Status", format_capability_state(to_string(result.status), fmt)) << "\n";
        std::cout << result.message << "\n\n";
        Table table({"#", "Name", "Role", "PCI Bus", "Display", "Render"});
        for (std::size_t i = 0; i < result.config->gpus.size(); ++i) {
            const auto& gpu = result.config->gpus[i];
            table.add_row({std::to_string(i), gpu.name, to_string(gpu.role),
                gpu.pci_bus.empty() ? "Unknown" : gpu.pci_bus,
                gpu.has_display_outputs ? "Yes" : "No", gpu.is_render_target ? "Yes" : "No"});
        }
        std::cout << table.render(fmt);
    } else {
        std::cout << fmt.badge(BadgeType::Error, "Detection Failed") << "\n";
        std::cout << result.message << "\n";
    }
}

void print_help(const Formatter& fmt) {
    std::cout << Banner::compact(fmt) << "\n" << fmt.section("Usage");
    std::cout << "  isb [options] command [args...]\n\n" << fmt.section("Options");
    std::cout << fmt.kv("--mock", "Use mock provider (no real hardware access)") << "\n";
    std::cout << fmt.kv("--json", "Output in JSON format") << "\n";
    std::cout << fmt.kv("--dry-run", "Show plan without applying changes") << "\n";
    std::cout << fmt.kv("--color", "Color policy: auto|always|never (default: auto)") << "\n";
    std::cout << fmt.kv("--help", "Show this help message") << "\n";
    std::cout << fmt.section("System Commands");
    std::cout << fmt.kv("status", "Show system status dashboard") << "\n";
    std::cout << fmt.kv("info", "Show detailed GPU and driver information") << "\n";
    std::cout << fmt.kv("capabilities", "Show GPU capabilities table") << "\n";
    std::cout << fmt.kv("telemetry", "Show current telemetry snapshot") << "\n";
    std::cout << fmt.kv("monitor", "Live monitoring mode (--interval N)") << "\n";
    std::cout << fmt.section("Profile Commands");
    std::cout << fmt.kv("profile list", "List available performance profiles") << "\n";
    std::cout << fmt.kv("profile plan <name>", "Show profile application plan") << "\n";
    std::cout << fmt.section("Optimization") << fmt.kv("optimize", "Show V100 optimization plan") << "\n";
    std::cout << fmt.section("Diagnostics");
    std::cout << fmt.kv("diagnose", "Run system diagnostics") << "\n";
    std::cout << fmt.kv("verify", "Verify system state") << "\n";
    std::cout << fmt.kv("doctor", "Comprehensive system health check") << "\n";
    std::cout << fmt.section("Graphics & Render Path");
    std::cout << fmt.kv("render detect", "Detect GPU roles and render path") << "\n";
    std::cout << fmt.kv("render plan", "Show render path plan") << "\n";
    std::cout << fmt.kv("render configure --apply", "Apply render path configuration") << "\n";
    std::cout << fmt.kv("render verify", "Verify render path configuration") << "\n";
    std::cout << fmt.kv("graphics status", "Show graphics upscaling status") << "\n";
    std::cout << fmt.kv("graphics list", "List available upscaler backends") << "\n";
    std::cout << fmt.section("Development");
    std::cout << fmt.kv("benchmark", "Run benchmark fixture") << "\n";
    std::cout << fmt.kv("report <dir>", "Generate report to directory") << "\n";
    std::cout << fmt.kv("about", "Show version and project info") << "\n";
    std::cout << "\n" << fmt.line("─") << "\n" << fmt.dim() << "Examples:" << fmt.reset() << "\n";
    std::cout << "  isb --mock status\n  isb --mock --json capabilities\n"
                 "  isb --mock profile plan Gaming\n  isb --mock monitor --interval 2\n"
                 "  isb --color never status | grep GPU\n\n";
}

void print_status(const Hub& hub, const Formatter& fmt) {
    const auto env = hub.environment();
    const auto caps = hub.capability_snapshot();
    const auto telem = hub.telemetry();
    std::cout << Banner::small(fmt) << "\n";
    std::cout << fmt.kv("Mode", format_provider_mode(to_string(env.mode), fmt)) << "\n";
    std::cout << fmt.kv("Provider", env.provenance.provider) << "\n";
    if (env.provenance.synthetic)
        std::cout << fmt.badge(BadgeType::Warning, "SYNTHETIC DATA - NOT REAL HARDWARE") << "\n";
    std::cout << fmt.section("GPU");
    const auto& identity = caps.capabilities.identity;
    std::cout << fmt.kv("Model", identity.model_name) << "\n";
    std::cout << fmt.kv("Architecture", identity.architecture) << "\n";
    if (identity.compute_capability) {
        std::cout << fmt.kv("Compute Cap", std::to_string(identity.compute_capability->major) + "." +
            std::to_string(identity.compute_capability->minor)) << "\n";
    }
    std::cout << fmt.kv("Variant", isb::cal::to_string(identity.variant)) << "\n";
    std::cout << fmt.section("Runtime");
    std::cout << fmt.kv("CUDA", format_capability_state(
        isb::cal::to_string(caps.capabilities.compute.cuda_state), fmt)) << "\n";
    std::cout << fmt.kv("Vulkan", format_capability_state(
        isb::cal::to_string(caps.capabilities.graphics.vulkan_state), fmt)) << "\n";
    std::cout << fmt.kv("NVML", env.mode == ProviderMode::Unavailable
        ? fmt.badge(BadgeType::Error, "Unavailable") : fmt.badge(BadgeType::Success, "Available")) << "\n";
    std::cout << fmt.section("Telemetry");
    std::cout << fmt.temperature(telem.temperature_c) << "\n";
    std::cout << fmt.kv("GPU Util", fmt.percent(telem.gpu_utilization_percent)) << "\n";
    std::cout << fmt.kv("Memory Util", fmt.percent(telem.memory_utilization_percent)) << "\n";
    if (telem.power_w.has_value()) {
        std::cout << fmt.kv("Power", fmt.power(telem.power_w));
        if (telem.power_limit_w.has_value()) std::cout << " / " << fmt.power(telem.power_limit_w) << " (limit)";
        std::cout << "\n";
    }
    std::cout << fmt.kv("GPU Clock", fmt.frequency(telem.gpu_clock_mhz)) << "\n";
    std::cout << fmt.kv("Memory Clock", fmt.frequency(telem.memory_clock_mhz)) << "\n";
    if (telem.vram_total_mib.has_value()) {
        std::cout << fmt.kv("VRAM", fmt.memory(telem.vram_used_mib.value_or(0)) + " / " +
            fmt.memory(telem.vram_total_mib.value())) << "\n";
    }
    std::cout << fmt.kv("Perf State", telem.performance_state.empty() ? "Unknown" : telem.performance_state) << "\n";
    std::cout << fmt.kv("ECC", telem.ecc.empty() ? "Unknown" : telem.ecc) << "\n";
    std::cout << fmt.kv("PCIe", telem.pcie.empty() ? "Unknown" : telem.pcie) << "\n";
    std::cout << fmt.kv("NVLink", telem.nvlink.empty() ? "Unknown" : telem.nvlink) << "\n";
    std::cout << fmt.section("Health");
    bool has_issues = false;
    if (!telem.temperature_c.has_value()) {
        std::cout << fmt.badge(BadgeType::Warning, "Temperature unavailable") << "\n";
        has_issues = true;
    }
    if (env.mode == ProviderMode::Unavailable) {
        std::cout << fmt.badge(BadgeType::Error, "Provider unavailable") << "\n";
        has_issues = true;
    }
    if (env.provenance.synthetic) {
        std::cout << fmt.badge(BadgeType::Info, "Mock mode - no real hardware data") << "\n";
        has_issues = true;
    }
    if (!has_issues) std::cout << fmt.badge(BadgeType::Success, "All systems nominal") << "\n";
    std::cout << "\n";
}

void print_info(const Hub& hub, const Formatter& fmt) {
    const auto env = hub.environment();
    const auto caps = hub.capability_snapshot();
    const auto& c = caps.capabilities;
    std::cout << Banner::compact(fmt) << fmt.section("Hardware");
    std::cout << fmt.kv("Vendor", c.identity.vendor) << "\n";
    std::cout << fmt.kv("Model", c.identity.model_name) << "\n";
    std::cout << fmt.kv("Architecture", c.identity.architecture) << "\n";
    std::cout << fmt.kv("Variant", isb::cal::to_string(c.identity.variant)) << "\n";
    std::cout << fmt.kv("Hardware Details", c.identity.exact_hardware_variant) << "\n";
    if (c.identity.compute_capability) {
        std::cout << fmt.kv("Compute Capability", std::to_string(c.identity.compute_capability->major) + "." +
            std::to_string(c.identity.compute_capability->minor)) << "\n";
    }
    std::cout << fmt.subsection("Features");
    auto feature = [&](const char* name, const char* state) {
        std::cout << fmt.kv(name, format_capability_state(state, fmt)) << "\n";
    };
    feature("Tensor Cores", isb::cal::to_string(c.hardware.tensor_cores.state));
    feature("HBM2", isb::cal::to_string(c.hardware.hbm2.state));
    feature("ECC", isb::cal::to_string(c.hardware.ecc.state));
    feature("RT Cores", isb::cal::to_string(c.hardware.rt_cores.state));
    feature("NVLink", isb::cal::to_string(c.hardware.nvlink.state));
    feature("MIG", isb::cal::to_string(c.hardware.mig.state));
    feature("Display Outputs", isb::cal::to_string(c.hardware.display_outputs.state));
    std::cout << fmt.section("Driver & Runtime");
    std::cout << fmt.kv("OS", env.os) << "\n" << fmt.kv("Driver Version", env.driver_version) << "\n";
    std::cout << fmt.kv("Provider Mode", format_provider_mode(to_string(env.mode), fmt)) << "\n";
    if (c.compute.cuda_version)
        std::cout << fmt.kv("CUDA Version", std::to_string(c.compute.cuda_version->major) + "." +
            std::to_string(c.compute.cuda_version->minor)) << "\n";
    if (c.graphics.vulkan_api_version)
        std::cout << fmt.kv("Vulkan API", std::to_string(c.graphics.vulkan_api_version->major) + "." +
            std::to_string(c.graphics.vulkan_api_version->minor)) << "\n";
    std::cout << fmt.section("ISB Hub") << fmt.kv("Provenance", caps.provenance.provider) << "\n";
    std::cout << fmt.kv("Synthetic", caps.provenance.synthetic ? "Yes (Mock)" : "No") << "\n";
    if (!caps.provenance.detail.empty()) std::cout << fmt.kv("Detail", caps.provenance.detail) << "\n";
    std::cout << "\n";
}

void print_capabilities(const Hub& hub, const Formatter& fmt) {
    const auto c = hub.capabilities();
    std::cout << Banner::compact(fmt) << fmt.section("GPU Capabilities");
    Table table({"Capability", "State", "Details"});
    table.add_row({"CUDA", isb::cal::to_string(c.compute.cuda_state), ""});
    if (c.compute.compute_capability) {
        table.add_row({"Compute Capability", "Available", std::to_string(c.compute.compute_capability->major) +
            "." + std::to_string(c.compute.compute_capability->minor)});
    }
    if (c.compute.tensor_cores.generation) {
        table.add_row({"Tensor Cores", isb::cal::to_string(c.compute.tensor_cores.state),
            "Gen " + std::to_string(*c.compute.tensor_cores.generation)});
    }
    table.add_row({"Vulkan", isb::cal::to_string(c.graphics.vulkan_state), ""});
    table.add_row({"OpenGL", isb::cal::to_string(c.graphics.opengl_state), ""});
    table.add_row({"HBM2", isb::cal::to_string(c.hardware.hbm2.state), ""});
    table.add_row({"ECC", isb::cal::to_string(c.hardware.ecc.state), ""});
    table.add_row({"NVLink", isb::cal::to_string(c.hardware.nvlink.state), ""});
    table.add_row({"RT Cores", isb::cal::to_string(c.hardware.rt_cores.state), "Not present on V100"});
    table.add_row({"Optical Flow", isb::cal::to_string(c.hardware.optical_flow_accelerator.state), "Not present on V100"});
    table.add_row({"MIG", isb::cal::to_string(c.hardware.mig.state), "Not supported on V100"});
    table.add_row({"Display Outputs", isb::cal::to_string(c.hardware.display_outputs.state), "V100 has no display outputs"});
    std::cout << table.render(fmt) << "\n";
}

void print_telemetry(const Hub& hub, const Formatter& fmt) {
    const auto t = hub.telemetry();
    std::cout << Banner::compact(fmt) << fmt.section("Telemetry Snapshot");
    std::cout << fmt.kv("Timestamp", format_timestamp(t.timestamp, fmt)) << "\n";
    if (t.synthetic) std::cout << fmt.badge(BadgeType::Warning, "SYNTHETIC DATA") << "\n";
    std::cout << "\n" << fmt.subsection("Thermal") << fmt.kv("Temperature", fmt.temperature(t.temperature_c)) << "\n";
    std::cout << "\n" << fmt.subsection("Utilization");
    std::cout << fmt.kv("GPU", fmt.percent(t.gpu_utilization_percent)) << "\n";
    std::cout << fmt.kv("Memory", fmt.percent(t.memory_utilization_percent)) << "\n";
    std::cout << "\n" << fmt.subsection("Power") << fmt.kv("Usage", fmt.power(t.power_w)) << "\n";
    std::cout << fmt.kv("Limit", fmt.power(t.power_limit_w)) << "\n";
    std::cout << "\n" << fmt.subsection("Clocks") << fmt.kv("GPU", fmt.frequency(t.gpu_clock_mhz)) << "\n";
    std::cout << fmt.kv("Memory", fmt.frequency(t.memory_clock_mhz)) << "\n";
    std::cout << "\n" << fmt.subsection("Memory");
    if (t.vram_total_mib && t.vram_used_mib) {
        std::cout << fmt.kv("Total", fmt.memory(*t.vram_total_mib)) << "\n";
        std::cout << fmt.kv("Used", fmt.memory(*t.vram_used_mib)) << "\n";
        std::cout << fmt.kv("Free", fmt.memory(*t.vram_total_mib - *t.vram_used_mib)) << "\n";
    }
    std::cout << "\n" << fmt.subsection("System");
    std::cout << fmt.kv("Perf State", t.performance_state.empty() ? "Unknown" : t.performance_state) << "\n";
    std::cout << fmt.kv("ECC", t.ecc.empty() ? "Unknown" : t.ecc) << "\n";
    std::cout << fmt.kv("PCIe", t.pcie.empty() ? "Unknown" : t.pcie) << "\n";
    std::cout << fmt.kv("NVLink", t.nvlink.empty() ? "Unknown" : t.nvlink) << "\n";
    std::cout << fmt.kv("Processes", t.process_count ? std::to_string(*t.process_count) : "Unknown") << "\n";
    std::cout << fmt.kv("Throttling", t.throttling_hint) << "\n\n";
}

void run_monitor(const Hub& hub, const Formatter& fmt, int interval_seconds) {
    g_monitoring_active.store(true);
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    while (g_monitoring_active.load()) {
        const auto t = hub.telemetry();
        std::cout << "[" << format_timestamp(t.timestamp, fmt) << "] "
                  << "Temp: " << fmt.temperature(t.temperature_c)
                  << " | GPU: " << fmt.percent(t.gpu_utilization_percent)
                  << " | Mem: " << fmt.percent(t.memory_utilization_percent)
                  << " | Power: " << fmt.power(t.power_w) << "\n";
        std::cout.flush();
        for (int i = 0; i < interval_seconds * 10 && g_monitoring_active.load(); ++i)
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << "\nMonitoring stopped.\n";
}

void print_profile_list(const Hub&, const Formatter& fmt) {
    std::cout << Banner::compact(fmt) << fmt.section("Performance Profiles");
    Table table({"Profile", "Description"});
    table.add_row({"Balanced", "Normal performance / power balance"});
    table.add_row({"Power Saving", "Reduced power target where supported"});
    table.add_row({"Gaming", "Graphics-oriented policy template"});
    table.add_row({"Maximum Performance", "Sustained performance policy template"});
    table.add_row({"Compute", "Compute workload policy template"});
    table.add_row({"AI / Tensor", "Tensor workload policy template"});
    table.add_row({"Custom", "User-defined policy template"});
    std::cout << table.render(fmt) << "\n";
}

void print_profile_plan(const Hub& hub, const std::string& name, const Formatter& fmt) {
    const auto plan = hub.profile_plan(name);
    std::cout << Banner::compact(fmt) << fmt.section("Profile Plan: " + name);
    std::cout << fmt.badge(BadgeType::Info, "DRY RUN") << "\n\n";
    for (const auto& op : plan.operations)
        std::cout << "  " << op.operation << " → " << op.value << "\n    " << op.rationale << "\n";
    if (!plan.unknown.empty()) {
        std::cout << fmt.subsection("Unknown");
        for (const auto& item : plan.unknown) std::cout << "  ? " << item << "\n";
    }
    if (!plan.unsupported.empty()) {
        std::cout << fmt.subsection("Unsupported");
        for (const auto& item : plan.unsupported) std::cout << "  - " << item << "\n";
    }
    std::cout << fmt.subsection("Risks");
    for (const auto& risk : plan.risks) std::cout << "  ! " << risk << "\n";
    std::cout << "\n" << fmt.line("─") << "\n" << fmt.dim()
              << "Read-only plan. No changes are applied without explicit approval and backend support.\n"
              << fmt.reset() << "\n";
}

void print_optimize_plan(const Hub& hub, const Formatter& fmt) {
    const auto plan = hub.optimize_plan();
    std::cout << Banner::compact(fmt) << fmt.section("V100 Optimization Plan");
    std::cout << fmt.kv("Target", plan.target) << "\n" << fmt.kv("Mode", "Dry run") << "\n\n";
    for (const auto& op : plan.operations)
        std::cout << "  " << op.operation << " → " << op.value << "\n    " << op.rationale << "\n";
    std::cout << fmt.subsection("Unknown / Unsupported");
    for (const auto& item : plan.unknown) std::cout << "  ? " << item << "\n";
    for (const auto& item : plan.unsupported) std::cout << "  - " << item << "\n";
    std::cout << fmt.subsection("Risks");
    for (const auto& risk : plan.risks) std::cout << "  ! " << risk << "\n";
    std::cout << "\n";
}

void print_diagnostics(const Hub& hub, const Formatter& fmt) {
    const auto errors = hub.diagnose();
    std::cout << Banner::compact(fmt) << fmt.section("System Diagnostics");
    if (errors.empty()) {
        std::cout << "\n" << fmt.badge(BadgeType::Success, "No issues detected") << "\n\n";
        return;
    }
    std::cout << "\n";
    for (const auto& error : errors) {
        const auto badge = error.code.find("SYNTHETIC") != std::string::npos ? BadgeType::Info
            : (error.code.find("UNAVAILABLE") != std::string::npos ? BadgeType::Error : BadgeType::Warning);
        std::cout << fmt.badge(badge, error.code) << "\n  " << error.message << "\n\n";
    }
}

void print_doctor(const Hub& hub, const Formatter& fmt) {
    const auto env = hub.environment();
    const auto caps = hub.capabilities();
    const auto telem = hub.telemetry();
    const auto errors = hub.diagnose();
    std::cout << Banner::small(fmt) << fmt.section("Driver Doctor - Comprehensive Health Check");
    int pass_count = 0, warn_count = 0, error_count = 0;
    std::cout << fmt.subsection("Hardware");
    if (caps.identity.variant != isb::cal::GpuVariant::Unknown) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " GPU detected: " << caps.identity.model_name << "\n";
        ++pass_count;
    } else {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " GPU variant unknown\n";
        ++warn_count;
    }
    std::cout << fmt.subsection("Driver Stack");
    if (env.mode != ProviderMode::Unavailable) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " Provider available (" << to_string(env.mode) << ")\n";
        ++pass_count;
    } else {
        std::cout << fmt.badge(BadgeType::Error, "FAIL") << " Provider unavailable\n";
        ++error_count;
    }
    auto runtime_check = [&](const char* name, isb::cal::CapabilityState state) {
        std::cout << fmt.subsection(name);
        if (state == isb::cal::CapabilityState::Available) {
            std::cout << fmt.badge(BadgeType::Success, "PASS") << " Available\n";
            ++pass_count;
        } else if (state == isb::cal::CapabilityState::Unknown) {
            std::cout << fmt.badge(BadgeType::Warning, "WARN") << " State unknown\n";
            ++warn_count;
        } else {
            std::cout << fmt.badge(BadgeType::Error, "FAIL") << " Unavailable\n";
            ++error_count;
        }
    };
    runtime_check("CUDA Runtime", caps.compute.cuda_state);
    runtime_check("Vulkan Runtime", caps.graphics.vulkan_state);
    std::cout << fmt.subsection("Telemetry");
    if (telem.temperature_c) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " Temperature monitoring active\n";
        ++pass_count;
    } else {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " Temperature unavailable\n";
        ++warn_count;
    }
    std::cout << fmt.subsection("Detected Issues");
    if (errors.empty()) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " No issues detected\n";
        ++pass_count;
    } else {
        for (const auto& error : errors)
            std::cout << fmt.badge(BadgeType::Warning, "ISSUE") << " " << error.message << "\n";
    }
    std::cout << fmt.section("Summary") << fmt.kv("Passed", std::to_string(pass_count)) << "\n";
    std::cout << fmt.kv("Warnings", std::to_string(warn_count)) << "\n";
    std::cout << fmt.kv("Errors", std::to_string(error_count)) << "\n\n";
}

void print_verify(const Hub& hub, const Formatter& fmt) {
    const auto result = hub.verify();
    std::cout << Banner::compact(fmt) << fmt.section("Verification Result");
    std::cout << fmt.badge(result.verified ? BadgeType::Success : BadgeType::Warning,
        result.verified ? "VERIFIED" : "NOT VERIFIED") << "\n";
    std::cout << fmt.kv("Code", result.code) << "\n" << fmt.kv("Message", result.message) << "\n";
    std::cout << fmt.kv("Provider", result.provenance.provider) << "\n\n";
}

void print_benchmark(const Hub& hub, const Formatter& fmt) {
    const auto result = hub.benchmark();
    std::cout << Banner::compact(fmt) << fmt.section("Benchmark Result");
    std::cout << fmt.kv("Name", result.name) << "\n" << fmt.kv("Version", result.version) << "\n";
    std::cout << fmt.kv("Synthetic", result.synthetic ? "Yes" : "No") << "\n";
    std::cout << fmt.kv("Correctness Verified", result.correctness_verified ? "Yes" : "No") << "\n";
    std::cout << fmt.kv("Result", result.result) << "\n" << fmt.kv("Provenance", result.provenance.provider) << "\n";
    if (result.synthetic)
        std::cout << "\n" << fmt.badge(BadgeType::Warning, "FIXTURE ONLY - NO PERFORMANCE CLAIM") << "\n";
    std::cout << "\n";
}

void print_report_result(const ReportBundle& report, const Formatter& fmt) {
    std::cout << Banner::compact(fmt) << fmt.section("Report Generated");
    std::cout << fmt.kv("Directory", report.directory) << "\n";
    std::cout << fmt.kv("Manifest", report.manifest.empty() ? "N/A" : "Generated") << "\n\n";
}

} // anonymous namespace

int main(int argc, char** argv) {
    bool mock = false;
    bool json_output = false;
    bool dry = false;
    (void)dry;
    ColorPolicy color_policy = ColorPolicy::Auto;
    int monitor_interval = 2;
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string x = argv[i];
        if (x == "--mock") mock = true;
        else if (x == "--json") json_output = true;
        else if (x == "--dry-run") dry = true;
        else if (x == "--help") {
            Formatter fmt(ColorPolicy::Auto);
            print_help(fmt);
            return 0;
        } else if (x == "--color") {
            if (i + 1 < argc) {
                std::string val = argv[++i];
                if (val == "auto") color_policy = ColorPolicy::Auto;
                else if (val == "always") color_policy = ColorPolicy::Always;
                else if (val == "never") color_policy = ColorPolicy::Never;
            }
        } else if (x == "--interval" && i + 1 < argc) {
            const char* raw = argv[++i];
            char* end = nullptr;
            errno = 0;
            const long parsed = std::strtol(raw, &end, 10);
            if (errno != 0 || end == raw || *end != '\0' || parsed < 1 || parsed > 3600) {
                std::cerr << "invalid --interval value: '" << raw << "' (expected an integer between 1 and 3600)\n";
                return 2;
            }
            monitor_interval = static_cast<int>(parsed);
        } else args.push_back(x);
    }

    Formatter fmt(color_policy);
    if (args.empty()) {
        std::cerr << "usage: isb [--mock] [--json] [--color POLICY] [--help] <command>\n"
                     "Run 'isb --help' for more information.\n";
        return 2;
    }

    if (args[0] == "render") {
        if (args.size() < 2) {
            std::cerr << "usage: isb render <detect|plan|configure|verify> [--apply]\n";
            return 2;
        }
        auto render_manager = isb::graphics::create_render_path_manager(mock);
        if (args[1] == "detect") {
            print_render_detection(render_manager->detect_gpus(), fmt);
            return 0;
        }
        if (args[1] == "plan") {
            const auto result = render_manager->plan_config();
            std::cout << result.message << "\n";
            return result.status == isb::common::CapabilityState::Error ? 1 : 0;
        }
        if (args[1] == "configure") {
            const bool apply = std::find(args.begin() + 2, args.end(), "--apply") != args.end();
            const auto result = render_manager->apply_config(apply);
            std::cout << result.message << "\n";
            return result.status == isb::common::CapabilityState::Error ? 1 : 0;
        }
        if (args[1] == "verify") {
            const auto result = render_manager->verify_config();
            std::cout << result.message << "\n";
            return result.status == isb::common::CapabilityState::Error ? 1 : 0;
        }
        std::cerr << "unknown render command: " << args[1] << "\n";
        return 2;
    }

    if (args[0] == "graphics" || args[0] == "upscaler") {
        if (args.size() < 2) {
            std::cerr << "usage: isb " << args[0] << " <status|list>\n";
            return 2;
        }
        if (args[1] == "status" || args[1] == "list") {
            auto manager = isb::graphics::create_upscaler_manager(mock);
            const auto backends = manager->list_backends();
            std::cout << Banner::compact(fmt) << fmt.section("Upscaler Backends");
            if (backends.empty()) std::cout << fmt.badge(BadgeType::Info, "No backends detected") << "\n";
            for (const auto& b : backends) {
                std::cout << "\n" << fmt.bold() << isb::graphics::to_string(b.type) << ": " << b.name << fmt.reset() << "\n";
                std::cout << fmt.kv("Version", b.version) << "\n";
                std::cout << fmt.badge(b.available ? BadgeType::Success : (b.supported ? BadgeType::Warning : BadgeType::Error),
                    b.available ? "Available" : (b.supported ? "Supported, not installed" : "Boundary/Unsupported")) << "\n";
                if (b.requires_external) std::cout << fmt.kv("Requires", b.external_component) << "\n";
                if (b.provenance.synthetic) std::cout << fmt.badge(BadgeType::Warning, "MOCK") << "\n";
            }
            std::cout << "\n";
            return 0;
        }
        std::cerr << "unknown graphics command: " << args[1] << "\n";
        return 2;
    }

    auto provider = isb::cli::make_provider(mock);
    Hub hub(*provider);

    if (args[0] == "status") {
        if (json_output) std::cout << hub.status_json() << "\n"; else print_status(hub, fmt);
    } else if (args[0] == "info") {
        if (json_output) std::cout << isb::cal::to_json(hub.capabilities()) << "\n"; else print_info(hub, fmt);
    } else if (args[0] == "inspect" || args[0] == "capabilities") {
        if (json_output) std::cout << isb::cal::to_json(hub.capabilities()) << "\n"; else print_capabilities(hub, fmt);
    } else if (args[0] == "telemetry") {
        if (json_output) std::cout << json(hub.telemetry()) << "\n"; else print_telemetry(hub, fmt);
    } else if (args[0] == "monitor") {
        run_monitor(hub, fmt, monitor_interval);
        return 0;
    } else if (args[0] == "profile" && args.size() > 1 && args[1] == "list") {
        print_profile_list(hub, fmt);
        return 0;
    } else if (args[0] == "profile" && args.size() > 2 && args[1] == "plan") {
        if (json_output) std::cout << json(hub.profile_plan(args[2])) << "\n"; else print_profile_plan(hub, args[2], fmt);
        return 0;
    } else if (args[0] == "optimize") {
        if (json_output) std::cout << json(hub.optimize_plan()) << "\n"; else print_optimize_plan(hub, fmt);
        return 0;
    } else if (args[0] == "diagnose") {
        print_diagnostics(hub, fmt);
        return 0;
    } else if (args[0] == "doctor") {
        print_doctor(hub, fmt);
        return 0;
    } else if (args[0] == "verify") {
        if (json_output) std::cout << json(hub.verify()) << "\n"; else print_verify(hub, fmt);
        return 0;
    } else if (args[0] == "benchmark") {
        if (json_output) std::cout << json(hub.benchmark()) << "\n"; else print_benchmark(hub, fmt);
        return 0;
    } else if (args[0] == "report") {
        try {
            print_report_result(hub.report(args.size() > 1 ? args[1] : "isb-report"), fmt);
            return 0;
        } catch (const std::exception& error) {
            std::cerr << "report generation failed: " << error.what() << "\n";
            return 1;
        }
    } else if (args[0] == "about") {
        std::cout << Banner::small(fmt) << fmt.section("About ISB V100 Driver");
        std::cout << "Intelligent Systems Bureau Tesla V100 Control Layer\n\n";
        std::cout << fmt.kv("Version", "0.1.0") << "\n" << fmt.kv("Target", "Tesla V100 SXM2 16GB") << "\n";
        std::cout << fmt.kv("Architecture", "Volta GV100") << "\n";
        std::cout << fmt.kv("Repository", "github.com/AL3XZIG/Isb-tesla-v100-driver") << "\n";
        std::cout << "\n" << fmt.dim() << "This is a user-space control plane, not a replacement NVIDIA driver.\n"
                  << fmt.reset() << "\n";
        return 0;
    } else {
        std::cerr << "unknown command: " << args[0] << "\nRun 'isb --help' for available commands.\n";
        return 2;
    }
    return 0;
}
