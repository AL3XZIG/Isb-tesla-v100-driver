#pragma once

#include "isb/common/provenance.hpp"

#include <optional>
#include <utility>

namespace isb::providers::nvml {

/// A value reported by a provider. An empty value is an unknown observation.
template <typename T>
struct Observed {
    std::optional<T> value;
    common::SourceKind source = common::SourceKind::Unknown;
    common::Confidence confidence = common::Confidence::Unknown;

    static Observed unknown() noexcept { return {}; }

    static Observed reported(T reported_value) {
        return {std::move(reported_value), common::SourceKind::Nvml, common::Confidence::Reported};
    }
};

} // namespace isb::providers::nvml
