#pragma once

#include <string>

namespace isb::performance {

/// Deterministic ISO-8601 UTC timestamp derived from seconds since the epoch.
/// Uses gmtime-independent civil-from-days arithmetic so results are stable
/// across time zones and fully reproducible in verification runs.
std::string iso8601_utc(long long seconds_since_epoch);

/// Current wall-clock time as an ISO-8601 UTC string, or "unknown" when the
/// system clock cannot be trusted. Never fabricates a timestamp.
std::string current_timestamp();

} // namespace isb::performance
