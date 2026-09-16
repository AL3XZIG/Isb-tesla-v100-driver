# Benchmarks — Technical Task

## Goal
Measure driver/API/workaround effects reproducibly rather than relying on subjective impressions.

## Must implement
- Standardized environment capture.
- Warmup and repeated measurements.
- Correctness validation before performance claims.
- p50/p95/p99 latency where applicable.
- FPS/frame-time and 1% low for graphics tests.
- CUDA Events for kernel-level timing where appropriate.
- Separate baseline, candidate and regression reports.

## Rules
- Do not compare different environments without recording the differences.
- Effects below the measured noise floor must not be presented as confirmed improvements.

## Acceptance
- Every benchmark result references exact GPU, driver, application and test revision.
