#pragma once

#include "isb/hub/hub.hpp"

#include <cstdint>
#include <string>
#include <vector>

namespace isb::performance {

/// Why a hardware limit is currently being reduced. Detection is evidence
/// based: unknown states are reported as Unknown, never guessed away.
enum class ThrottleReason : std::uint32_t {
    None = 0,
    HwSlowdown = 1u << 0,           ///< HW thermal slowdown (NVML)
    SwThermal = 1u << 1,            ///< SW thermal slowdown (NVML)
    PowerBrake = 1u << 2,           ///< Power braking / low power source (NVML)
    DisplayClockSettingChange = 1u << 3,
    ClocksEventReasonUnknown = 1u << 4,
};

struct ThrottleState {
    /// Raw provider bitmask (NVML retired-clocks reasons). Empty when the
    /// provider does not expose throttling state at all.
    bool observed = false;
    std::uint32_t raw_bits = 0;
    std::vector<ThrottleReason> active_reasons;
    bool throttling = false;
};

/// Extension of the Hub telemetry contract with V100 tuning-relevant fields.
/// The base snapshot keeps its provenance/synthetic guarantees; everything
/// added here remains optional and is only filled when a provider reports it.
struct PerformanceTelemetry {
    hub::TelemetrySnapshot base;
    ThrottleState throttle;
    /// ECC error counters where exposed (Volatile totals from NVML).
    bool ecc_errors_observed = false;
    std::uint64_t ecc_corrected_errors = 0;
    std::uint64_t ecc_uncorrected_errors = 0;
    /// PCIe traffic counters in bytes since driver load, where exposed.
    bool pcie_traffic_observed = false;
    std::uint64_t pcie_tx_bytes = 0;
    std::uint64_t pcie_rx_bytes = 0;

    /// True when at least one tuning-relevant field was actually observed by a
    /// real provider (as opposed to being synthesized or left unknown).
    bool has_evidence() const noexcept {
        return throttle.observed || ecc_errors_observed || pcie_traffic_observed ||
               !base.synthetic;
    }
};

/// Maps an NVML `nvmlDeviceGetCurrentClocksThrottleReasons` bitmask onto the
/// provider-independent throttle state. Bit layout follows the NVML
/// `NVML_CLOCKS_THROTTLE_REASON_*` values so the mapping stays auditable.
/// `observed` must be non-zero only when the query itself succeeded; a failed
/// query yields `ThrottleState::observed == false`, never a guessed "no
/// throttling".
ThrottleState throttle_state_from_nvml_bits(std::uint32_t observed, std::uint32_t bits) noexcept;

/// Serializable JSON for the extended telemetry view. Unobserved optional
/// sections are emitted as null — absence is never presented as zero.
std::string to_json(const PerformanceTelemetry& telemetry);

} // namespace isb::performance
