#include "isb/benchmarks/report.hpp"

#include <sstream>

namespace isb::benchmarks {

std::string make_report(const CudaCheck& cuda, const std::vector<BenchmarkResult>& results) {
    std::ostringstream report;
    report << "GPU:\n" << (cuda.device_name.empty() ? "UNKNOWN" : cuda.device_name) << "\n\n";
    report << "CUDA:\n" << to_string(cuda.state) << "\n\n";
    report << "SM:\n";
    if (cuda.state == VerificationState::Verified) {
        report << cuda.compute_capability_major << '.' << cuda.compute_capability_minor;
    } else {
        report << "UNKNOWN";
    }
    report << "\n\n";
    for (const auto& result : results) report << result.name << ":\n" << to_string(result.available) << "\n\n";
    return report.str();
}

} // namespace isb::benchmarks
