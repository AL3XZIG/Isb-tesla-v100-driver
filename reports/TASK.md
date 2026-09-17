# Reports — Deterministic Support Bundles

## Goal
Provide a compact, reproducible report format for V100 diagnosis, optimization and regression analysis.

## Must implement
- Versioned `manifest.json`.
- GPU, driver, capabilities, games, OptiScaler, performance and error records where available.
- Operation and verification logs.
- Environment hash and provenance/source metadata.
- Baseline vs optimized diff support.
- Explicit synthetic-fixture markers.

## Acceptance
The same input state produces deterministic machine-readable output suitable for issue reports and regression fixtures.
