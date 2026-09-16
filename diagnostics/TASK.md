# Diagnostics — Technical Task

## Goal
Collect reproducible environment, API and failure evidence and emit the existing IDR format.

## Must implement
- OS, kernel/build, GPU, driver and application fingerprinting.
- API probes and health checks.
- IDR generation with provenance and timestamps.
- Evidence bundle for diagnosis and regression tests.
- Human-readable report plus machine-readable output.
- No guessed values when a probe is unavailable.

## Acceptance
- `isb scan` can produce a complete report with UNKNOWN fields when tools are missing.
- Re-running the same probe produces comparable structured output.
- Sensitive paths/identifiers are redacted where appropriate.
