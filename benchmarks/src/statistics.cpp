#include "isb/benchmarks/statistics.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace isb::benchmarks {
namespace {

std::string trim(std::string s) {
    const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!s.empty() && is_space(s.front())) s.erase(s.begin());
    while (!s.empty() && is_space(s.back())) s.pop_back();
    return s;
}

std::string first_line_containing(const char* path, const char* needle) {
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line)) {
        if (line.find(needle) != std::string::npos) return trim(line);
    }
    return {};
}

/// Nearest-rank percentile on a pre-sorted ascending sample vector.
double percentile(const std::vector<double>& sorted, double q) {
    if (sorted.empty()) return 0.0;
    const std::size_t rank = static_cast<std::size_t>(
        std::ceil(q * static_cast<double>(sorted.size())));
    const std::size_t index = rank == 0 ? 0 : std::min(rank - 1, sorted.size() - 1);
    return sorted[index];
}

bool same_environment(const EnvironmentSnapshot& a, const EnvironmentSnapshot& b) {
    return a.os == b.os && a.cpu_model == b.cpu_model && a.gpu_model == b.gpu_model &&
           a.driver_version == b.driver_version && a.test_revision == b.test_revision;
}

} // namespace

std::string EnvironmentSnapshot::to_string() const {
    std::ostringstream o;
    o << "os=" << (os.empty() ? "unknown" : os)
      << ";cpu=" << (cpu_model.empty() ? "unknown" : cpu_model)
      << ";gpu=" << (gpu_model.empty() ? "unknown" : gpu_model)
      << ";driver=" << (driver_version.empty() ? "unknown" : driver_version)
      << ";revision=" << (test_revision.empty() ? "unknown" : test_revision);
    return o.str();
}

EnvironmentSnapshot capture_environment() {
    EnvironmentSnapshot env;
#if defined(_WIN32)
    env.os = "Windows";
#elif defined(__linux__)
    env.os = "Linux";
#elif defined(__APPLE__)
    env.os = "macOS";
#endif

    env.cpu_model = first_line_containing("/proc/cpuinfo", "model name");
    if (env.cpu_model.empty()) {
        env.cpu_model = first_line_containing("/proc/cpuinfo", "Model");
    }
    if (!env.cpu_model.empty()) {
        const auto sep = env.cpu_model.find(':');
        if (sep != std::string::npos) env.cpu_model = trim(env.cpu_model.substr(sep + 1));
    }

    // GPU evidence comes from the NVIDIA proc driver record only when
    // readable; absence stays empty and is rendered as "unknown" rather than
    // being guessed.
    env.gpu_model = first_line_containing("/proc/driver/nvidia/version", "NVRM");
    if (!env.gpu_model.empty()) {
        std::istringstream parts(env.gpu_model);
        std::string token;
        while (parts >> token) {
            if (token == "Version:") {
                parts >> token;
                env.driver_version = trim(token);
                break;
            }
        }
        env.gpu_model = "NVIDIA"; // Vendor family only; exact model needs NVML.
    } else {
        env.gpu_model.clear();
    }

    env.test_revision = first_line_containing("/workspace/.git/HEAD", "/");
    if (!env.test_revision.empty()) {
        const auto slash = env.test_revision.rfind('/');
        if (slash != std::string::npos) env.test_revision = env.test_revision.substr(slash + 1);
    }
    return env;
}

LatencyStats compute_stats(const std::vector<double>& samples_us) {
    LatencyStats stats;
    if (samples_us.size() < 3) return stats; // Too few samples to be trustworthy.

    std::vector<double> sorted = samples_us;
    std::sort(sorted.begin(), sorted.end());

    stats.count = sorted.size();
    double sum = 0.0;
    for (const double v : sorted) sum += v;
    stats.mean_us = sum / static_cast<double>(sorted.size());
    stats.p50_us = percentile(sorted, 0.50);
    stats.p95_us = percentile(sorted, 0.95);
    stats.p99_us = percentile(sorted, 0.99);

    std::vector<double> deviations;
    deviations.reserve(sorted.size());
    for (const double v : sorted) deviations.push_back(std::fabs(v - stats.p50_us));
    std::sort(deviations.begin(), deviations.end());
    stats.noise_floor_us = 1.4826 * percentile(deviations, 0.50);
    stats.valid = true;
    return stats;
}

FpsSummary compute_fps_summary(const std::vector<double>& frame_times_ms) {
    FpsSummary summary;
    if (frame_times_ms.size() < 10) return summary;

    std::vector<double> sorted = frame_times_ms;
    std::sort(sorted.begin(), sorted.end());

    double sum = 0.0;
    for (const double v : sorted) sum += v;
    const double mean_ms = sum / static_cast<double>(sorted.size());
    if (mean_ms <= 0.0) return summary;

    summary.frame_count = sorted.size();
    summary.fps_mean = 1000.0 / mean_ms;
    summary.median_frame_time_ms = percentile(sorted, 0.50);

    // 1% low: mean of the slowest 1% frames (largest frame times).
    const std::size_t slowest_count =
        std::max<std::size_t>(1, sorted.size() / 100);
    double slow_sum = 0.0;
    for (std::size_t i = sorted.size() - slowest_count; i < sorted.size(); ++i) {
        slow_sum += sorted[i];
    }
    summary.one_percent_low_fps = 1000.0 / (slow_sum / static_cast<double>(slowest_count));
    summary.valid = true;
    return summary;
}

ComparisonResult compare(const LatencyStats& baseline, const LatencyStats& candidate,
                         const EnvironmentSnapshot& baseline_env,
                         const EnvironmentSnapshot& candidate_env) {
    ComparisonResult result;
    if (!baseline.valid || !candidate.valid) {
        result.verdict = ComparisonVerdict::Incomparable;
        result.explanation = "insufficient samples in baseline or candidate run";
        return result;
    }
    if (!same_environment(baseline_env, candidate_env)) {
        result.verdict = ComparisonVerdict::Incomparable;
        result.explanation = "environment differs: baseline[" + baseline_env.to_string() +
                             "] vs candidate[" + candidate_env.to_string() + "]";
        return result;
    }

    result.delta_us = candidate.p50_us - baseline.p50_us;
    result.relative_delta = baseline.p50_us > 0.0
                                ? result.delta_us / baseline.p50_us
                                : 0.0;

    const double noise =
        std::max(baseline.noise_floor_us, candidate.noise_floor_us);
    if (std::fabs(result.delta_us) <= noise) {
        result.verdict = ComparisonVerdict::WithinNoise;
        char buffer[160];
        std::snprintf(buffer, sizeof(buffer),
                      "|delta|=%.3fus <= noise floor %.3fus; effect not confirmed",
                      std::fabs(result.delta_us), noise);
        result.explanation = buffer;
    } else if (result.delta_us < 0.0) {
        result.verdict = ComparisonVerdict::Improvement;
        result.explanation = "candidate p50 below baseline beyond the noise floor";
    } else {
        result.verdict = ComparisonVerdict::Regression;
        result.explanation = "candidate p50 above baseline beyond the noise floor";
    }
    return result;
}

const char* to_string(ComparisonVerdict verdict) noexcept {
    switch (verdict) {
    case ComparisonVerdict::WithinNoise: return "within_noise";
    case ComparisonVerdict::Improvement: return "improvement";
    case ComparisonVerdict::Regression: return "regression";
    case ComparisonVerdict::Incomparable: break;
    }
    return "incomparable";
}

} // namespace isb::benchmarks
