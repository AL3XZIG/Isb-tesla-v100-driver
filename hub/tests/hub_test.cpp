#include "isb/hub/hub.hpp"
#include <cassert>
#include <filesystem>
#include <string>

using namespace isb::hub;

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
    assert(plan.operations.empty() && !plan.unknown.empty());
    assert(plan.dry_run);
    // Mock telemetry is synthetic: the optimize plan must disclose it.
    assert([&] {
        for (const auto& r : plan.risks)
            if (r.find("synthetic") != std::string::npos) return true;
        return false;
    }());

    // Throttling evidence from a provider must surface as an optimize risk,
    // never as an automatically proposed mutation. MockProvider is final, so
    // compose it instead of deriving from it.
    struct ThrottledProvider final : Provider {
        explicit ThrottledProvider(const MockProvider& base) : base_(base) {}
        Environment environment() const override { return base_.environment(); }
        isb::cal::GpuCapabilities capabilities() const override { return base_.capabilities(); }
        TelemetrySnapshot telemetry() const override {
            auto t = base_.telemetry();
            t.throttling_hint = "throttling";
            return t;
        }
        std::vector<Control> controls() const override { return base_.controls(); }

        const MockProvider& base_;
    };
    ThrottledProvider throttled{mock};
    Hub ht(throttled);
    auto tplan = ht.optimize_plan();
    assert(tplan.operations.empty());
    bool throttle_risk = false;
    for (const auto& r : tplan.risks)
        if (r.find("Throttling") != std::string::npos) throttle_risk = true;
    assert(throttle_risk);

    // Telemetry JSON exposes the throttling observation explicitly.
    assert(json(ht.telemetry()).find("\"throttling\":\"throttling\"") != std::string::npos);
    assert(json(h.telemetry()).find("\"throttling\":\"unknown\"") != std::string::npos);

    auto no = h.apply(plan, false);
    assert(!no.mutated && !no.verification.verified);

    auto applied = h.apply(plan, true);
    assert(!applied.mutated);

    // OperationPlan JSON must expose the full public contract: operations,
    // unsupported/unknown lists, risks and verification steps. A hardcoded
    // empty "operations" array here used to silently drop profile_plan()
    // data from CLI/GUI consumers.
    {
        const auto pplan = h.profile_plan("Balanced");
        const auto pjson = json(pplan);
        assert(pjson.find("\"operations\":[") != std::string::npos);
        assert(pjson.find("\"risks\":[") != std::string::npos);
        assert(pjson.find("policy templates") != std::string::npos);
        assert(pjson.find("\"verification_steps\":[") != std::string::npos);
        assert(pjson.find("Read back every applied setting.") != std::string::npos);
        assert(pjson.find("\"unknown\":[") != std::string::npos);
        assert(pjson.find("\"unsupported\":[]") != std::string::npos);
    }
    // A plan that actually contains operations serializes them (synthetic
    // provider yields none because all mock controls are Unknown; construct a
    // supported control explicitly to prove the array is not hardcoded).
    {
        struct SupportedProvider final : Provider {
            explicit SupportedProvider(const MockProvider& base) : base_(base) {}
            Environment environment() const override { return base_.environment(); }
            isb::cal::GpuCapabilities capabilities() const override { return base_.capabilities(); }
            TelemetrySnapshot telemetry() const override { return base_.telemetry(); }
            std::vector<Control> controls() const override {
                auto c = base_.controls();
                for (auto& x : c) { x.state = ControlState::Supported; x.reason.clear(); }
                return c;
            }
            const MockProvider& base_;
        };
        SupportedProvider sup{mock};
        Hub hs(sup);
        const auto pjson = json(hs.profile_plan("Gaming"));
        assert(pjson.find("\"id\":\"persistence_mode\"") != std::string::npos);
        assert(pjson.find("Apply Gaming policy") != std::string::npos);
        assert(pjson.find("\"requires_approval\":true") != std::string::npos);
    }

    // Result JSON keeps the provenance evidence fields the structs define.
    assert(json(h.benchmark()).find("\"provenance\":{\"provider\":\"mock-provider\"") != std::string::npos);
    assert(json(h.verify()).find("\"provenance\"") != std::string::npos);

    assert(h.benchmark().synthetic && h.benchmark().correctness_verified);

    auto report = h.report("hub-test-report");
    assert(std::filesystem::exists(report.directory + "/manifest.json"));
    std::filesystem::remove_all(report.directory);

    UnavailableProvider unavailable;
    Hub u(unavailable);
    auto unavailable_status = u.status_json();
    assert(unavailable_status.find("\"mode\":\"unavailable\"") != std::string::npos);
    assert(unavailable_status.find("\"driver\":\"unknown\"") != std::string::npos);
    assert(!u.verify().verified);

    return 0;
}
