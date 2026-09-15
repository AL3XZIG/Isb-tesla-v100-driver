# ISB Tesla V100 Driver

**ISB — Intelligent Systems Bureau**

Open-source research and engineering project for NVIDIA Tesla V100 / GV100 systems.

> The project does not attempt to turn GV100 into an RTX GPU. It focuses on exposing, orchestrating, measuring, and extending what the hardware can actually do, with software compensation only where technically feasible.

## Status

**Architecture / Experimental Protocol phase.**

The repository starts with architecture, interfaces, diagnostics, reproducibility, and experimental methodology before higher-level feature implementation.

## Goals

- Detect and characterize Tesla V100 hardware and runtime environment.
- Provide a Capability Abstraction Layer (CAL).
- Separate OS primitives (HAL) from technology-specific providers.
- Manage and verify supported driver/runtime stacks without redistributing proprietary NVIDIA binaries.
- Provide CUDA, graphics, neural, diagnostics, and benchmark infrastructure.
- Research workload-aware execution across CUDA Cores and Tensor Cores.
- Investigate neural super-resolution/reconstruction and external accelerator workflows.
- Keep software ray tracing and frame generation explicitly experimental/research-only.
- Reuse or integrate mature open-source components where they provide a better-tested implementation than a new ISB-specific replacement.

## Non-goals

- No RT Core emulation claim.
- No dedicated Optical Flow Accelerator claim.
- No proprietary NVIDIA DLSS implementation.
- No replacement NVIDIA kernel/display driver in MVP.
- No redistribution of proprietary NVIDIA binaries without appropriate rights.
- No universal game compatibility guarantee.
- No guarantee of real-time frame generation.
- No blind copying of third-party source code without license and provenance review.

## Upstream integration

ISB uses a **reuse-first, provenance-first** strategy: mature open-source code can be bundled when its license, dependencies, and hardware scope make that appropriate; otherwise it remains an external component or isolated reference.

Current upstream inputs:

- **Mesa 3D** — layered graphics/API architecture reference. Individual source files require SPDX-level audit before import.
- **NVIDIA Open GPU Kernel Modules** — OS-agnostic versus platform-specific driver architecture reference. The current upstream target is Turing and later, so it is not treated as a V100 backend.
- **OptiScaler** — external GPL-3.0-or-later upscaling/frame-generation component. ISB can eventually detect, configure, validate, and launch it without making it part of ISB core.
- **fakenvapi** — MIT-licensed NVAPI/low-latency compatibility reference. An isolated `low_latency.h` snapshot is retained under `third_party/reference/` for future Windows compatibility work and is not built.
- **DLSS-Enabler** — MIT-licensed application interception and external-component orchestration reference. Proprietary NVIDIA binaries are not bundled.

Machine-readable provenance: [`third_party/UPSTREAM_COMPONENTS.json`](third_party/UPSTREAM_COMPONENTS.json).

Legal/provenance policy: [`legal/THIRD_PARTY_UPSTREAM.md`](legal/THIRD_PARTY_UPSTREAM.md).

## Graphics compatibility foundation

The first implementation boundary is provider-neutral rather than vendor-specific:

```text
Game / Application
        |
        v
ISB Compatibility Layer
        |
        +-- future DX11/DX12/Vulkan interception
        +-- external component selection/validation
        |
        v
UpscalerBackend / GraphicsBackend
        |
        v
ISB Runtime / CAL / Provider
```

ISB now provides an initial `UpscalerBackend` abstraction and an external-component model. These allow future integrations with OptiScaler, FSR, XeSS, DLSS-compatible application paths, and an eventual ISB neural backend without coupling CAL to one graphics technology.

## Architecture

Control Center / CLI / Remote API → ISB API → ISB Core → CAL / Diagnostics / Profiles → Providers → HAL → OS / Driver / Runtime Stack → CUDA / Vulkan / OpenGL / Integrations → Neural / Compute / Graphics → Tesla V100 / GV100.

For application-level graphics compatibility, an additional isolated layer may sit above the runtime:

Application / Game → ISB Compatibility Layer → external upscaling/interception component → ISB Runtime / graphics backend → render/display GPU.

Detailed architecture: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Repository structure

```text
core/                  Core orchestration and GPU state
cal/                   Capability Abstraction Layer
hal/                   Low-level OS primitives
providers/             Technology/runtime-specific providers
capabilities/          Capability schemas and definitions
drivers/               Driver stack detection and management
integrations/          External graphics/runtime integrations
graphics/              Graphics runtime and compatibility infrastructure
compute/               CUDA/compute infrastructure
neural/                Neural runtime and data acquisition
research/              Research-only components
benchmarks/            Reproducible benchmark workloads
diagnostics/           Diagnostics and IDR reports
performance/           Performance measurement infrastructure
models/                Model registry and metadata
installer/             Installation and verification workflow
control-center/        Optional GUI control plane
cli/                   Command-line interface
profiles/              Hardware/workload profiles
manifests/             Reproducibility and stack manifests
tests/                 Unit/integration/system tests
tools/                 Development and research tooling
third_party/           Audited upstream licenses and isolated references
docs/                   Project documentation
legal/                 Licensing and proprietary-component policy
.github/               CI and repository automation
```

## Workload-Aware Resource Utilization

The project investigates whether a GV100-aware execution policy can dynamically select, decompose, transform, and schedule workload stages according to available execution resources and runtime constraints.

> Do not directly emulate hardware that GV100 does not have. Redistribute computational work across the resources GV100 actually provides.

The hypothesis is tested experimentally against strong existing baselines rather than assumed from hardware specifications.

## Experimental status model

- **Stable** — supported engineering functionality with reproducible validation.
- **Experimental** — functional research/engineering work whose behavior or compatibility is still being evaluated.
- **Research** — exploratory work with no production guarantee.
- **Verify Before Freeze** — claim or interface requiring source and/or hardware validation before becoming normative.

## Experimental protocol

The workload-aware execution research uses controlled baselines, oracle definitions, measurement modes, leakage prevention, statistical analysis, non-stationarity tests, and reproducibility requirements.

Protocol documents live under `docs/experimental/`.

## Hardware scope

The primary development target is **Tesla V100 SXM2**. V100 PCIe is a separate hardware variant and must never be silently mixed with SXM2 measurements or assumptions.

Hardware facts and performance figures are classified separately; peak specifications are not treated as achieved application performance.

## Development principles

1. Evidence before claims.
2. Measurements before optimization claims.
3. Explicit separation of stable, experimental, and research code.
4. No benchmark result is valid without provenance.
5. Baselines must be workload-specific and fair.
6. Profiling overhead must not be hidden inside production latency measurements.
7. Hardware variants and runtime configurations must be recorded.
8. Proprietary components remain externally supplied unless redistribution rights are established.
9. Mature upstream components should be integrated through explicit, auditable boundaries instead of duplicated blindly.
10. Every imported third-party source file must have provenance and license metadata.

## License

The project license and third-party component policy are maintained under [`legal/`](legal/).

Proprietary NVIDIA software is not bundled by default.
