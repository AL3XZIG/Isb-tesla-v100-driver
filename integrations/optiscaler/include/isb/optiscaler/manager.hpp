#pragma once

#include <filesystem>
#include <string>

namespace isb::optiscaler {

enum class InstallState {
    NotInstalled,
    Installed,
    Invalid,
};

struct ComponentManifest {
    std::string id;
    std::string version;
    std::string license;
    std::string upstream;
    std::string provenance;
};

struct DetectionResult {
    InstallState state = InstallState::NotInstalled;
    ComponentManifest manifest;
    std::string reason;
};

class Manager final {
public:
    explicit Manager(std::filesystem::path component_root);

    DetectionResult detect() const;

private:
    std::filesystem::path root_;
};

} // namespace isb::optiscaler
