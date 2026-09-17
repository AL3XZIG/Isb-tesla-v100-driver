#pragma once

#include "isb/benchmarks/result.hpp"

#include <memory>

namespace isb::benchmarks {

/// Lifecycle for a minimal, non-destructive GPU capability verification.
class IBenchmark {
public:
    virtual ~IBenchmark() = default;

    virtual void initialize() = 0;
    virtual void run() = 0;
    virtual const BenchmarkResult& result() const = 0;
    virtual void shutdown() = 0;
};

/// Creates checks for FP32, FP16, and Tensor Core capability respectively.
std::unique_ptr<IBenchmark> make_fp32_check();
std::unique_ptr<IBenchmark> make_fp16_check();
std::unique_ptr<IBenchmark> make_tensor_capability_check();

} // namespace isb::benchmarks
