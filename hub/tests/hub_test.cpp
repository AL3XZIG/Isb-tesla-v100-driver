#include "isb/hub/hub.hpp"
#include <cassert>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>

using namespace isb::hub;

namespace {

struct RealProvider final : Provider {
    Environment environment() const override {
        return {"Linux", "550.54.14", ProviderMode::Real,
                {"real-test", "non-synthetic test evidence", false}};
    }
    isb::cal::GpuCapabilities capabilities() const override {
        isb::cal::GpuCapabilities caps;
        caps.identity.vendor = "NVIDIA";
        caps.identity.model_name = "NVIDIA GeForce RTX 4090";
        caps.identity.exact_hardware_variant = caps.identity.model_name;
        return caps;
    }
    TelemetrySnapshot telemetry() const override {
        TelemetrySnapshot telemetry;
        telemetry.timestamp = "2026-01-01T00:00:00Z";
        telemetry.synthetic = false;
        telemetry.provenance = {"real-test", "non-synthetic test evidence", false};
        telemetry.driver = "550.54.14";
        return telemetry;
    }
    std::vector<Control> controls() const override { return {}; }
};

} // namespace

int main() {
    MockProvider mock;
    Hub h(mock);

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
    assert(plan.operations.empty() == false);
    assert(plan.dry_run);
    assert([&] {
        for (const auto& r : plan.risks)
            if (r.find("synthetic") != std::string::npos) return true;
        return false;
    }());

    struct ThrottledProvider final : Provider {
        explicit ThrottledProvider(const MockProvider& base) : base_(base) {}
        Environment environment() const override { return base_.environment(); }
        isb::cal::GpuCapabilities capabilities() const override { return base_.capabilities(); }
        TelemetrySnapshot telemetry() const override {
            auto telemetry = base_.telemetry();
            telemetry.throttling_hint = "throttling";
            return telemetry;
        }
        std::vector<Control> controls() const override { return base_.controls(); }
        const MockProvider& base_;
    };
    ThrottledProvider throttled{mock};
    Hub ht(throttled);
    auto tplan = ht.optimize_plan();
    bool throttle_risk = false;
    for (const auto& r : tplan.risks)
        if (r.find("Throttling") != std::string::npos) throttle_risk = true;
    assert(throttle_risk);

    assert(json(ht.telemetry()).find("\"throttling\":\"throttling\"") != std::string::npos);
    assert(json(h.telemetry()).find("\"throttling\":\"unknown\"") != std::string::npos);

    auto no = h.apply(h.profile_plan("Balanced"), false);
    assert(!no.mutated && !no.verification.verified);
    auto applied = h.apply(h.profile_plan("Balanced"), true);
    assert(!applied.mutated);

    assert(h.benchmark().synthetic && h.benchmark().correctness_verified);
    assert(h.diagnose().size() == 1);
    assert(h.diagnose().front().code == "ISB_SYNTHETIC_EVIDENCE");

    const auto temp = std::filesystem::temp_directory_path();
    const auto mock_report_dir = temp / "isb-hub-mock-report-test";
    std::filesystem::remove_all(mock_report_dir);
    auto report = h.report(mock_report_dir.string());
    assert(std::filesystem::exists(mock_report_dir / "manifest.json"));
    assert(report.manifest.find("\"environment_hash\":\"deterministic-mock-v1\"") != std::string::npos);
    assert(report.manifest.find("\"synthetic\":true") != std::string::npos);
    std::filesystem::remove_all(mock_report_dir);

    RealProvider real;
    Hub real_hub(real);
    assert(real_hub.diagnose().empty());
    assert(real_hub.status_json().find("\"synthetic\":false") != std::string::npos);
    const auto real_report_dir = temp / "isb-hub-real-report-test";
    std::filesystem::remove_all(real_report_dir);
    const auto real_report = real_hub.report(real_report_dir.string());
    assert(real_report.manifest.find("deterministic-mock-v1") == std::string::npos);
    assert(real_report.manifest.find("\"environment_hash\":\"fnv1a64-") != std::string::npos);
    assert(real_report.manifest.find("\"mode\":\"real\"") != std::string::npos);
    assert(real_report.manifest.find("\"synthetic\":false") != std::string::npos);
    std::filesystem::remove_all(real_report_dir);

    // A non-manifest write failure must not be reported as a successful bundle.
    const auto failed_report_dir = temp / "isb-hub-failed-report-test";
    std::filesystem::remove_all(failed_report_dir);
    std::filesystem::create_directories(failed_report_dir / "capabilities.json");
    bool report_threw = false;
    try {
        (void)real_hub.report(failed_report_dir.string());
    } catch (const std::runtime_error&) {
        report_threw = true;
    }
    assert(report_threw);
    std::filesystem::remove_all(failed_report_dir);

    UnavailableProvider unavailable;
    Hub u(unavailable);
    auto unavailable_status = u.status_json();
    assert(unavailable_status.find("\"mode\":\"unavailable\"") != std::string::npos);
    assert(unavailable_status.find("\"driver\":\"unknown\"") != std::string::npos);
    assert(u.diagnose().size() == 1);
    assert(u.diagnose().front().code == "ISB_PROVIDER_UNAVAILABLE");
    assert(!u.verify().verified);

    return 0;
}
