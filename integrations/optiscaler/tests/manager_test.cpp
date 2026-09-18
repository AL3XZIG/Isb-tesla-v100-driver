#include "isb/optiscaler/manager.hpp"

#include <cassert>
#include <filesystem>
#include <fstream>

int main() {
    const auto root = std::filesystem::temp_directory_path() / "isb-optiscaler-test";
    std::filesystem::remove_all(root);

    {
        isb::optiscaler::Manager manager(root);
        const auto result = manager.detect();
        assert(result.state == isb::optiscaler::InstallState::NotInstalled);
    }

    std::filesystem::create_directories(root);
    {
        std::ofstream(root / "component.version") << "0.9.4\n";
        std::ofstream(root / "LICENSE") << "GNU GPL v3\n";
        std::ofstream(root / "OptiScaler.ini") << "; managed component marker\n";
    }

    {
        isb::optiscaler::Manager manager(root);
        const auto result = manager.detect();
        assert(result.state == isb::optiscaler::InstallState::Installed);
        assert(result.manifest.id == "optiscaler");
        assert(result.manifest.version == "0.9.4");
        assert(result.manifest.license == "GPL-3.0");
    }

    std::filesystem::remove(root / "LICENSE");

    {
        isb::optiscaler::Manager manager(root);
        const auto result = manager.detect();
        assert(result.state == isb::optiscaler::InstallState::Invalid);
    }

    std::filesystem::remove_all(root);
    return 0;
}
