#pragma once

#include "isb/hub/hub.hpp"
#include "isb/performance/throttling.hpp"
#include "isb/providers/nvml/provider.hpp"
#include <vector>

namespace isb::providers::nvml {

/// Adapts read-only NVML observations to the Hub Provider contract.
/// This class performs no hardware mutation.
class HubProvider final : public hub::Provider {
public:
    hub::Environment environment() const override;
    cal::GpuCapabilities capabilities() const override;
    hub::TelemetrySnapshot telemetry() const override;
    std::vector<hub::Control> controls() const override;

    performance::PerformanceTelemetry performance_telemetry() const;
    std::vector<RawGpuObservation> observe_devices() const;
};

} // namespace isb::providers::nvml
