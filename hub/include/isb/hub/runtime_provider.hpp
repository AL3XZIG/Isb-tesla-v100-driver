#pragma once

#include "isb/hub/hub.hpp"

#include <memory>

namespace isb::hub {

/// Runtime provider that aggregates real optional providers without exposing
/// provider-specific APIs to CLI/GUI. It is read-only by design.
class RuntimeProvider final : public Provider {
public:
    Environment environment() const override;
    cal::GpuCapabilities capabilities() const override;
    TelemetrySnapshot telemetry() const override;
    std::vector<Control> controls() const override;
};

std::unique_ptr<Provider> make_runtime_provider();

} // namespace isb::hub
