# Architecture Refactoring Roadmap Status

## Goal

Consolidate ISB architecture before adding more hardware functionality, then complete the real provider-backed V100 product in evidence-gated milestones.

## Current phase

**Phase 1 — Real provider/capability completion and end-to-end Hub integration**

Foundation ownership, canonical Hub direction, capability ownership and deterministic status work are established. The repository is now past the original architecture-refactoring milestone, but most hardware-facing product features remain incomplete or only partially implemented.

## Verified implementation boundary

Source and CI remain authoritative. Current repository evidence shows:

- CAL capability contracts and V100/GV100 capability data exist.
- Hub is the canonical control plane.
- NVML has a real read-only provider path.
- CUDA and Vulkan provider boundaries exist, but full multi-provider capability aggregation is incomplete.
- Control Center exists as a Qt6 frontend/foundation, but real provider-backed telemetry and mutation are not complete.
- Hub optimization/profile/apply/verify paths are intentionally conservative; current apply/verify behavior must not be presented as real hardware tuning.
- Render-path detection/configuration is only partially complete; persistent host configuration and end-to-end application routing remain open.
- OptiScaler integration is an external-component boundary, not a complete deployment manager.
- Driver lifecycle and release-builder architecture are documented, but discovery/download/cache/install and release publication are not yet proven production-complete.
- FixEngine integration and the first reproducible real V100 workaround are still open.
- Real physical V100 qualification is still a separate milestone.

## Remaining roadmap

### P0 — Close stabilization gate

- [ ] Clean configure/build/test on a fresh checkout.
- [ ] Audit optional provider/test guards.
- [ ] Add CI architecture/dependency guard where practical.
- [ ] Remove remaining duplicate/legacy control contracts.
- [ ] Keep documentation synchronized with source/CI.

### P1 — Real read-only V100 stack

- [ ] Exact V100 variant detection: SXM2 / PCIe / 16 GB / 32 GB.
- [ ] Complete CUDA provider observations.
- [ ] Complete Vulkan provider observations.
- [ ] Complete Linux environment/driver/platform observation path.
- [ ] Complete Linux DRM/PRIME/Vulkan/OpenGL observations.
- [ ] Aggregate NVML + CUDA + Vulkan + Linux observations in Hub.
- [ ] Preserve provenance and UNKNOWN/PERMISSION_DENIED/ERROR semantics.
- [ ] Build a deterministic capability reconciliation engine.

### P1 — Telemetry and evidence

- [ ] Complete real telemetry snapshot.
- [ ] Add bounded telemetry history.
- [ ] Add throttling/thermal analysis.
- [ ] Add ECC/NVLink/PCIe/process observations where exposed.
- [ ] Produce deterministic real-vs-synthetic evidence reports.
- [ ] Add baseline/candidate report diff.

### P1 — FixEngine / Driver Doctor

- [ ] Finish deterministic FixEngine matcher.
- [ ] Integrate fingerprint + IDR evidence into diagnosis.
- [ ] Implement tri-state rule evaluation.
- [ ] Add compatibility database records.
- [ ] Select the first real reproducible V100 driver/API problem.
- [ ] Implement one reversible user-space/configuration workaround.
- [ ] Verify before/after and rollback.
- [ ] Convert the case into a regression fixture.

### P1 — Tuning / Optimize V100

- [ ] Implement provider-backed control descriptors.
- [ ] Implement read -> validate -> plan -> approval -> apply -> read-back -> verify.
- [ ] Implement rollback for reversible controls.
- [ ] Add real persistence/power/clocks/compute controls only where provider support is verified.
- [ ] Complete profile engine.
- [ ] Complete Optimize V100 planner.
- [ ] Never report a mutation as successful without read-back verification.

### P2 — Graphics and render path

- [ ] Complete multi-GPU role reconciliation.
- [ ] Complete Linux DRM/PRIME/Vulkan render-path evidence.
- [ ] Complete Linux multi-GPU role reconciliation.
- [ ] Implement persistent application-scoped Linux configuration where safe.
- [ ] Add rollback for render-path configuration.
- [ ] Add end-to-end application render verification.
- [ ] Keep TCC/WDDM changes explicit and user-controlled.

### P2 — Game Manager

- [ ] Steam/Epic/GOG/standalone discovery providers.
- [ ] Executable/API detection.
- [ ] Per-game profile storage.
- [ ] Anti-cheat detection with PRESENT/ABSENT/UNKNOWN.
- [ ] Compatibility database integration.
- [ ] Safe dry-run/apply/verify/rollback lifecycle.
- [ ] No silent online/anti-cheat modification.

### P2 — OptiScaler Manager

- [ ] Installed version/provenance detection.
- [ ] Compatible-version selection.
- [ ] Official/source artifact discovery.
- [ ] Download + checksum/hash verification.
- [ ] Backup/install/update/remove/rollback.
- [ ] Per-game configuration.
- [ ] License/SPDX/provenance tracking.
- [ ] Verify the resulting game state after every mutation.

### P2 — Graphics Enhancement Layer

- [ ] Backend-neutral enhancement contracts.
- [ ] Spatial scaling/Lanczos.
- [ ] CAS-style sharpening.
- [ ] Dynamic resolution/DRS where application configuration permits.
- [ ] Frame-pacing telemetry.
- [ ] External upscaler integration boundary.
- [ ] Experimental neural reconstruction.
- [ ] Experimental frame interpolation.
- [ ] Keep all software reconstruction distinct from native DLSS/RT/Optical Flow hardware.

### P2 — Driver Manager

- [ ] NVIDIA Linux/Data Center release provider.
- [ ] Provider/branch/version/OS/package normalization.
- [ ] URL/release-page parser.
- [ ] V100 compatibility resolver.
- [ ] Official-source metadata/provenance.
- [ ] Safe downloader.
- [ ] SHA-256/signature verification.
- [ ] Local cache with content identity.
- [ ] CLI integration.
- [ ] Control Center Drivers page.
- [ ] Explicit installation backend only after technical/legal validation.
- [ ] Never silently replace the active base driver.

Current external driver support must be represented as provider/version/Linux-package-specific evidence, not as one universal version. The Linux driver path must be validated against the exact V100 variant and required runtime/API capabilities before being marked compatible.

### P2 — Release Builder

- [ ] Reproducible release build script.
- [ ] Linux x86_64 .deb.
- [ ] Linux .tar.gz.
- [ ] SHA256SUMS.
- [ ] Release manifest.
- [ ] Version/tag consistency checks.
- [ ] GitHub Actions build matrix.
- [ ] Test gate before publication.
- [ ] GitHub Release publication.
- [ ] Artifact provenance and integrity verification.
- [ ] Ensure proprietary NVIDIA/Google packages are never bundled accidentally.

### P2 — Installer / Deployment

- [ ] Production ISB installer packaging.
- [ ] Preflight checks.
- [ ] User-local/system installation modes.
- [ ] Configuration initialization.
- [ ] Shortcuts/service/autostart only where explicitly selected.
- [ ] Upgrade path.
- [ ] Uninstall path.
- [ ] Rollback after failed installation/configuration.
- [ ] Linux packaging integration.
- [ ] Driver installation remains a separate explicit workflow.

### P2 — Reports / Diagnostics

- [ ] Versioned report schema.
- [ ] Full manifest.
- [ ] GPU/driver/capabilities/performance/errors/game/OptiScaler sections.
- [ ] Operation history.
- [ ] Provenance graph.
- [ ] Baseline/candidate comparison.
- [ ] Real/synthetic evidence classification.
- [ ] Export from CLI and Control Center.

### P3 — Control Center completion

- [ ] Real Home/status page.
- [ ] Real telemetry page.
- [ ] Real Tuning page.
- [ ] Optimize V100 workflow.
- [ ] Games page.
- [ ] Graphics page.
- [ ] Drivers page.
- [ ] Tools/diagnostics/reports page.
- [ ] Consistent state presentation.
- [ ] No direct provider access from GUI.
- [ ] Every mutating UI action uses Hub transaction semantics.

### P3 — Benchmark Suite

- [ ] CUDA compute benchmark.
- [ ] Tensor Core benchmark.
- [ ] HBM bandwidth benchmark.
- [ ] PCIe benchmark.
- [ ] NVLink benchmark where present.
- [ ] Graphics/frame-time benchmark.
- [ ] Application before/after benchmark.
- [ ] Correctness gate before performance claim.
- [ ] Reproducible metadata for every result.

### P4 — Experimental V100 graphics/AI

- [ ] Neural super-resolution/reconstruction.
- [ ] Frame interpolation.
- [ ] CUDA software ray tracing/ray marching.
- [ ] SSR/SSAO/SSGI.
- [ ] Voxel lighting.
- [ ] Additional graphics compatibility layers.
- [ ] Keep experimental code isolated from stable Hub dependencies.

### Research-only

- [ ] Alternative KMD/UMD/HAL/GPUVM research under research/alternative-driver/.
- [ ] Reverse-engineering research with provenance/legal boundaries.
- [ ] Never make alternative-driver research a Hub runtime dependency.

## Current blockers

1. Physical V100 hardware is required to close hardware qualification.
3. Mutation must remain disabled until providers can apply and read back real state.
4. External component deployment requires provenance/licensing and artifact verification.
5. Release publication requires reproducible packaging and CI gates.

## Milestone order

M0 stabilization gate
-> M1 Linux real read-only V100 observation
-> M2 capability reconciliation + telemetry
-> M3 FixEngine + first real workaround
-> M4 real tuning + Optimize V100
-> M5 Linux render path + graphics compatibility
-> M6 Game/OptiScaler manager
-> M7 Linux Driver Manager
-> M8 Linux Release Builder + production installer
-> M9 completed Control Center
-> M10 benchmark/qualification release
-> M11 experimental neural/graphics features
-> Research: alternative driver

## Documentation rule

Architecture documentation describes intended behavior. CURRENT_IMPLEMENTATION.md, source code and CI determine implementation status. Any feature promoted from planned/experimental to stable must update the implementation boundary and add evidence/tests.
