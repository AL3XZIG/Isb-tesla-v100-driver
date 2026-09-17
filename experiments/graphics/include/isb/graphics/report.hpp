#pragma once

#include "isb/graphics/adapter.hpp"

#include <string>
#include <vector>

namespace isb::graphics {

struct UnsupportedFeature {
    std::string feature;
    std::string reason;
};

class GraphicsReport {
public:
    explicit GraphicsReport(const GraphicsAdapter& adapter);

    const std::string& gpu_identity() const noexcept;
    const std::vector<ObservedGraphicsBackend>& backends() const noexcept;
    const std::vector<UnsupportedFeature>& unsupported_features() const noexcept;
    std::string to_text() const;
    std::string to_json() const;

private:
    std::string gpu_identity_;
    GraphicsCapability capability_;
    std::vector<ObservedGraphicsBackend> backends_;
    std::vector<UnsupportedFeature> unsupported_features_;
};

} // namespace isb::graphics
