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
    /// Why the value is absent (for example an API error string). Must remain
    /// empty for successfully reported values so that no failure is ever
    /// presented as a successful observation.
    std::string reason;

    static Observed unknown() noexcept { return {}; }

    static Observed reported(T reported_value) {
        static_assert(Source != SourceKind::Unknown,
                      "reported() requires a concrete observation source");
        Observed observed{std::move(reported_value), Source, Confidence::Reported, {}};
        return observed;
    }

    /// A failed query: no value, plus the diagnostic reason. This preserves
    /// UNKNOWN instead of manufacturing a zero/default value.
    static Observed failed(std::string failure_reason) noexcept {
        Observed observed{};
        observed.source = Source;
        observed.reason = std::move(failure_reason);
        return observed;
    }
};

} // namespace isb::common
