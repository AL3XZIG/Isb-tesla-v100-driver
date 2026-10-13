#pragma once

#include "isb/hub/hub.hpp"
#include "isb/performance/throttling.hpp"
#include "isb/providers/nvml/provider.hpp"

namespace isb::providers::nvml {

/// Adapts read-only NVML observations to the Hub Provider contract.
/// This class performs no hardware mutation.
class HubProvider final : public hub::Provider {
public:
    hub::Environment environment() const override;
    cal::GpuCapabilities capabilities() const override;
    hub::TelemetrySnapshot telemetry() const override;
    std::vector<hub::Control> controls() const override;

    /// Extended V100 telemetry view: base snapshot plus throttle detection,
    /// ECC counters and PCIe traffic where the driver exposes them. Absent
    /// fields stay unknown — they are never defaulted to healthy values.
    performance::PerformanceTelemetry performance_telemetry() const;
};

} // namespace isb::providers::nvml
