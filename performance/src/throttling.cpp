#include "isb/performance/throttling.hpp"

#include "isb/common/json_writer.hpp"

#include <sstream>

namespace isb::performance {
namespace {

const char* reason_name(ThrottleReason reason) noexcept {
    switch (reason) {
    case ThrottleReason::HwSlowdown: return "hw_thermal_slowdown";
    case ThrottleReason::SwThermal: return "sw_thermal_slowdown";
    case ThrottleReason::PowerBrake: return "power_brake";
    case ThrottleReason::DisplayClockSettingChange: return "display_clock_setting_change";
    case ThrottleReason::ClocksEventReasonUnknown: return "unknown";
    case ThrottleReason::None: break;
    }
    return "none";
}

// Delegates to the shared RFC 8259-safe escaper (isb::common::json_writer);
// the previous local copy passed tabs/newlines through raw and produced
// invalid JSON for strings containing control characters.
std::string q(const std::string& s) {
    return isb::common::quoted_json_string(s);
}

} // namespace

ThrottleState throttle_state_from_nvml_bits(std::uint32_t observed, std::uint32_t bits) noexcept {
    ThrottleState state;
    state.observed = observed != 0U;
    state.raw_bits = bits;
    if (!state.observed) return state;

    if (bits & 1u) state.active_reasons.push_back(ThrottleReason::HwSlowdown);
    if (bits & 2u) state.active_reasons.push_back(ThrottleReason::SwThermal);
    if (bits & 4u) state.active_reasons.push_back(ThrottleReason::PowerBrake);
    if (bits & 8u) state.active_reasons.push_back(ThrottleReason::DisplayClockSettingChange);
    if (bits & ~7u) state.active_reasons.push_back(ThrottleReason::ClocksEventReasonUnknown);
    state.throttling = !state.active_reasons.empty();
    return state;
}

std::string to_json(const PerformanceTelemetry& telemetry) {
    std::ostringstream o;
    o << "{\"base\":" << hub::json(telemetry.base);

    o << ",\"throttle\":{\"observed\":" << (telemetry.throttle.observed ? "true" : "false");
    if (telemetry.throttle.observed) {
        o << ",\"raw_bits\":" << telemetry.throttle.raw_bits << ",\"reasons\":[";
        for (std::size_t i = 0; i < telemetry.throttle.active_reasons.size(); ++i) {
            if (i) o << ',';
            o << q(reason_name(telemetry.throttle.active_reasons[i]));
        }
        o << "],\"throttling\":" << (telemetry.throttle.throttling ? "true" : "false");
    }
    o << "}";

    o << ",\"ecc_errors\":null";
    if (telemetry.ecc_errors_observed) {
        o << ",\"ecc_errors\":{\"corrected\":" << telemetry.ecc_corrected_errors
          << ",\"uncorrected\":" << telemetry.ecc_uncorrected_errors << "}";
    }

    o << ",\"pcie_traffic\":null";
    if (telemetry.pcie_traffic_observed) {
        o << ",\"pcie_traffic\":{\"tx_bytes\":" << telemetry.pcie_tx_bytes
          << ",\"rx_bytes\":" << telemetry.pcie_rx_bytes << "}";
    }

    o << "}";
    return o.str();
}

} // namespace isb::performance
