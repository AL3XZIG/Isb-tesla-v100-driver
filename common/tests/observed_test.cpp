#include "isb/common/observed.hpp"

#include <cassert>
#include <memory>
#include <string>

int main() {
    using Unknown = isb::common::Observed<std::string>;
    using Nvml = isb::common::Observed<std::string, isb::common::SourceKind::Nvml>;
    using Cuda = isb::common::Observed<int, isb::common::SourceKind::Cuda>;

    const auto unknown = Unknown::unknown();
    assert(!unknown.value.has_value());
    assert(unknown.source == isb::common::SourceKind::Unknown);
    assert(unknown.confidence == isb::common::Confidence::Unknown);

    const auto reported = Nvml::reported("Tesla V100");
    assert(reported.value.has_value());
    assert(*reported.value == "Tesla V100");
    assert(reported.source == isb::common::SourceKind::Nvml);
    assert(reported.confidence == isb::common::Confidence::Reported);

    const auto cuda = Cuda::reported(70);
    assert(cuda.value.has_value());
    assert(*cuda.value == 70);
    assert(cuda.source == isb::common::SourceKind::Cuda);

    auto move_only = isb::common::Observed<std::unique_ptr<int>, isb::common::SourceKind::Vulkan>::reported(
        std::make_unique<int>(42));
    assert(move_only.value.has_value());
    const int deref_value = *(*move_only.value);
    assert(deref_value == 42);
    assert(move_only.source == isb::common::SourceKind::Vulkan);
}
