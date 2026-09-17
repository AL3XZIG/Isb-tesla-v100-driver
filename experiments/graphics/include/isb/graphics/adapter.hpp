#pragma once

#include "isb/cal/capabilities.hpp"
#include "isb/graphics/backend.hpp"
#include "isb/graphics/capability.hpp"

#include <string>
#include <vector>

namespace isb::graphics {

/// Combines CAL/NVML observations with lightweight user-space backend probes.
/// Probes never benchmark or execute compute workloads.
class GraphicsAdapter {
public:
    explicit GraphicsAdapter(cal::GpuCapabilities capabilities = {});

    void probe();
    const GraphicsCapability& capability() const noexcept;
    const std::string& gpu_identity() const noexcept;
    std::vector<ObservedGraphicsBackend> backends() const;

private:
    cal::GpuCapabilities cal_capabilities_;
    GraphicsCapability capability_;
    std::string gpu_identity_;
};

} // namespace isb::graphics
