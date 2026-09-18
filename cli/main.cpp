#include "isb/hub/hub.hpp"
#include "isb/hub/runtime_provider.hpp"
#include "isb/graphics/upscaler_types.hpp"
#include "isb/graphics/render_path_types.hpp"
#include <algorithm>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
using namespace isb::hub;

namespace {
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

void print_render_detection(const isb::graphics::RenderPathResult& result) {
    std::cout << "{\n";
    std::cout << "  \"status\": \"" << isb::common::to_string(result.status) << "\",\n";
    std::cout << "  \"message\": \"" << json_escape(result.message) << "\",\n";
    std::cout << "  \"synthetic\": " << (result.provenance.synthetic ? "true" : "false") << ",\n";
    std::cout << "  \"gpus\": [";
    if (result.config) {
        for (std::size_t i = 0; i < result.config->gpus.size(); ++i) {
            const auto& gpu = result.config->gpus[i];
            if (i != 0) std::cout << ",";
            std::cout << "\n    {\"name\":\"" << json_escape(gpu.name)
                      << "\",\"vendor\":\"" << json_escape(gpu.vendor)
                      << "\",\"pci_bus\":\"" << json_escape(gpu.pci_bus)
                      << "\",\"role\":\"" << isb::graphics::to_string(gpu.role)
                      << "\",\"driver_model\":\"" << json_escape(gpu.driver_model)
                      << "\",\"display\":" << (gpu.has_display_outputs ? "true" : "false")
                      << ",\"render_target\":" << (gpu.is_render_target ? "true" : "false")
                      << "}";
        }
        std::cout << "\n  ";
    }
    std::cout << "]\n}\n";
}
} // namespace

void print_help() {
    std::cout << R"(ISB Tesla V100 Driver - Command Line Interface

Usage: isb [options] command [args...]

Options:
  --mock       Use mock provider (no real hardware access)
  --json       Output in JSON format
  --dry-run    Show plan without applying changes
  --help       Show this help message

Commands:
  status                          Show system status
  inspect                         Show GPU capabilities
  capabilities                    Alias for inspect
  telemetry                       Show telemetry data
  profile list                    List available profiles
  profile plan <name>             Show profile application plan
  optimize                        Show optimization plan
  diagnose                        Run diagnostics
  verify                          Verify system state
  benchmark                       Run benchmarks
  report <dir>                    Generate report to directory
  render detect                   Detect GPU roles and render path\n  render plan                     Show render path plan\n  render configure --apply        Apply render path configuration (if supported)\n  render verify                   Verify render path configuration\n  graphics status                 Show graphics upscaling status
  graphics list                   List available upscaler backends
  upscaler status                 Alias for graphics status
  upscaler list                   Alias for graphics list

Examples:
  isb --mock status
  isb --mock --json capabilities
  isb --mock profile plan appearance
  isb --mock optimize
  isb --mock graphics status
)";
}

int main(int argc, char** argv) {
    bool mock = false;
    bool json_output = false;
    bool dry = false;
    std::vector<std::string> args;

    for (int i = 1; i < argc; ++i) {
        std::string x = argv[i];
        if (x == "--mock") mock = true;
        else if (x == "--json") json_output = true;
        else if (x == "--dry-run") dry = true;
        else if (x == "--help") {
            print_help();
            return 0;
        }
        else args.push_back(x);
    }

    if (args.empty()) {
        std::cerr << "usage: isb [--mock] [--json] [--dry-run] [--help] <command>\n";
        std::cerr << "Run 'isb --help' for more information.\n";
        return 2;
    }

    if (args[0] == "render") {
        if (args.size() < 2) {
            std::cerr << "usage: isb render <detect|plan|configure|verify> [--apply]\n";
            return 2;
        }

        auto render_manager = isb::graphics::create_render_path_manager(mock);

        if (args[1] == "detect") {
            const auto result = render_manager->detect_gpus();
            if (json_output) {
                print_render_detection(result);
            } else {
                std::cout << result.message << "\\n";
                if (result.config) {
                    for (std::size_t i = 0; i < result.config->gpus.size(); ++i) {
                        const auto& gpu = result.config->gpus[i];
                        std::cout << "[" << i << "] " << gpu.name
                                  << " | role=" << isb::graphics::to_string(gpu.role)
                                  << " | PCI=" << (gpu.pci_bus.empty() ? "Unknown" : gpu.pci_bus)
                                  << " | display=" << (gpu.has_display_outputs ? "yes" : "no")
                                  << " | render=" << (gpu.is_render_target ? "yes" : "no")
                                  << " | driver=" << (gpu.driver_model.empty() ? "Unknown" : gpu.driver_model)
                                  << "\\n";
                    }
                }
            }
            return 0;
        }
        if (args[1] == "plan") {
            const auto result = render_manager->plan_config();
            std::cout << result.message << "\\n";
            return result.status == isb::common::CapabilityState::Error ? 1 : 0;
        }
        if (args[1] == "configure") {
            const bool apply = std::find(args.begin() + 2, args.end(), "--apply") != args.end();
            const auto result = render_manager->apply_config(apply);
            std::cout << result.message << "\\n";
            return result.status == isb::common::CapabilityState::Error ? 1 : 0;
        }
        if (args[1] == "verify") {
            const auto result = render_manager->verify_config();
            std::cout << result.message << "\\n";
            return result.status == isb::common::CapabilityState::Error ? 1 : 0;
        }
        std::cerr << "unknown render command: " << args[1] << "\\n";
        return 2;
    }

    // Handle graphics/upscaler commands
    if (args[0] == "graphics" || args[0] == "upscaler") {
        if (args.size() < 2) {
            std::cerr << "usage: isb " << args[0] << " <status|list>\n";
            return 2;
        }
        
        if (args[1] == "status" || args[1] == "list") {
            // Create upscaler manager and show status
            auto manager = isb::graphics::create_upscaler_manager(mock);
            auto backends = manager->list_backends();
            
            if (json_output) {
                std::cout << "{\n  \"backends\": [\n";
                bool first = true;
                for (const auto& b : backends) {
                    if (!first) std::cout << ",\n";
                    first = false;
                    std::cout << "    {\n";
                    std::cout << "      \"type\": \"" << isb::graphics::to_string(b.type) << "\",\n";
                    std::cout << "      \"name\": \"" << b.name << "\",\n";
                    std::cout << "      \"version\": \"" << b.version << "\",\n";
                    std::cout << "      \"available\": " << (b.available ? "true" : "false") << ",\n";
                    std::cout << "      \"supported\": " << (b.supported ? "true" : "false") << ",\n";
                    std::cout << "      \"requires_external\": " << (b.requires_external ? "true" : "false") << "\n";
                    if (b.requires_external) {
                        std::cout << "      \"external_component\": \"" << b.external_component << "\"\n";
                    }
                    std::cout << "    }";
                }
                std::cout << "\n  ]\n}\n";
            } else {
                std::cout << "Upscaler Backends:\n";
                for (const auto& b : backends) {
                    std::cout << "  " << isb::graphics::to_string(b.type) << ": " << b.name;
                    if (b.available) {
                        std::cout << " [Available]";
                    } else if (b.supported) {
                        std::cout << " [Supported, not installed]";
                    } else {
                        std::cout << " [Boundary/Unsupported]";
                    }
                    if (b.requires_external) {
                        std::cout << " (requires: " << b.external_component << ")";
                    }
                    if (b.provenance.synthetic) {
                        std::cout << " [MOCK]";
                    }
                    std::cout << "\n";
                }
            }
            return 0;
        } else {
            std::cerr << "unknown graphics command: " << args[1] << "\n";
            return 2;
        }
    }

    std::unique_ptr<Provider> provider =
        mock ? std::unique_ptr<Provider>(new MockProvider)
             : make_runtime_provider();
    Hub hub(*provider);

    std::string output;
    if (args[0] == "status") {
        output = hub.status_json();
    } else if (args[0] == "inspect" || args[0] == "capabilities") {
        output = isb::cal::to_json(hub.capabilities());
    } else if (args[0] == "telemetry") {
        output = json(hub.telemetry());
    } else if (args[0] == "profile" && args.size() > 1 && args[1] == "list") {
        for (const auto& profile : hub.profiles()) std::cout << profile << '\n';
        return 0;
    } else if (args[0] == "profile" && args.size() > 2 && args[1] == "plan") {
        output = json(hub.profile_plan(args[2]));
    } else if (args[0] == "optimize") {
        output = json(hub.optimize_plan());
    } else if (args[0] == "diagnose") {
        for (const auto& error : hub.diagnose())
            std::cout << error.code << ": " << error.message << '\n';
        return 0;
    } else if (args[0] == "verify") {
        output = json(hub.verify());
    } else if (args[0] == "benchmark") {
        output = json(hub.benchmark());
    } else if (args[0] == "report") {
        auto report = hub.report(args.size() > 1 ? args[1] : "isb-report");
        std::cout << report.directory << '\n';
        return 0;
    } else {
        std::cerr << "unknown command: " << args[0] << "\n";
        std::cerr << "Run 'isb --help' for available commands.\n";
        return 2;
    }

    std::cout << output << '\n';
    return 0;
}
