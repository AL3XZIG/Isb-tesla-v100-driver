# Database — Technical Task

## Goal
Store versioned, auditable knowledge about GPUs, driver stacks, applications, issues and compatibility profiles.

## Must implement
- `drivers/`: base-driver metadata and tested revisions.
- `issues/`: reproducible issue definitions and workaround metadata.
- GPU profiles: hardware facts separated from observations.
- Application profiles: executable/API/version matching.
- Evidence links to probe IDs and test reports.
- Schema validation and deterministic parsing.

## Rules
- Never encode an unverified hardware value as fact.
- Historical driver observations must remain immutable once used by a regression result.
- Unknown is different from unavailable.

## Acceptance
- Schema validation tests exist.
- A driver profile can be compared against an observed fingerprint.
