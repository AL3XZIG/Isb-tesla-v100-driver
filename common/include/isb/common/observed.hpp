#pragma once

#include "isb/common/provenance.hpp"

#include <optional>
#include <utility>

namespace isb::common {

template <typename T, SourceKind Source = SourceKind::Unknown>
struct Observed {
    std::optional<T> value;
    SourceKind source = SourceKind::Unknown;
    Confidence confidence = Confidence::Unknown;

    static Observed unknown() noexcept { return {}; }

    static Observed reported(T reported_value) {
        return {std::move(reported_value), Source, Confidence::Reported};
    }
};

} // namespace isb::common
