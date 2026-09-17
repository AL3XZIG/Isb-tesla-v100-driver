#pragma once

#include "isb/cal/capabilities.hpp"

#include <string>

namespace isb::graphics {

enum class GraphicsBackend { CUDA, Vulkan, DirectX12, OpenCL, Unknown };

const char* to_string(GraphicsBackend backend) noexcept;

class IGraphicsBackend {
public:
    virtual ~IGraphicsBackend() = default;
    virtual bool available() const noexcept = 0;
    virtual GraphicsBackend type() const noexcept = 0;
    virtual std::string name() const = 0;
};

/// A value backend for results that were already observed by a probe or CAL.
class ObservedGraphicsBackend final : public IGraphicsBackend {
public:
    ObservedGraphicsBackend(GraphicsBackend type, bool available, std::string name);
    bool available() const noexcept override;
    GraphicsBackend type() const noexcept override;
    std::string name() const override;

private:
    GraphicsBackend type_;
    bool available_;
    std::string name_;
};

} // namespace isb::graphics
