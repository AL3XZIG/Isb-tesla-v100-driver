#pragma once

#include "isb/common/provenance.hpp"

#include <optional>
#include <utility>

namespace isb::providers::cuda {

template <typename T>
struct Observed {
    std::optional<T> value;
    common::SourceKind source = common::SourceKind::Unknown;
    common::Confidence confidence = common::Confidence::Unknown;

    static Observed unknown() noexcept { return {}; }

    static Observed reported(T reported_value) {
        return {std::move(reported_value), common::SourceKind::Cuda, common::Confidence::Reported};
    }
};

} // namespace isb::providers::cuda
