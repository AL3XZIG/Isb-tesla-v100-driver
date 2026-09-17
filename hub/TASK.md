# Hub — Control Plane

## Goal
Provide one lightweight user-facing orchestration layer for the ISB V100 Hub. The Hub coordinates existing CAL, capability, provider, FixEngine, game, performance, integration, benchmark, diagnostics and report modules.

## Must implement
- `scan`, `inspect`, `status`, `optimize`, `profile`, `apply`, `rollback`, `verify`, `benchmark`, `report` contracts.
- Capability aggregation without duplicating CAL or provider logic.
- Explicit operation plans and `requested -> applied -> verified` state.
- Transaction/rollback coordination.
- Shared contracts for CLI and Control Center.
- Deterministic JSON output and audit/provenance records.
- Dry-run before mutating operations.

## Non-goals
- GPU kernel driver implementation.
- Second FixEngine/rule system.
- Duplicate GUI business logic.
- Mandatory background daemon.

## Acceptance
- CLI can execute all critical workflows without the GUI.
- GUI can consume the same contracts.
- Failed reversible operations can be rolled back.
- Unknown capabilities never become implicit support.
