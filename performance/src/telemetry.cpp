#include "isb/performance/telemetry.hpp"

#include <chrono>
#include <cstdio>

namespace isb::performance {
namespace {

// Howard Hinnant's civil_from_days algorithm (public domain). Converts days
// since 1970-01-01 to a proleptic Gregorian y/m/d without any libc locale or
// time-zone dependencies.
void civil_from_days(long long z, int& year, unsigned& month, unsigned& day) noexcept {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    const long long y = static_cast<long long>(yoe) + era * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    const unsigned d = doy - (153 * mp + 2) / 5 + 1;
    const unsigned m = mp < 10 ? mp + 3 : mp - 9;
    year = static_cast<int>(y + (m <= 2 ? 1 : 0));
    month = m;
    day = d;
}

} // namespace

std::string iso8601_utc(long long seconds_since_epoch) {
    long long days = seconds_since_epoch / 86400;
    long long secs = seconds_since_epoch % 86400;
    if (secs < 0) {
        secs += 86400;
        --days;
    }
    int year = 0;
    unsigned month = 0;
    unsigned day = 0;
    civil_from_days(days, year, month, day);
    const unsigned hour = static_cast<unsigned>(secs / 3600);
    const unsigned minute = static_cast<unsigned>((secs % 3600) / 60);
    const unsigned second = static_cast<unsigned>(secs % 60);

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%04d-%02u-%02uT%02u:%02u:%02uZ",
                  year, month, day, hour, minute, second);
    return std::string(buffer);
}

std::string current_timestamp() {
    const auto now = std::chrono::system_clock::now();
    if (now.time_since_epoch().count() < 0) {
        // A clock before the POSIX epoch is not trustworthy evidence; report
        // it explicitly instead of emitting a misleading timestamp.
        return "unknown";
    }
    return iso8601_utc(std::chrono::duration_cast<std::chrono::seconds>(
                           now.time_since_epoch())
                           .count());
}

} // namespace isb::performance
