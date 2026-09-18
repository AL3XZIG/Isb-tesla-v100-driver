#include "isb/hub/hub.hpp"
#include <iostream>
#include <memory>
using namespace isb::hub;

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
        else args.push_back(x);
    }

    if (args.empty()) {
        std::cerr << "usage: isb [--mock] [--json] status|inspect|capabilities|telemetry|profile list|profile plan <name>|optimize|diagnose|verify|benchmark|report <dir>\n";
        return 2;
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
        std::cerr << "unknown command\n";
        return 2;
    }

    std::cout << output << '\n';
    return 0;
}
