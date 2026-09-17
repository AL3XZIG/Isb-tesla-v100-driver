#pragma once

#include "isb/cal/capabilities.hpp"

namespace isb::compute {

enum class Backend {
    CUDA,
    VulkanCompute,
    OpenCL,
};

/// Lifecycle state for a selected compute backend.
///
/// Initialization does not load a runtime or create a GPU context. It only
/// activates this infrastructure object when the supplied capability
/// observation says that the selected backend is available.
class ComputeContext {
public:
    explicit ComputeContext(Backend backend = Backend::CUDA,
                            cal::CapabilityState availability = cal::CapabilityState::Unknown) noexcept;

    bool initialize() noexcept;
    bool available() const noexcept;
    void shutdown() noexcept;
    Backend backend() const noexcept;

private:
    Backend backend_;
    cal::CapabilityState availability_;
    bool initialized_ = false;
};

} // namespace isb::compute
