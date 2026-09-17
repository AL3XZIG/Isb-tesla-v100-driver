# ISB V100 Hub — Implementation Tasks

This roadmap defines the implementation order for the V100-focused user-space control center. It supersedes the old assumption that the mainline project must become a complete NVIDIA driver.

## P0 — Foundation and integration

- [ ] Reconcile the existing CAL, capability, FixEngine, verification and fingerprinting implementations into one buildable control-plane model.
- [ ] Define stable `Hub` / control-plane contracts shared by CLI and GUI.
- [ ] Define common `Environment`, `GpuIdentity`, `CapabilitySnapshot`, `ErrorEvent`, `Operation`, `VerificationResult` and report contracts without duplicating existing models.
- [ ] Keep capability state tri-state: `AVAILABLE`, `UNAVAILABLE`, `UNKNOWN`.
- [ ] Make all mutations explicit, logged, reversible where possible, and represented as `requested -> applied -> verified`.
- [ ] Define versioned deterministic report-bundle schema.
- [ ] Keep synthetic fixtures explicitly marked `synthetic: true`.

## P0 — V100 capability and provider layer

- [ ] Detect exact V100 variant: SXM2 / PCIe / 16 GB / 32 GB where possible.
- [ ] Separate hardware capability from base-driver/API capability and ISB-added capability.
- [ ] Implement read-only providers for NVIDIA/NVML, CUDA and relevant graphics APIs.
- [ ] Add Windows DXGI/D3D capability probing without assuming DirectCompute availability.
- [ ] Add Vulkan capability probing and provenance.
- [ ] Record driver/package/version/API provenance for every observed capability.
- [ ] Add capability-aware feature gating for Tensor Cores, ECC, RT Cores, Optical Flow, MIG, NVLink and display/output assumptions.

## P1 — Home / V100 status

- [ ] Implement a headless `status` operation first.
- [ ] Expose GPU identity, driver, CUDA/NVML/API state, temperature, utilization, power, clocks and ECC where available.
- [ ] Show active ISB profile and known problems.
- [ ] Generate a one-click-equivalent deterministic report from the CLI.
- [ ] Add `Optimize V100` plan generation without mutation.

## P1 — Performance control

- [ ] Implement read-only telemetry first.
- [ ] Detect supported management controls before exposing them.
- [ ] Add persistence mode where supported.
- [ ] Add application clocks where supported.
- [ ] Add power-limit controls where supported and permitted.
- [ ] Add compute-mode controls where supported.
- [ ] Detect auto-boost / application-clock semantics rather than assuming a fixed clock model.
- [ ] Implement profiles: Balanced, Gaming, Compute, AI/Tensor, Maximum Performance, Low Power, Custom.
- [ ] Store whether each setting is current, persistent, temporary or requires reset/restart.
- [ ] Verify every applied setting through the underlying provider/API.

## P1 — Diagnostics / Driver Doctor

- [ ] Integrate existing fingerprinting and IDR evidence.
- [ ] Integrate the existing FixEngine; do not create a second rule engine.
- [ ] Add diagnosis -> candidate fixes -> dry-run -> explicit apply -> verify -> rollback flow.
- [ ] Add stable reason/error codes.
- [ ] Preserve distinction between `False` and `Unknown` during diagnostics.
- [ ] Add driver/API/application compatibility records with maturity states.
- [ ] Add regression fixtures for known V100 combinations.

## P1 — Game Manager

- [ ] Discover Steam/Epic/GOG/standalone installations where detectable.
- [ ] Identify executable and graphics API where possible.
- [ ] Detect DX11/DX12/Vulkan and relevant upscaler/runtime signals.
- [ ] Define per-game profile schema.
- [ ] Add compatibility state: `AVAILABLE`, `UNAVAILABLE`, `UNKNOWN`.
- [ ] Add anti-cheat state: `PRESENT`, `ABSENT`, `UNKNOWN`.
- [ ] Unknown anti-cheat/compatibility defaults to dry-run/manual approval.
- [ ] Never silently modify online/anti-cheat games.
- [ ] Add safe bulk optimization only for explicitly eligible games.

## P1 — OptiScaler Manager

- [ ] Treat OptiScaler as an external managed component.
- [ ] Detect installed version and provenance.
- [ ] Determine compatible versions per game/API/V100 environment.
- [ ] Implement explicit install/update/remove/rollback operations.
- [ ] Back up target files before mutation.
- [ ] Verify hashes and restore on failed/partial operations.
- [ ] Keep configuration per game rather than globally injecting every executable.
- [ ] Add license/SPDX/provenance metadata.
- [ ] Do not silently download arbitrary binaries.
- [ ] Preserve upstream OptiScaler source/license obligations if redistribution is ever enabled.

## P1 — Graphics enhancement layer

- [ ] Define backend-independent graphics enhancement interfaces.
- [ ] Implement a safe spatial upscaling path (FSR1-style / equivalent) where technically appropriate.
- [ ] Implement Lanczos scaling.
- [ ] Implement CAS-style sharpening.
- [ ] Add resolution scaling, DRS and image-quality controls where an application exposes a safe configuration path.
- [ ] Add frame-pacing telemetry and safe configuration hooks.
- [ ] Keep Vulkan/DX11/DX12 implementations behind backend boundaries.
- [ ] Prefer documented loader/layer/plugin mechanisms over binary patching.
- [ ] Do not claim native DLSS, RT hardware or Optical Flow support on V100.

## P2 — Tensor Core / AI lab

- [ ] Add FP16 and INT8 benchmark/correctness probes where supported.
- [ ] Add Tensor Core throughput benchmark.
- [ ] Add HBM bandwidth benchmark.
- [ ] Add CUDA compute benchmark.
- [ ] Evaluate optional user-space neural super-resolution/reconstruction using CUDA/Tensor Cores.
- [ ] Evaluate small ONNX/TensorRT-style models as optional external/runtime integrations.
- [ ] Benchmark latency and image quality; do not equate AI upscaling with DLSS.
- [ ] Investigate frame interpolation as a separate experimental feature with explicit latency/quality warnings.

## P2 — Thermal Guard

- [ ] Add temperature history.
- [ ] Detect thermal and power throttling.
- [ ] Detect sustained clock drops.
- [ ] Track ECC anomalies.
- [ ] Correlate utilization, power, temperature and clocks.
- [ ] Add user-configurable warning thresholds without silently changing hardware limits.

## P2 — Interconnect

- [ ] Detect PCIe generation and width.
- [ ] Record PCIe traffic/errors where exposed.
- [ ] Detect NVLink topology/state where present.
- [ ] Test CUDA P2P capability.
- [ ] Add measured PCIe/NVLink bandwidth tests.
- [ ] Keep topology/state/capability/measured performance as separate fields.

## P2 — Benchmark suite

- [ ] Create reproducible graphics benchmark.
- [ ] Create CUDA compute benchmark.
- [ ] Create Tensor Core benchmark.
- [ ] Create HBM bandwidth benchmark.
- [ ] Create PCIe/interconnect benchmark.
- [ ] Add application-level before/after comparison.
- [ ] Include GPU/driver/configuration/test revision in every result.
- [ ] Add correctness checks before reporting performance improvements.

## P2 — Reports

- [ ] Implement deterministic report bundle:
  - `manifest.json`
  - `gpu.json`
  - `driver.json`
  - `capabilities.json`
  - `games.json`
  - `optiscaler.json`
  - `performance.json`
  - `errors.json`
  - `logs/`
- [ ] Include schema version, ISB version, generation time, environment hash and source/provenance metadata.
- [ ] Make reports diffable between baseline and optimized states.

## P2 — Lightweight Control Center

- [ ] Build GUI only on top of stable hub contracts.
- [ ] Keep GUI and CLI free of duplicated business logic.
- [ ] Preferred target: lightweight C++ GUI using Qt or ImGui/SDL; avoid Electron-scale runtime.
- [ ] Tabs: Home, Performance, Games, Tools, Diagnostics.
- [ ] Provide prominent `Optimize V100` action.
- [ ] Display unsupported/unknown features instead of hiding them.
- [ ] Make risky operations visibly explicit and reversible.

## P3 — Experimental graphics / compute research

- [ ] Software reconstruction / neural SR.
- [ ] Software frame interpolation.
- [ ] Software ray tracing / ray marching experiments.
- [ ] SSR / SSAO / SSGI / voxel-lighting experiments.
- [ ] Research driver/API compatibility extensions.
- [ ] Keep all experimental features isolated from stable hub dependencies.

## P3 — Alternative driver research

- [ ] Continue independent KMD/UMD/HAL/GPUVM research only under `research/alternative-driver/`.
- [ ] Never make the Hub depend on the alternative driver track.
- [ ] Keep proprietary-driver analysis separate from the production compatibility layer.

## Quality gates

A feature is not considered stable until it has:

1. capability detection;
2. deterministic behavior;
3. provenance/evidence;
4. negative/unknown-path tests;
5. rollback or explicit non-reversible declaration;
6. post-change verification;
7. report/log coverage;
8. no false claims about V100 hardware capabilities.

## Recommended implementation order

`P0 foundation → capability/providers → Home/status → Performance → Driver Doctor → Games → OptiScaler → Graphics → Reports/Benchmarks → GUI → experimental features`
