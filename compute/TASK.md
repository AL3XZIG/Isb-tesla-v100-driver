# Compute — Technical Task

## Goal
Provide compute/API diagnostics and higher-level orchestration without replacing the CUDA driver/runtime.

## Must implement
- CUDA runtime/driver detection.
- Compute capability observation and provenance.
- DirectCompute detection on Windows.
- Compute workload compatibility tests.
- Tensor Core capability reporting based on hardware + observed runtime support.
- Provider-neutral compute status.

## Rules
- CUDA Toolkit remains an external dependency.
- Do not infer runtime support from hardware capability alone.
- Do not redistribute NVIDIA proprietary runtime components.

## Acceptance
- CUDA and DirectCompute states can independently be AVAILABLE, UNAVAILABLE or UNKNOWN.
