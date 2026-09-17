#include "isb/common/version.hpp"

#include <array>
#include <limits>

namespace isb::common {
namespace {

Result<std::uint32_t> parse_component(std::string_view component) {
    if (component.empty()) {
        return Status::failure(ErrorCode::InvalidArgument, "version component is empty");
    }

    std::uint64_t value = 0;
    for (const char character : component) {
        if (character < '0' || character > '9') {
            return Status::failure(ErrorCode::InvalidArgument,
                                   "version component is not decimal");
        }
        value = value * 10U + static_cast<std::uint64_t>(character - '0');
        if (value > std::numeric_limits<std::uint32_t>::max()) {
            return Status::failure(ErrorCode::InvalidArgument, "version component overflows");
        }
    }
    return static_cast<std::uint32_t>(value);
}

} // namespace

Result<Version> parse_version(std::string_view dotted) {
    std::array<std::string_view, 3> components;
    std::size_t component_index = 0;
    std::size_t begin = 0;
    while (true) {
        const std::size_t end = dotted.find('.', begin);
        if (component_index == components.size()) {
            return Status::failure(ErrorCode::InvalidArgument,
                                   "version must have exactly three dotted components");
        }
        components[component_index++] = dotted.substr(begin, end - begin);
        if (end == std::string_view::npos) break;
        begin = end + 1;
    }
    if (component_index != components.size()) {
        return Status::failure(ErrorCode::InvalidArgument,
                               "version must have exactly three dotted components");
    }

    auto major = parse_component(components[0]);
    if (!major.ok()) return major.status();
    auto minor = parse_component(components[1]);
    if (!minor.ok()) return minor.status();
    auto patch = parse_component(components[2]);
    if (!patch.ok()) return patch.status();
    return Version{major.value(), minor.value(), patch.value()};
}

std::string to_string(const Version& version) {
    return std::to_string(version.major) + "." + std::to_string(version.minor) + "." +
           std::to_string(version.patch);
}

} // namespace isb::common
