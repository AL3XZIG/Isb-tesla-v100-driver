#pragma once

#include <cstddef>
#include <string>
#include <vector>

namespace isb::benchmarks {

/// Environment fingerprint captured before any measurement so that results
/// from different machines or driver revisions are never silently compared.
struct EnvironmentSnapshot {
    std::string os;
    std::string cpu_model;
    std::string gpu_model;      ///< Empty when no GPU evidence exists.
    std::string driver_version; ///< Empty when unknown — never "0".
    std::string test_revision;  ///< VCS revision of the benchmark suite.

    std::string to_string() const;
};

EnvironmentSnapshot capture_environment();

/// Statistics over a sample set of durations (in microseconds).
struct LatencyStats {
    std::size_t count = 0;
    double mean_us = 0.0;
    double p50_us = 0.0;
    double p95_us = 0.0;
    double p99_us = 0.0;
    /// Median absolute deviation scaled like sigma (1.4826 * MAD). Used as
    /// the noise floor for improvement claims.
    double noise_floor_us = 0.0;
    bool valid = false; ///< False when there were not enough samples.
};

LatencyStats compute_stats(const std::vector<double>& samples_us);

/// Frame-time summary for graphics tests: FPS derived from mean frame time
/// plus the 1% low convention (mean of the slowest 1% of frames).
struct FpsSummary {
    std::size_t frame_count = 0;
    double fps_mean = 0.0;
    double one_percent_low_fps = 0.0;
    double median_frame_time_ms = 0.0;
    bool valid = false;
};

FpsSummary compute_fps_summary(const std::vector<double>& frame_times_ms);

/// Verdict comparing candidate against baseline. An effect smaller than the
/// measured noise floor is reported as `WithinNoise` and must not be
/// presented as a confirmed improvement.
enum class ComparisonVerdict { WithinNoise, Improvement, Regression, Incomparable };

struct ComparisonResult {
    ComparisonVerdict verdict = ComparisonVerdict::Incomparable;
    double delta_us = 0.0;      ///< candidate p50 - baseline p50
    double relative_delta = 0.0;
    std::string explanation;    ///< Why the verdict was reached.
};

ComparisonResult compare(const LatencyStats& baseline, const LatencyStats& candidate,
                         const EnvironmentSnapshot& baseline_env,
                         const EnvironmentSnapshot& candidate_env);

const char* to_string(ComparisonVerdict verdict) noexcept;

} // namespace isb::benchmarks
