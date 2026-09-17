#pragma once

#include "isb/common/error.hpp"

#include <cstdint>
#include <string>
#include <string_view>

namespace isb::common {

struct Version {
    std::uint32_t major = 0;
    std::uint32_t minor = 0;
    std::uint32_t patch = 0;

    friend bool operator==(const Version& lhs, const Version& rhs) noexcept {
        return lhs.major == rhs.major && lhs.minor == rhs.minor && lhs.patch == rhs.patch;
    }
    friend bool operator!=(const Version& lhs, const Version& rhs) noexcept { return !(lhs == rhs); }
};

Result<Version> parse_version(std::string_view dotted);
std::string to_string(const Version& version);

} // namespace isb::common
