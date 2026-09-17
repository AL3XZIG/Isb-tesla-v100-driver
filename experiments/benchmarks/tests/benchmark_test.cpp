#include "isb/benchmarks/benchmark.hpp"
#include "isb/benchmarks/report.hpp"

#include <cassert>
#include <memory>
#include <string>
#include <vector>

using namespace isb::benchmarks;

int main() {
    std::vector<BenchmarkResult> results;
    std::vector<std::unique_ptr<IBenchmark>> checks;
    checks.push_back(make_fp32_check());
    checks.push_back(make_fp16_check());
    checks.push_back(make_tensor_capability_check());
    for (auto& check : checks) {
        check->initialize();
        check->run();
        const BenchmarkResult& result = check->result();
        assert(!result.name.empty());
        assert(result.backend == "CUDA");
        assert(result.available != VerificationState::Failed);
        results.push_back(result);
        check->shutdown();
    }

    const CudaCheck no_backend{VerificationState::Unknown, {}, 0, 0, 0, "CUDA backend was not built"};
    const std::string report = make_report(no_backend, results);
    assert(report.find("CUDA:\nUNKNOWN") != std::string::npos);
    assert(report.find("FP32:\n") != std::string::npos);
    assert(report.find("FP16:\n") != std::string::npos);
    assert(report.find("Tensor:\n") != std::string::npos);
}
