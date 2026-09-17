#include "isb/common/version.hpp"

#include <cassert>
#include <cstdint>

int main() {
    const auto version = isb::common::parse_version("535.104.05");
    assert(version.ok());
    assert((version.value() == isb::common::Version{535, 104, 5}));
    assert(isb::common::to_string(version.value()) == "535.104.5");

    for (const char* invalid : {"535.104", "535.104.5.1", "535..5", "535.x.5", "-1.0.0"}) {
        const auto result = isb::common::parse_version(invalid);
        assert(!result.ok());
        assert(result.status().code() == isb::common::ErrorCode::InvalidArgument);
    }
}
