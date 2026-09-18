#include "isb/optiscaler/manager.hpp"

#include <fstream>
#include <utility>

namespace isb::optiscaler {
namespace {

std::string read_first_line(const std::filesystem::path& path) {
    std::ifstream input(path);
    std::string value;
    std::getline(input, value);
    return value;
}

} // namespace

Manager::Manager(std::filesystem::path component_root)
    : root_(std::move(component_root)) {}

DetectionResult Manager::detect() const {
    DetectionResult result;

    if (!std::filesystem::exists(root_)) {
        result.reason = "OptiScaler component directory is not installed";
        return result;
    }

    const auto manifest_path = root_ / "component.version";
    const auto license_path = root_ / "LICENSE";
    const auto marker_path = root_ / "OptiScaler.ini";

    if (!std::filesystem::is_regular_file(manifest_path) ||
        !std::filesystem::is_regular_file(license_path) ||
        !std::filesystem::is_regular_file(marker_path)) {
        result.state = InstallState::Invalid;
        result.reason = "OptiScaler component is present but its managed package markers are incomplete";
        return result;
    }

    result.manifest.id = "optiscaler";
    result.manifest.version = read_first_line(manifest_path);
    result.manifest.license = "GPL-3.0";
    result.manifest.upstream = "https://github.com/optiscaler/OptiScaler";
    result.manifest.provenance = "Official OptiScaler release artifact";

    if (result.manifest.version.empty()) {
        result.state = InstallState::Invalid;
        result.reason = "OptiScaler component version marker is empty";
        return result;
    }

    result.state = InstallState::Installed;
    result.reason = "OptiScaler is installed from a managed offline component package";
    return result;
}

} // namespace isb::optiscaler
