# Control Center — Technical Task

## Goal
Build an optional cross-platform GUI that replaces the need for NVIDIA App for ISB workflows while remaining independent of the fixer core.

## Must implement
- Dashboard: GPU, driver, APIs, health and ISB status.
- Driver profiles, diagnostics, fixes, verification and rollback.
- Per-game/application compatibility profiles.
- GPU telemetry and logging.
- Enhancement management: external upscalers, frame-generation paths and image/latency tools where supported.
- Clear distinction between base-driver features and ISB-added features.
- GUI communicates with the same core/CLI APIs; no duplicated business logic.

## Non-goals
- Reimplementing the NVIDIA driver UI wholesale.
- Making GUI mandatory for ISB operation.

## Acceptance
- All critical workflows remain usable from CLI.
- GUI actions produce the same structured audit/provenance records as CLI actions.
