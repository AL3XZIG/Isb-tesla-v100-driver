#include "isb/hub/hub.hpp"
#include "isb/graphics/upscaler_types.hpp"
#include "isb/graphics/render_path_types.hpp"
#include "presentation/formatter.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

using namespace isb::hub;
using namespace isb::cli::presentation;

namespace {

// Global flag for graceful shutdown during monitoring
std::atomic<bool> g_monitoring_active{false};

void signal_handler(int) {
    g_monitoring_active.store(false);
}

std::string json_escape(const std::string& value) {
    std::string out;
    for (char ch : value) {
        if (ch == '"' || ch == '\\') out += '\\';
        if (ch == '\n') out += "\\n";
        else if (ch == '\r') out += "\\r";
        else out += ch;
    }
    return out;
}

void print_render_detection(const isb::graphics::RenderPathResult& result, const Formatter& fmt) {
    if (result.config) {
        std::cout << fmt.section("GPU Detection Result");
        std::cout << fmt.kv("Status", format_capability_state(to_string(result.status), fmt)) << "\n";
        std::cout << result.message << "\n\n";
        
        Table table({"#", "Name", "Role", "PCI Bus", "Display", "Render"});
        for (std::size_t i = 0; i < result.config->gpus.size(); ++i) {
            const auto& gpu = result.config->gpus[i];
            table.add_row({
                std::to_string(i),
                gpu.name,
                to_string(gpu.role),
                gpu.pci_bus.empty() ? "Unknown" : gpu.pci_bus,
                gpu.has_display_outputs ? "Yes" : "No",
                gpu.is_render_target ? "Yes" : "No"
            });
        }
        std::cout << table.render(fmt);
    } else {
        std::cout << fmt.badge(BadgeType::Error, "Detection Failed") << "\n";
        std::cout << result.message << "\n";
    }
}

void print_help(const Formatter& fmt) {
    std::cout << Banner::compact(fmt);
    std::cout << "\n";
    std::cout << fmt.section("Usage");
    std::cout << "  isb [options] command [args...]\n\n";
    
    std::cout << fmt.section("Options");
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
    
    std::cout << fmt.section("Optimization");
    std::cout << fmt.kv("optimize", "Show V100 optimization plan") << "\n";
    
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
    
    std::cout << "\n" << fmt.line('─') << "\n";
    std::cout << fmt.dim << "Examples:" << fmt.reset() << "\n";
    std::cout << "  isb --mock status\n";
    std::cout << "  isb --mock --json capabilities\n";
    std::cout << "  isb --mock profile plan Gaming\n";
    std::cout << "  isb --mock monitor --interval 2\n";
    std::cout << "  isb --color never status | grep GPU\n\n";
}

void print_status(const Hub& hub, const Formatter& fmt) {
    const auto env = hub.environment();
    const auto caps = hub.capability_snapshot();
    const auto telem = hub.telemetry();
    
    std::cout << Banner::small(fmt);
    
    // Environment badge
    std::cout << "\n";
    std::cout << fmt.kv("Mode", format_provider_mode(to_string(env.mode), fmt)) << "\n";
    std::cout << fmt.kv("Provider", env.provenance.provider) << "\n";
    if (env.provenance.synthetic) {
        std::cout << fmt.badge(BadgeType::Warning, "SYNTHETIC DATA - NOT REAL HARDWARE") << "\n";
    }
    
    // GPU Identity
    std::cout << fmt.section("GPU");
    const auto& identity = caps.capabilities.identity;
    std::cout << fmt.kv("Model", identity.model_name) << "\n";
    std::cout << fmt.kv("Architecture", identity.architecture) << "\n";
    if (identity.compute_capability) {
        std::cout << fmt.kv("Compute Cap", 
            std::to_string(identity.compute_capability->major) + "." +
            std::to_string(identity.compute_capability->minor)) << "\n";
    }
    std::cout << fmt.kv("Variant", isb::cal::to_string(identity.variant)) << "\n";
    
    // Runtime Status
    std::cout << fmt.section("Runtime");
    auto cuda_state = format_capability_state(
        isb::cal::to_string(caps.capabilities.compute.cuda_state), fmt);
    std::cout << fmt.kv("CUDA", cuda_state) << "\n";
    
    auto vulkan_state = format_capability_state(
        isb::cal::to_string(caps.capabilities.graphics.vulkan_state), fmt);
    std::cout << fmt.kv("Vulkan", vulkan_state) << "\n";
    
    auto nvml_state = env.mode == ProviderMode::Unavailable ? 
        fmt.badge(BadgeType::Error, "Unavailable") :
        fmt.badge(BadgeType::Success, "Available");
    std::cout << fmt.kv("NVML", nvml_state) << "\n";
    
    // Telemetry
    std::cout << fmt.section("Telemetry");
    std::cout << fmt.temperature(telem.temperature_c) << "\n";
    std::cout << fmt.kv("GPU Util", fmt.percent(telem.gpu_utilization_percent)) << "\n";
    std::cout << fmt.kv("Memory Util", fmt.percent(telem.memory_utilization_percent)) << "\n";
    
    if (telem.power_w.has_value()) {
        std::cout << fmt.kv("Power", fmt.power(telem.power_w));
        if (telem.power_limit_w.has_value()) {
            std::cout << " / " << fmt.power(telem.power_limit_w) << " (limit)";
        }
        std::cout << "\n";
    }
    
    std::cout << fmt.kv("GPU Clock", fmt.frequency(telem.gpu_clock_mhz)) << "\n";
    std::cout << fmt.kv("Memory Clock", fmt.frequency(telem.memory_clock_mhz)) << "\n";
    
    if (telem.vram_total_mib.has_value()) {
        int total = telem.vram_total_mib.value();
        int used = telem.vram_used_mib.value_or(0);
        std::cout << fmt.kv("VRAM", fmt.memory(used) + " / " + fmt.memory(total)) << "\n";
    }
    
    std::cout << fmt.kv("Perf State", telem.performance_state.empty() ? "Unknown" : telem.performance_state) << "\n";
    std::cout << fmt.kv("ECC", telem.ecc.empty() ? "Unknown" : telem.ecc) << "\n";
    std::cout << fmt.kv("PCIe", telem.pcie.empty() ? "Unknown" : telem.pcie) << "\n";
    std::cout << fmt.kv("NVLink", telem.nvlink.empty() ? "Unknown" : telem.nvlink) << "\n";
    
    // Health summary
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
    if (!has_issues) {
        std::cout << fmt.badge(BadgeType::Success, "All systems nominal") << "\n";
    }
    
    std::cout << "\n";
}

void print_info(const Hub& hub, const Formatter& fmt) {
    const auto env = hub.environment();
    const auto caps = hub.capability_snapshot();
    
    std::cout << Banner::compact(fmt);
    
    // Hardware section
    std::cout << fmt.section("Hardware");
    const auto& identity = caps.capabilities.identity;
    std::cout << fmt.kv("Vendor", identity.vendor) << "\n";
    std::cout << fmt.kv("Model", identity.model_name) << "\n";
    std::cout << fmt.kv("Architecture", identity.architecture) << "\n";
    std::cout << fmt.kv("Variant", isb::cal::to_string(identity.variant)) << "\n";
    std::cout << fmt.kv("Hardware Details", identity.exact_hardware_variant) << "\n";
    
    if (identity.compute_capability) {
        std::cout << fmt.kv("Compute Capability", 
            std::to_string(identity.compute_capability->major) + "." +
            std::to_string(identity.compute_capability->minor)) << "\n";
    }
    
    // Hardware features
    std::cout << fmt.subsection("Features");
    std::cout << fmt.kv("Tensor Cores", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.tensor_cores.state), fmt)) << "\n";
    std::cout << fmt.kv("HBM2", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.hbm2.state), fmt)) << "\n";
    std::cout << fmt.kv("ECC", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.ecc.state), fmt)) << "\n";
    std::cout << fmt.kv("RT Cores", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.rt_cores.state), fmt)) << "\n";
    std::cout << fmt.kv("NVLink", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.nvlink.state), fmt)) << "\n";
    std::cout << fmt.kv("MIG", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.mig.state), fmt)) << "\n";
    std::cout << fmt.kv("Display Outputs", 
        format_capability_state(isb::cal::to_string(caps.capabilities.hardware.display_outputs.state), fmt)) << "\n";
    
    // Driver section
    std::cout << fmt.section("Driver & Runtime");
    std::cout << fmt.kv("OS", env.os) << "\n";
    std::cout << fmt.kv("Driver Version", env.driver_version) << "\n";
    std::cout << fmt.kv("Provider Mode", format_provider_mode(to_string(env.mode), fmt)) << "\n";
    
    if (caps.capabilities.compute.cuda_version) {
        auto v = caps.capabilities.compute.cuda_version.value();
        std::cout << fmt.kv("CUDA Version", 
            std::to_string(v.major) + "." + std::to_string(v.minor)) << "\n";
    }
    
    if (caps.capabilities.graphics.vulkan_api_version) {
        auto v = caps.capabilities.graphics.vulkan_api_version.value();
        std::cout << fmt.kv("Vulkan API", 
            std::to_string(v.major) + "." + std::to_string(v.minor)) << "\n";
    }
    
    // ISB section
    std::cout << fmt.section("ISB Hub");
    std::cout << fmt.kv("Provenance", caps.provenance.provider) << "\n";
    std::cout << fmt.kv("Synthetic", caps.provenance.synthetic ? "Yes (Mock)" : "No") << "\n";
    if (!caps.provenance.detail.empty()) {
        std::cout << fmt.kv("Detail", caps.provenance.detail) << "\n";
    }
    
    std::cout << "\n";
}

void print_capabilities(const Hub& hub, const Formatter& fmt) {
    const auto caps = hub.capability_snapshot();
    
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("GPU Capabilities");
    
    Table table({"Capability", "State", "Details"});
    
    // Compute capabilities
    table.add_row({"CUDA", 
        isb::cal::to_string(caps.capabilities.compute.cuda_state), ""});
    
    if (caps.capabilities.compute.compute_capability) {
        auto cc = caps.capabilities.compute.compute_capability.value();
        table.add_row({"Compute Capability", 
            "Available", 
            std::to_string(cc.major) + "." + std::to_string(cc.minor)});
    }
    
    if (caps.capabilities.compute.tensor_cores.generation) {
        table.add_row({"Tensor Cores", 
            isb::cal::to_string(caps.capabilities.compute.tensor_cores.state),
            "Gen " + std::to_string(caps.capabilities.compute.tensor_cores.generation.value())});
    }
    
    // Graphics capabilities
    table.add_row({"Vulkan", 
        isb::cal::to_string(caps.capabilities.graphics.vulkan_state), ""});
    table.add_row({"OpenGL", 
        isb::cal::to_string(caps.capabilities.graphics.opengl_state), ""});
    
    // Hardware features
    table.add_row({"HBM2", 
        isb::cal::to_string(caps.capabilities.hardware.hbm2.state), ""});
    table.add_row({"ECC", 
        isb::cal::to_string(caps.capabilities.hardware.ecc.state), ""});
    table.add_row({"NVLink", 
        isb::cal::to_string(caps.capabilities.hardware.nvlink.state), ""});
    table.add_row({"RT Cores", 
        isb::cal::to_string(caps.capabilities.hardware.rt_cores.state), 
        "Not present on V100"});
    table.add_row({"Optical Flow", 
        isb::cal::to_string(caps.capabilities.hardware.optical_flow_accelerator.state),
        "Not present on V100"});
    table.add_row({"MIG", 
        isb::cal::to_string(caps.capabilities.hardware.mig.state),
        "Not supported on V100"});
    table.add_row({"Display Outputs", 
        isb::cal::to_string(caps.capabilities.hardware.display_outputs.state),
        "V100 has no display outputs"});
    
    std::cout << table.render(fmt);
    std::cout << "\n";
}

void print_telemetry(const Hub& hub, const Formatter& fmt) {
    const auto telem = hub.telemetry();
    
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Telemetry Snapshot");
    std::cout << fmt.kv("Timestamp", format_timestamp(telem.timestamp, fmt)) << "\n";
    if (telem.synthetic) {
        std::cout << fmt.badge(BadgeType::Warning, "SYNTHETIC DATA") << "\n";
    }
    
    std::cout << "\n" << fmt.subsection("Thermal");
    std::cout << fmt.kv("Temperature", fmt.temperature(telem.temperature_c)) << "\n";
    
    std::cout << "\n" << fmt.subsection("Utilization");
    std::cout << fmt.kv("GPU", fmt.percent(telem.gpu_utilization_percent)) << "\n";
    std::cout << fmt.kv("Memory", fmt.percent(telem.memory_utilization_percent)) << "\n";
    
    std::cout << "\n" << fmt.subsection("Power");
    std::cout << fmt.kv("Usage", fmt.power(telem.power_w)) << "\n";
    std::cout << fmt.kv("Limit", fmt.power(telem.power_limit_w)) << "\n";
    
    std::cout << "\n" << fmt.subsection("Clocks");
    std::cout << fmt.kv("GPU", fmt.frequency(telem.gpu_clock_mhz)) << "\n";
    std::cout << fmt.kv("Memory", fmt.frequency(telem.memory_clock_mhz)) << "\n";
    
    std::cout << "\n" << fmt.subsection("Memory");
    if (telem.vram_total_mib.has_value() && telem.vram_used_mib.has_value()) {
        int total = telem.vram_total_mib.value();
        int used = telem.vram_used_mib.value();
        int free = total - used;
        std::cout << fmt.kv("Total", fmt.memory(total)) << "\n";
        std::cout << fmt.kv("Used", fmt.memory(used)) << "\n";
        std::cout << fmt.kv("Free", fmt.memory(free)) << "\n";
    }
    
    std::cout << "\n" << fmt.subsection("System");
    std::cout << fmt.kv("Perf State", telem.performance_state.empty() ? "Unknown" : telem.performance_state) << "\n";
    std::cout << fmt.kv("ECC", telem.ecc.empty() ? "Unknown" : telem.ecc) << "\n";
    std::cout << fmt.kv("PCIe", telem.pcie.empty() ? "Unknown" : telem.pcie) << "\n";
    std::cout << fmt.kv("NVLink", telem.nvlink.empty() ? "Unknown" : telem.nvlink) << "\n";
    std::cout << fmt.kv("Processes", telem.process_count.has_value() ? 
        std::to_string(telem.process_count.value()) : "Unknown") << "\n";
    
    std::cout << "\n";
}

void run_monitor(const Hub& hub, const Formatter& fmt, int interval_seconds) {
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Live Monitoring");
    std::cout << fmt.kv("Interval", std::to_string(interval_seconds) + "s") << "\n";
    std::cout << fmt.dim << "Press Ctrl+C to stop" << fmt.reset() << "\n\n";
    
    // Setup signal handler
    std::signal(SIGINT, signal_handler);
    
    g_monitoring_active.store(true);
    
    while (g_monitoring_active.load()) {
        const auto telem = hub.telemetry();
        
        // Clear line and move cursor up
        std::cout << "\033[2K\r"; // Clear current line
        
        // Compact one-line status
        std::cout << fmt.ansi_color(Color::Bold) << "[" 
                  << fmt.ansi_color(Color::Reset)
                  << fmt.ansi_color(Color::BrightCyan) << "TEMP:" << fmt.ansi_color(Color::Reset);
        
        if (telem.temperature_c.has_value()) {
            int t = telem.temperature_c.value();
            std::cout << t << "°C";
            if (t < 40) std::cout << fmt.ansi_color(Color::BrightGreen);
            else if (t < 70) std::cout << fmt.ansi_color(Color::BrightYellow);
            else std::cout << fmt.ansi_color(Color::BrightRed);
        } else {
            std::cout << fmt.ansi_color(Color::BrightBlack) << "Unknown";
        }
        
        std::cout << fmt.ansi_color(Color::Reset) << " | ";
        std::cout << fmt.ansi_color(Color::BrightCyan) << "GPU:" << fmt.ansi_color(Color::Reset);
        
        if (telem.gpu_utilization_percent.has_value()) {
            std::cout << telem.gpu_utilization_percent.value() << "%";
        } else {
            std::cout << fmt.ansi_color(Color::BrightBlack) << "Unknown";
        }
        
        std::cout << fmt.ansi_color(Color::Reset) << " | ";
        std::cout << fmt.ansi_color(Color::BrightCyan) << "PWR:" << fmt.ansi_color(Color::Reset);
        
        if (telem.power_w.has_value()) {
            std::cout << telem.power_w.value() << "W";
        } else {
            std::cout << fmt.ansi_color(Color::BrightBlack) << "Unknown";
        }
        
        std::cout << fmt.ansi_color(Color::Reset) << " | ";
        std::cout << fmt.ansi_color(Color::BrightCyan) << "CLK:" << fmt.ansi_color(Color::Reset);
        
        if (telem.gpu_clock_mhz.has_value()) {
            std::cout << telem.gpu_clock_mhz.value() << "MHz";
        } else {
            std::cout << fmt.ansi_color(Color::BrightBlack) << "Unknown";
        }
        
        std::cout << fmt.ansi_color(Color::Reset) << "]";
        std::cout.flush();
        
        // Sleep in small increments to allow signal handling
        for (int i = 0; i < interval_seconds * 10 && g_monitoring_active.load(); ++i) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    std::cout << "\n" << fmt.dim << "Monitoring stopped." << fmt.reset() << "\n";
}

void print_profile_list(const Hub& hub, const Formatter& fmt) {
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Available Profiles");
    
    auto profiles = hub.profiles();
    for (std::size_t i = 0; i < profiles.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << profiles[i] << "\n";
    }
    std::cout << "\n";
}

void print_profile_plan(const Hub& hub, const std::string& profile, const Formatter& fmt) {
    auto plan = hub.profile_plan(profile);
    
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Profile Plan: " + profile);
    std::cout << fmt.kv("ID", plan.id) << "\n";
    std::cout << fmt.kv("Dry Run", plan.dry_run ? "Yes" : "No") << "\n";
    
    if (!plan.operations.empty()) {
        std::cout << fmt.subsection("Operations");
        for (const auto& op : plan.operations) {
            std::cout << fmt.badge(BadgeType::Info, op.id) << " " << op.description << "\n";
        }
    }
    
    if (!plan.unsupported.empty()) {
        std::cout << fmt.subsection("Unsupported");
        for (const auto& item : plan.unsupported) {
            std::cout << fmt.badge(BadgeType::Error, "SKIP") << " " << item << "\n";
        }
    }
    
    if (!plan.unknown.empty()) {
        std::cout << fmt.subsection("Unknown State");
        for (const auto& item : plan.unknown) {
            std::cout << fmt.badge(BadgeType::Unknown, "?") << " " << item << "\n";
        }
    }
    
    if (!plan.risks.empty()) {
        std::cout << fmt.subsection("Risks");
        for (const auto& risk : plan.risks) {
            std::cout << fmt.badge(BadgeType::Warning, "!") << " " << risk << "\n";
        }
    }
    
    if (!plan.verification_steps.empty()) {
        std::cout << fmt.subsection("Verification Steps");
        for (const auto& step : plan.verification_steps) {
            std::cout << "  • " << step << "\n";
        }
    }
    
    std::cout << "\n";
}

void print_optimize_plan(const Hub& hub, const Formatter& fmt) {
    auto plan = hub.optimize_plan();
    
    std::cout << Banner::small(fmt);
    std::cout << fmt.section("Optimize V100 Plan");
    std::cout << fmt.kv("ID", plan.id) << "\n";
    
    Progress progress(plan.operations.size() + plan.verification_steps.size());
    progress.set_message("Planning...");
    std::cout << progress.render(fmt) << "\n\n";
    
    if (plan.operations.empty() && plan.unsupported.empty() && plan.unknown.empty()) {
        std::cout << fmt.badge(BadgeType::Info, "No operations available") << "\n";
        std::cout << "The optimization backend may not be fully implemented yet.\n";
    }
    
    if (!plan.operations.empty()) {
        std::cout << fmt.subsection("Proposed Changes");
        for (std::size_t i = 0; i < plan.operations.size(); ++i) {
            const auto& op = plan.operations[i];
            std::cout << fmt.kv("[" + std::to_string(i+1) + "]", op.description) << "\n";
        }
    }
    
    if (!plan.unsupported.empty()) {
        std::cout << fmt.subsection("Skipped (Unsupported)");
        for (const auto& item : plan.unsupported) {
            std::cout << fmt.badge(BadgeType::Error, "SKIP") << " " << item << "\n";
        }
    }
    
    if (!plan.unknown.empty()) {
        std::cout << fmt.subsection("Skipped (Unknown)");
        for (const auto& item : plan.unknown) {
            std::cout << fmt.badge(BadgeType::Unknown, "?") << " " << item << "\n";
        }
    }
    
    if (!plan.risks.empty()) {
        std::cout << fmt.subsection("Risks & Warnings");
        for (const auto& risk : plan.risks) {
            std::cout << fmt.badge(BadgeType::Warning, "!") << " " << risk << "\n";
        }
    }
    
    if (!plan.verification_steps.empty()) {
        std::cout << fmt.subsection("Verification");
        for (const auto& step : plan.verification_steps) {
            std::cout << "  ✓ " << step << "\n";
        }
    }
    
    std::cout << "\n" << fmt.line('─') << "\n";
    std::cout << fmt.dim << "Note: This is a read-only plan. No changes will be applied without explicit approval.\n";
    std::cout << "Use 'profile apply <name>' to apply changes (when backend supports mutation).\n" << fmt.reset();
    std::cout << "\n";
}

void print_diagnostics(const Hub& hub, const Formatter& fmt) {
    auto errors = hub.diagnose();
    
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("System Diagnostics");
    
    if (errors.empty()) {
        std::cout << "\n" << fmt.badge(BadgeType::Success, "No issues detected") << "\n\n";
    } else {
        std::cout << "\n";
        for (const auto& error : errors) {
            if (error.code.find("SYNTHETIC") != std::string::npos) {
                std::cout << fmt.badge(BadgeType::Info, error.code) << "\n";
            } else if (error.code.find("UNAVAILABLE") != std::string::npos) {
                std::cout << fmt.badge(BadgeType::Error, error.code) << "\n";
            } else {
                std::cout << fmt.badge(BadgeType::Warning, error.code) << "\n";
            }
            std::cout << "  " << error.message << "\n\n";
        }
    }
}

void print_doctor(const Hub& hub, const Formatter& fmt) {
    const auto env = hub.environment();
    const auto caps = hub.capability_snapshot();
    const auto telem = hub.telemetry();
    auto errors = hub.diagnose();
    
    std::cout << Banner::small(fmt);
    std::cout << fmt.section("Driver Doctor - Comprehensive Health Check");
    
    int pass_count = 0, warn_count = 0, error_count = 0;
    
    // Hardware check
    std::cout << fmt.subsection("Hardware");
    if (caps.capabilities.identity.variant != isb::cal::GpuVariant::Unknown) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " GPU detected: " 
                  << caps.capabilities.identity.model_name << "\n";
        pass_count++;
    } else {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " GPU variant unknown\n";
        warn_count++;
    }
    
    // Driver check
    std::cout << fmt.subsection("Driver Stack");
    if (env.mode != ProviderMode::Unavailable) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " Provider available (" 
                  << to_string(env.mode) << ")\n";
        pass_count++;
    } else {
        std::cout << fmt.badge(BadgeType::Error, "FAIL") << " Provider unavailable\n";
        error_count++;
    }
    
    // CUDA check
    std::cout << fmt.subsection("CUDA Runtime");
    if (caps.capabilities.compute.cuda_state == isb::cal::CapabilityState::Available) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " CUDA available\n";
        pass_count++;
    } else if (caps.capabilities.compute.cuda_state == isb::cal::CapabilityState::Unknown) {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " CUDA state unknown\n";
        warn_count++;
    } else {
        std::cout << fmt.badge(BadgeType::Error, "FAIL") << " CUDA unavailable\n";
        error_count++;
    }
    
    // Vulkan check
    std::cout << fmt.subsection("Vulkan Runtime");
    if (caps.capabilities.graphics.vulkan_state == isb::cal::CapabilityState::Available) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " Vulkan available\n";
        pass_count++;
    } else if (caps.capabilities.graphics.vulkan_state == isb::cal::CapabilityState::Unknown) {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " Vulkan state unknown\n";
        warn_count++;
    } else {
        std::cout << fmt.badge(BadgeType::Error, "FAIL") << " Vulkan unavailable\n";
        error_count++;
    }
    
    // Memory/ECC check
    std::cout << fmt.subsection("Memory & ECC");
    if (caps.capabilities.hardware.hbm2.state == isb::cal::CapabilityState::Available) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " HBM2 detected\n";
        pass_count++;
    } else {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " HBM2 state unknown\n";
        warn_count++;
    }
    
    // Telemetry check
    std::cout << fmt.subsection("Telemetry");
    if (telem.temperature_c.has_value()) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " Temperature monitoring active\n";
        pass_count++;
    } else {
        std::cout << fmt.badge(BadgeType::Warning, "WARN") << " Temperature unavailable\n";
        warn_count++;
    }
    
    // Errors from diagnose
    std::cout << fmt.subsection("Detected Issues");
    if (errors.empty()) {
        std::cout << fmt.badge(BadgeType::Success, "PASS") << " No issues detected\n";
        pass_count++;
    } else {
        for (const auto& error : errors) {
            std::cout << fmt.badge(BadgeType::Warning, "ISSUE") << " " << error.message << "\n";
        }
    }
    
    // Summary
    std::cout << fmt.section("Summary");
    std::cout << fmt.kv("Passed", std::to_string(pass_count)) << "\n";
    std::cout << fmt.kv("Warnings", std::to_string(warn_count)) << "\n";
    std::cout << fmt.kv("Errors", std::to_string(error_count)) << "\n";
    
    if (error_count > 0) {
        std::cout << "\n" << fmt.badge(BadgeType::Error, "ACTION REQUIRED") 
                  << " Critical issues detected. Review above.\n";
    } else if (warn_count > 0) {
        std::cout << "\n" << fmt.badge(BadgeType::Warning, "REVIEW ADVISED") 
                  << " Some features may be limited.\n";
    } else {
        std::cout << "\n" << fmt.badge(BadgeType::Success, "HEALTHY") 
                  << " System appears healthy.\n";
    }
    
    std::cout << "\n";
}

void print_verify(const Hub& hub, const Formatter& fmt) {
    auto result = hub.verify();
    
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Verification Result");
    
    if (result.verified) {
        std::cout << fmt.badge(BadgeType::Success, "VERIFIED") << "\n";
    } else {
        std::cout << fmt.badge(BadgeType::Warning, "NOT VERIFIED") << "\n";
    }
    
    std::cout << fmt.kv("Code", result.code) << "\n";
    std::cout << fmt.kv("Message", result.message) << "\n";
    std::cout << fmt.kv("Provider", result.provenance.provider) << "\n";
    
    std::cout << "\n";
}

void print_benchmark(const Hub& hub, const Formatter& fmt) {
    auto result = hub.benchmark();
    
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Benchmark Result");
    
    std::cout << fmt.kv("Name", result.name) << "\n";
    std::cout << fmt.kv("Version", result.version) << "\n";
    std::cout << fmt.kv("Synthetic", result.synthetic ? "Yes" : "No") << "\n";
    std::cout << fmt.kv("Correctness Verified", result.correctness_verified ? "Yes" : "No") << "\n";
    std::cout << fmt.kv("Result", result.result) << "\n";
    std::cout << fmt.kv("Provenance", result.provenance.provider) << "\n";
    
    if (result.synthetic) {
        std::cout << "\n" << fmt.badge(BadgeType::Warning, "FIXTURE ONLY - NO PERFORMANCE CLAIM") << "\n";
    }
    
    std::cout << "\n";
}

void print_report_result(const ReportBundle& report, const Formatter& fmt) {
    std::cout << Banner::compact(fmt);
    std::cout << fmt.section("Report Generated");
    std::cout << fmt.kv("Directory", report.directory) << "\n";
    std::cout << fmt.kv("Manifest", report.manifest.empty() ? "N/A" : "Generated") << "\n";
    std::cout << "\n";
}

} // anonymous namespace

int main(int argc, char** argv) {
    bool mock = false;
    bool json_output = false;
    bool dry = false;
    ColorPolicy color_policy = ColorPolicy::Auto;
    int monitor_interval = 2; // default 2 seconds
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
        }
        else if (x == "--color") {
            if (i + 1 < argc) {
                std::string val = argv[++i];
                if (val == "auto") color_policy = ColorPolicy::Auto;
                else if (val == "always") color_policy = ColorPolicy::Always;
                else if (val == "never") color_policy = ColorPolicy::Never;
            }
        }
        else if (x == "--interval" && i + 1 < argc) {
            monitor_interval = std::stoi(argv[++i]);
        }
        else args.push_back(x);
    }

    Formatter fmt(color_policy);

    if (args.empty()) {
        std::cerr << "usage: isb [--mock] [--json] [--color POLICY] [--help] <command>\n";
        std::cerr << "Run 'isb --help' for more information.\n";
        return 2;
    }

    // Handle render commands
    if (args[0] == "render") {
        if (args.size() < 2) {
            std::cerr << "usage: isb render <detect|plan|configure|verify> [--apply]\n";
            return 2;
        }

        auto render_manager = isb::graphics::create_render_path_manager(mock);

        if (args[1] == "detect") {
            const auto result = render_manager->detect_gpus();
            if (json_output) {
                print_render_detection(result, fmt);
            } else {
                print_render_detection(result, fmt);
            }
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

    // Handle graphics/upscaler commands
    if (args[0] == "graphics" || args[0] == "upscaler") {
        if (args.size() < 2) {
            std::cerr << "usage: isb " << args[0] << " <status|list>\n";
            return 2;
        }
        
        if (args[1] == "status" || args[1] == "list") {
            auto manager = isb::graphics::create_upscaler_manager(mock);
            auto backends = manager->list_backends();
            
            std::cout << Banner::compact(fmt);
            std::cout << fmt.section("Upscaler Backends");
            
            if (backends.empty()) {
                std::cout << fmt.badge(BadgeType::Info, "No backends detected") << "\n";
            }
            
            for (const auto& b : backends) {
                std::cout << "\n" << fmt.bold << isb::graphics::to_string(b.type) 
                          << ": " << b.name << fmt.reset() << "\n";
                std::cout << fmt.kv("Version", b.version) << "\n";
                
                if (b.available) {
                    std::cout << fmt.badge(BadgeType::Success, "Available") << "\n";
                } else if (b.supported) {
                    std::cout << fmt.badge(BadgeType::Warning, "Supported, not installed") << "\n";
                } else {
                    std::cout << fmt.badge(BadgeType::Error, "Boundary/Unsupported") << "\n";
                }
                
                if (b.requires_external) {
                    std::cout << fmt.kv("Requires", b.external_component) << "\n";
                }
                if (b.provenance.synthetic) {
                    std::cout << fmt.badge(BadgeType::Warning, "MOCK") << "\n";
                }
            }
            std::cout << "\n";
            return 0;
        } else {
            std::cerr << "unknown graphics command: " << args[1] << "\n";
            return 2;
        }
    }

    // Create Hub with appropriate provider
    std::unique_ptr<Provider> provider =
        mock ? std::unique_ptr<Provider>(new MockProvider)
             : std::unique_ptr<Provider>(new UnavailableProvider);
    Hub hub(*provider);

    // Command dispatch
    if (args[0] == "status") {
        if (json_output) {
            std::cout << hub.status_json() << "\n";
        } else {
            print_status(hub, fmt);
        }
    } else if (args[0] == "info") {
        if (json_output) {
            std::cout << cal::to_json(hub.capabilities()) << "\n";
        } else {
            print_info(hub, fmt);
        }
    } else if (args[0] == "inspect" || args[0] == "capabilities") {
        if (json_output) {
            std::cout << isb::cal::to_json(hub.capabilities()) << "\n";
        } else {
            print_capabilities(hub, fmt);
        }
    } else if (args[0] == "telemetry") {
        if (json_output) {
            std::cout << json(hub.telemetry()) << "\n";
        } else {
            print_telemetry(hub, fmt);
        }
    } else if (args[0] == "monitor") {
        run_monitor(hub, fmt, monitor_interval);
        return 0;
    } else if (args[0] == "profile" && args.size() > 1 && args[1] == "list") {
        print_profile_list(hub, fmt);
        return 0;
    } else if (args[0] == "profile" && args.size() > 2 && args[1] == "plan") {
        if (json_output) {
            std::cout << json(hub.profile_plan(args[2])) << "\n";
        } else {
            print_profile_plan(hub, args[2], fmt);
        }
        return 0;
    } else if (args[0] == "optimize") {
        if (json_output) {
            std::cout << json(hub.optimize_plan()) << "\n";
        } else {
            print_optimize_plan(hub, fmt);
        }
        return 0;
    } else if (args[0] == "diagnose") {
        print_diagnostics(hub, fmt);
        return 0;
    } else if (args[0] == "doctor") {
        print_doctor(hub, fmt);
        return 0;
    } else if (args[0] == "verify") {
        if (json_output) {
            std::cout << json(hub.verify()) << "\n";
        } else {
            print_verify(hub, fmt);
        }
        return 0;
    } else if (args[0] == "benchmark") {
        if (json_output) {
            std::cout << json(hub.benchmark()) << "\n";
        } else {
            print_benchmark(hub, fmt);
        }
        return 0;
    } else if (args[0] == "report") {
        auto report = hub.report(args.size() > 1 ? args[1] : "isb-report");
        print_report_result(report, fmt);
        return 0;
    } else if (args[0] == "about") {
        std::cout << Banner::small(fmt);
        std::cout << fmt.section("About ISB V100 Driver");
        std::cout << "Intelligent Systems Bureau Tesla V100 Control Layer\n\n";
        std::cout << fmt.kv("Version", "0.1.0") << "\n";
        std::cout << fmt.kv("Target", "Tesla V100 SXM2 16GB") << "\n";
        std::cout << fmt.kv("Architecture", "Volta GV100") << "\n";
        std::cout << fmt.kv("Repository", "github.com/AL3XZIG/Isb-tesla-v100-driver") << "\n";
        std::cout << "\n" << fmt.dim << "This is a user-space control plane, not a replacement NVIDIA driver.\n" << fmt.reset();
        std::cout << "\n";
        return 0;
    } else {
        std::cerr << "unknown command: " << args[0] << "\n";
        std::cerr << "Run 'isb --help' for available commands.\n";
        return 2;
    }

    return 0;
}
