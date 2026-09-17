# Games — V100 Compatibility Manager

## Goal
Discover installed games, fingerprint their graphics/runtime environment and manage V100-specific compatibility profiles safely.

## Must implement
- Steam/Epic/GOG/standalone discovery where detectable.
- Executable and graphics API detection where possible.
- DLSS/FSR/XeSS/runtime signal detection where possible.
- Anti-cheat state: `PRESENT`, `ABSENT`, `UNKNOWN`.
- Compatibility state: `AVAILABLE`, `UNAVAILABLE`, `UNKNOWN`.
- Per-game profile schema and import/export.
- Safe OptiScaler integration through `integrations/`.
- File backup/hash/rollback for managed changes.
- Dry-run and explicit approval for mutations.
- Safe bulk optimization only for eligible games.

## Safety
Unknown compatibility or anti-cheat state must not become automatic install permission. Online/anti-cheat games must never be modified silently.

## Acceptance
`SCAN → DETECT → COMPATIBILITY → DRY-RUN → APPROVAL → APPLY → VERIFY → LOG → ROLLBACK` is deterministic and auditable.
