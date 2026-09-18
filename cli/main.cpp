#include "isb/hub/hub.hpp"
#include "isb/graphics/upscaler_types.hpp"
#include <iostream>
#include <memory>
using namespace isb::hub;

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
  graphics status                 Show graphics upscaling status
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
             : std::unique_ptr<Provider>(new UnavailableProvider);
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
