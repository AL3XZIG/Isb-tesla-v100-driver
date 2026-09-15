#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace isb::integrations {

enum class ComponentState {
    Unknown,
    Missing,
    Present,
    Compatible,
    Incompatible,
    Invalid
};

struct ExternalComponent {
    std::string name;
    std::string version;
    std::string source_url;
    std::string license_spdx;
    std::string path;
    std::string sha256;
    ComponentState state = ComponentState::Unknown;
    std::vector<std::string> capabilities;
};

class ExternalComponentValidator {
public:
    static ComponentState validate(const ExternalComponent& component,
                                   const std::string& expected_architecture);
};

} // namespace isb::integrations
