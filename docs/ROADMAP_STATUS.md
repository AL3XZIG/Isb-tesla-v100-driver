# Architecture Refactoring Roadmap Status

## Goal

Consolidate ISB architecture before adding more hardware functionality.

## Current phase

Phase 0 — Baseline and ownership audit

## Planned sequence

### PR #29 — Foundation ownership audit

- Verify ownership of Result, Status, ErrorCode primitives.
- Do not migrate common/core without source evidence.
- Fix runtime safety issues only after ownership is confirmed.

### PR #30 — Observation contract consolidation

- Audit observation wrappers and capability facts.
- Remove duplicate concepts only after confirming real usage.

### PR #31 — Hub canonical API

- Make Hub the single user control entry point.
- Remove duplicate control-center control-plane logic.
- Panel should use Hub API only.

### PR #32 — Capability model consolidation

- CAL owns capability semantics.
- Unknown must remain different from Unsupported.

### PR #33 — Experiments/providers audit

- Decide migration, retention, or removal of experimental modules.
- Prevent duplicate provider implementations.

### PR #34-#37

- Stabilization, build validation, reporting and hardware qualification.

## Documentation rule

Architecture documentation must describe verified implementation or explicitly marked target state.
