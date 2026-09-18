#include "isb/hub/hub.hpp"
#include <cassert>
#include <filesystem>
#include <string>

int main() {
    isb::hub::MockProvider mock;
    isb::hub::Hub h(mock);

    auto t = h.telemetry();
    assert(t.synthetic && t.temperature_c == 42);
    assert(h.capabilities().hardware.rt_cores.state ==
           isb::cal::CapabilityState::Unavailable);

    auto status = h.status_json();
    assert(status.find("\"schema_version\":\"1\"") != std::string::npos);
    assert(status.find("\"mode\":\"mock\"") != std::string::npos);
    assert(status.find("\"capabilities\":") != std::string::npos);
    assert(status.find("\"telemetry\":") != std::string::npos);
    assert(status.find("\"synthetic\":true") != std::string::npos);

    auto plan = h.optimize_plan();
    assert(plan.operations.empty() && !plan.unknown.empty());

    auto no = h.apply(plan, false);
    assert(!no.mutated && !no.verification.verified);

    auto applied = h.apply(plan, true);
    assert(!applied.mutated);

    assert(h.benchmark().synthetic && h.benchmark().correctness_verified);

    auto report = h.report("hub-test-report");
    assert(std::filesystem::exists(report.directory + "/manifest.json"));
    std::filesystem::remove_all(report.directory);

    isb::hub::UnavailableProvider unavailable;
    isb::hub::Hub u(unavailable);
    auto unavailable_status = u.status_json();
    assert(unavailable_status.find("\"mode\":\"unavailable\"") != std::string::npos);
    assert(unavailable_status.find("\"driver\":\"unknown\"") != std::string::npos);
    assert(!u.verify().verified);

    return 0;
}
