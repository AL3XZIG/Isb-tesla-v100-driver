#pragma once

#include <chrono>
#include <map>
#include <string>

namespace isb::benchmarks {

/// Outcome of a non-destructive GPU capability verification.
enum class VerificationState {
    Unknown,
    Verified,
    Unavailable,
    Failed,
};

/// A compact, serializable-by-caller result for one capability check.
struct BenchmarkResult {
    std::string name;
    std::string backend;
    VerificationState available = VerificationState::Unknown;
    std::chrono::milliseconds duration_ms{0};
    std::string error;
    std::map<std::string, std::string> metadata;
};

const char* to_string(VerificationState state) noexcept;

} // namespace isb::benchmarks
