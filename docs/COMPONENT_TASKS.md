# ISB V100 Hub — Component Task Map

This document is the short routing map for agents. The detailed implementation checklist is [`docs/TASKS.md`](TASKS.md); architecture contracts are in [`docs/ARCHITECTURE.md`](ARCHITECTURE.md).

## Global rules

1. Inspect the current branch and existing implementation before changing code.
2. Architecture documents are contracts, not evidence of implemented hardware behavior.
3. Hardware, base-driver/runtime and ISB-added capabilities are separate facts.
4. Every externally observable claim needs provenance and, where applicable, a reproducible probe.
5. Prefer small, testable components and explicit interfaces.
6. Do not redistribute NVIDIA proprietary binaries, headers, SDKs or copied proprietary code.
7. External projects remain behind explicit integration boundaries with license/provenance review.
8. V100-specific assumptions must be marked verified, assumed or blocked until hardware validation.
9. Mutations require dry-run/plan, explicit apply, verification and rollback where technically possible.
10. GUI and CLI use the same Hub contracts; GUI must not duplicate business logic.
11. `UNKNOWN` is never equivalent to `AVAILABLE`.
12. FixEngine remains the single authoritative workaround/rule engine.

## New implementation order

### P0 — Shared foundation

`core` → `cal` → `capabilities` → `providers` → `diagnostics` → `verification`

Goal: one coherent environment/capability/evidence model and buildable provider boundaries.

### P1 — User-facing control plane

`hub` → `cli` → `performance` → `compatibility` → `games` → `profiles`

Goal: make the V100 observable, diagnosable and controllable from a headless interface before GUI work.

### P1 — Compatibility integrations

`fixer` / existing FixEngine → `integrations` → `graphics`

Goal: reuse the existing FixEngine and provide safe per-application enhancement paths, including external OptiScaler management.

> `fixer/` is the current repository ownership name for the existing FixEngine area. Do not create a parallel `fix/` implementation unless a deliberate migration is performed and all callers/tests are moved together.

### P2 — Measurements and presentation

`benchmarks` → `reports` → `control-center` → `installer`

Goal: reproducible before/after measurements, deterministic reports and a lightweight GUI on top of stable Hub contracts.

### P2/P3 — V100 feature expansion

`compute` → `neural` → graphics experiments → Thermal Guard / Interconnect expansion

Goal: Tensor Core/AI features, software reconstruction and other V100-specific experiments with explicit capability and performance validation.

### Research only

`research/alternative-driver`

The independent KMD/UMD/HAL track is isolated and must never become a runtime dependency of the Hub.

## First end-to-end Hub milestone

```text
scan
  -> fingerprint
  -> capability aggregation
  -> diagnose
  -> optimize --dry-run
  -> explicit apply
  -> verify
  -> report
```

For games/OptiScaler the equivalent flow is:

```text
SCAN -> DETECT -> COMPATIBILITY -> DRY-RUN -> APPROVAL
     -> APPLY/INSTALL -> VERIFY -> LOG -> ROLLBACK
```

No component is complete merely because it compiles. Stable functionality requires tests, provenance, negative/unknown-path handling and verification.

---

## Current implementation-gap routing

The following routing reflects the September 2026 source/CI audit.

### Provider completion
- NVML: real read-only path exists; management/mutation remains incomplete.
- CUDA: provider boundary exists; complete observation/capability aggregation remains open.
- Vulkan: provider boundary exists; complete observation/capability aggregation remains open.
- Linux DRM/Vulkan/OpenGL/PRIME: implementation remains a required roadmap item.
- Linux platform/runtime detection: implementation remains a required roadmap item.

### Hub
- Keep Hub as the sole orchestration boundary.
- Replace conservative/mock paths with real provider-backed operations only after read-back verification exists.
- Complete scan, diagnose, optimize, apply, rollback, verify and report transactions.

### Performance
- Complete real telemetry.
- Add bounded history and throttling analysis.
- Add provider-backed tuning with rollback and verification.

### FixEngine / diagnostics
- Finish deterministic rule matching.
- Integrate real fingerprint/IDR evidence.
- Implement the first reproducible V100 workaround and regression case.

### Graphics
- Complete render-path detection.
- Implement safe application-scoped routing/configuration.
- Add end-to-end verification.
- Keep software enhancement distinct from native V100 hardware capability.

### Games / OptiScaler
- Implement game discovery and compatibility providers.
- Implement safe OptiScaler lifecycle management with backup/hash/rollback.

### Drivers
- Implement NVIDIA/Google release providers.
- Implement URL parsing, compatibility resolution, downloader, verification and cache.
- Keep installation explicit and separate from ISB installation.

### Installer / Release
- Complete production ISB packaging and upgrade/uninstall.
- Complete Linux release artifacts, checksums, manifests and GitHub Release automation.

### Control Center
- Convert mock/contract UI into real Hub-backed views incrementally.
- Never move provider/business logic into Qt.

### Evidence / benchmarks
- Complete deterministic reports and baseline/candidate comparison.
- Implement reproducible CUDA/Tensor/HBM/PCIe/NVLink/graphics benchmarks.
- Require correctness and provenance before performance claims.
\n