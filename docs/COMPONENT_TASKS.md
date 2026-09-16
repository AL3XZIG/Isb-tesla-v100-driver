# ISB Component Technical Tasks

This document defines the implementation contract for the Driver Fixer architecture. Each major component also has a local `TASK.md` with its focused scope.

## Global rules

1. Inspect the current branch and existing implementation before changing code.
2. Architecture documents are not implementation. Do not invent hardware behavior to satisfy a design.
3. Hardware, driver, observed runtime, and ISB-added capabilities are separate facts.
4. Every externally observable claim needs provenance and, where applicable, a reproducible probe.
5. Prefer small, testable components and explicit interfaces.
6. No NVIDIA proprietary binaries, headers, SDK redistributions, or copied proprietary code.
7. External projects must remain behind explicit integration boundaries and undergo license/provenance review.
8. V100-specific assumptions must be marked as verified, assumed, or blocked until hardware validation.
9. Fixes must support dry-run, verification, and rollback where the action can change system state.
10. Do not make the GUI a dependency of the CLI/fixer runtime.

## Implementation order

P0: `core`, `capabilities`, `diagnostics`, `providers`, `database`, `fixer`, `cli`.

P1: `drivers`, `compatibility`, `profiles`, `tests`, `benchmarks`.

P2: `integrations`, `compute`, `graphics`, `neural`, `control-center`, `installer`.

Research only: `research/alternative-driver`.

The first end-to-end milestone is:

`scan -> fingerprint -> diagnose -> fix --dry-run -> fix -> verify -> report`

No component is considered complete merely because it compiles; behavior needs tests and provenance.
