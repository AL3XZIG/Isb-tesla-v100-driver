# ISB V100 Hub

**ISB — Intelligent Systems Bureau**

Open-source **V100-focused user-space control, compatibility, optimization and diagnostics hub** built on top of an installed NVIDIA driver stack.

> ISB does not replace the NVIDIA kernel/user-mode driver stack. It detects the real hardware and software environment, exposes V100-specific controls, manages compatibility workarounds and external integrations, verifies changes, and provides a lightweight user-facing control center.

## What ISB is

ISB has evolved from a generic **Driver Fixer** concept into a focused **Tesla V100 control plane**.

The base NVIDIA/Google-compatible driver remains responsible for device initialization, native OS driver models, command submission and hardware access. ISB operates above or beside that stack:

```text
Game / Application / Compute workload
                    |
                    v
              ISB V100 Hub
     +--------------+--------------+
     |              |              |
  Optimize       Games          Diagnose
     |              |              |
 Performance   Profiles +      FixEngine
 Controls      OptiScaler     Verification
     |              |              |
     +--------------+--------------+
                    |
             Base Driver Stack
                    |
              Windows / Linux
                    |
                V100 / GV100
```

The goal is simple: **make a Tesla V100 easier to use, tune, diagnose and get more useful work out of without pretending to be a replacement NVIDIA driver.**

## User-facing feature set

### Home

A single V100 status page:

- exact GPU identity and SXM2/PCIe variant;
- driver, CUDA, NVML and API status;
- temperature, utilization, power and clocks;
- ECC state/errors;
- active profile and applied ISB changes;
- detected problems and recommended actions;
- one-click **Optimize V100** workflow.

### Performance

V100-aware monitoring and tuning where the installed stack exposes the required controls:

- GPU and HBM utilization;
- temperature;
- power draw and power limit;
- SM and memory clocks;
- PCIe traffic and link state;
- ECC telemetry;
- running processes;
- persistence mode;
- application clocks;
- compute mode;
- supported power/performance controls;
- profile presets such as Balanced, Gaming, Compute, AI/Tensor, Maximum Performance, Low Power and Custom.

Every control follows **requested → applied → verified** and records whether it is current, persistent, temporary or requires a reset/restart.

ISB must never claim a setting was applied when the underlying management API rejected or ignored it.

### Games

A lightweight game compatibility manager:

- discover Steam/Epic/GOG/standalone installations where detectable;
- identify executable and graphics API where possible;
- detect DX11/DX12/Vulkan and relevant upscaler/runtime signals;
- maintain per-game compatibility profiles;
- detect OptiScaler applicability;
- show compatibility as `AVAILABLE`, `UNAVAILABLE` or `UNKNOWN`;
- support safe bulk optimization for compatible games;
- back up every modified file and support rollback;
- keep anti-cheat handling conservative.

The safe workflow is:

**SCAN → DETECT → COMPATIBILITY → DRY-RUN → USER APPROVAL → INSTALL → VERIFY → LOG → ROLLBACK**

Unknown compatibility must not be silently treated as supported. Online/anti-cheat games must never be modified silently.

### OptiScaler Manager

OptiScaler is an **external managed component**, not an ISB reimplementation.

ISB should provide:

- installed/version detection;
- compatible-version selection;
- update and rollback;
- per-game configuration;
- backups and hash verification;
- installation logs;
- compatibility state;
- provenance and license metadata.

ISB must respect OptiScaler's license and distribution requirements and must not silently download arbitrary binaries.

### Driver Doctor

The original Driver Fixer capability remains a core subsystem:

- hardware/driver/API fingerprinting;
- deterministic diagnosis;
- known-issue/rule matching;
- reversible workarounds;
- FixEngine plans;
- dry-run and explicit apply;
- verification after changes;
- rollback on failure;
- deterministic diagnostic reports.

### Tensor Core Lab

V100-specific compute diagnostics and benchmarks:

- FP16/INT8 probes where supported;
- Tensor Core throughput;
- HBM bandwidth;
- CUDA compute probes;
- baseline/candidate comparisons;
- correctness checks before performance claims.

### Thermal Guard

Continuous or on-demand analysis of:

- temperature;
- thermal throttling;
- power throttling;
- clock drops;
- ECC anomalies;
- abnormal utilization/power patterns.

### Interconnect

Separate, capability-aware diagnostics for:

- PCIe generation and width;
- PCIe traffic/errors where exposed;
- NVLink topology/state where present;
- CUDA P2P capability;
- measured interconnect bandwidth.

### Graphics Tweaks

Safe per-game/user-space controls where technically available:

- resolution scaling;
- sharpening;
- dynamic resolution settings;
- frame pacing;
- VSync/FPS configuration where the application exposes a safe configuration path;
- shader/cache management;
- selected image-quality and latency options;
- OptiScaler configuration.

These are configuration/compatibility features, not claims that V100 gains hardware features it does not contain.

### V100 Benchmark

A reproducible benchmark suite covering:

- graphics;
- CUDA compute;
- Tensor Cores;
- HBM bandwidth;
- PCIe/interconnect;
- application-level before/after measurements.

Results must identify the exact GPU, driver, application, configuration and test revision.

### One-click report

ISB can generate a deterministic report bundle containing, where available:

```text
manifest.json
gpu.json
driver.json
capabilities.json
games.json
optiscaler.json
performance.json
errors.json
logs/
```

## Optimize V100

The primary high-level workflow is **Optimize V100**.

It analyzes:

1. hardware and exact V100 variant;
2. OS and driver stack;
3. exposed APIs and management interfaces;
4. current performance/thermal state;
5. game/application configuration;
6. OptiScaler compatibility;
7. applicable FixEngine rules;
8. available performance controls.

It then produces an explicit plan showing:

- proposed changes;
- expected effect category;
- required privileges;
- restart/reset requirements;
- risks and unsupported items;
- verification steps.

No mutation is performed without an explicit apply operation. Every applied change is logged and reversible where technically possible.

## Architecture

```text
                    ISB V100 Hub
                         |
       +-----------------+-----------------+
       |                 |                 |
   Control Plane      Feature Modules    Evidence
       |                 |                 |
    core/cli        games/performance   diagnostics/
       |             integrations/      verification/
       |             benchmarks/        reports
       |                 |
       +---------> FixEngine <----------+
                         |
                 Provider / API Layer
             NVML / CUDA / Vulkan / DXGI
                         |
                   Base Driver Stack
                         |
                      V100/GV100
```

The GUI is a frontend to the same control-plane contracts used by the CLI. Business logic must not be duplicated between GUI and CLI.

Detailed architecture: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Repository structure

The repository is organized around the user-facing hub while preserving the existing CAL, FixEngine and verification foundations:

```text
core/                  shared state, orchestration and contracts
cal/                   capability abstraction; no runtime probing
capabilities/           V100/GV100 capability definitions
providers/              NVML, CUDA, Vulkan, DXGI and platform providers
fix/                    deterministic FixEngine and reversible actions
games/                  game discovery, compatibility and per-game profiles
performance/            telemetry, tuning and V100 performance controls
integrations/           OptiScaler, DXVK/VKD3D and other external components
benchmarks/             reproducible graphics/compute/Tensor/HBM/PCIe tests
diagnostics/            IDR, fingerprints, errors and evidence
verification/           regression and post-change verification
reports/                deterministic report bundles
profiles/               hardware, driver, workload and application profiles
hub/                    user-facing control-plane orchestration
control-center/         optional lightweight GUI frontend
cli/                    headless interface to the same hub contracts
installer/              install/configure/verify/rollback workflows
drivers/                base-driver detection and lifecycle metadata
compute/                CUDA/DirectCompute/compute compatibility
graphics/               graphics compatibility infrastructure
neural/                 V100 neural/SR/denoising research and runtime experiments
database/               drivers, GPUs, applications and known issues
manifests/              reproducibility manifests
research/               experimental alternative-driver work
third_party/            audited external references/components
legal/                  licensing and proprietary-component policy
tests/                  unit/compatibility/regression/hardware tests
docs/                   architecture, tasks and protocols
.github/                CI
```

`research/alternative-driver/` remains isolated. It is not required by the V100 Hub.

## Capability model

ISB separates three layers:

1. **Hardware capability** — what the physical V100 contains.
2. **Base-driver capability** — what the installed driver/runtime exposes on the current OS.
3. **ISB capability** — what ISB adds above the base stack.

For V100 this distinction is mandatory. GV100/SM70 has first-generation Tensor Cores and ECC HBM2, but no RT Cores, dedicated Optical Flow Accelerator or MIG. Software features must not be named as native hardware support when they are implemented through another path.

For example, if a particular NVIDIA/Google driver exposes DirectCompute, ISB detects and uses that **base-driver capability**; ISB does not claim to have implemented DirectCompute itself.

## V100 scope

Primary qualification target: **Tesla V100 SXM2 16 GB**.

V100 PCIe and 32 GB variants are tracked separately and must be detected rather than assumed.

Initial OS scope:

- Windows 10/11 x64;
- Linux x86-64.

Legacy Windows/BSD support is separate qualification work.

## What ISB cannot magically add

- RT Cores to V100;
- a dedicated Optical Flow Accelerator;
- MIG;
- display outputs that are not physically present;
- proprietary driver functionality that the installed stack does not expose and cannot legally/technically be substituted for;
- arbitrary kernel-driver recovery from user space.

ISB may provide software alternatives, compatibility paths or user-space compute implementations, but those must be explicitly labeled as such.

## Development principles

1. **Hub first, driver replacement later.**
2. **Reuse existing foundations.** CAL, capability models, FixEngine and verification are shared infrastructure.
3. **One control plane.** GUI and CLI call the same contracts.
4. **Evidence before claims.** Probes and benchmarks record provenance and verification state.
5. **Requested → applied → verified.** Never report an unverified configuration as successful.
6. **Reversible by default.** Back up before mutation and provide rollback where possible.
7. **Unknown is not available.** Unknown compatibility must remain visible.
8. **External components stay external.** OptiScaler/DXVK/VKD3D retain their own licenses and provenance.
9. **No silent global modification.** Especially for online/anti-cheat applications.
10. **Keep the client lightweight.** C++ backend and a lightweight GUI; no Electron-scale runtime and no unnecessary always-on service.
11. **V100-specific knowledge is the differentiator.** Do not build a generic GPU suite at the expense of precise V100 behavior.
12. **Research remains isolated.** The independent KMD/UMD track never becomes an accidental dependency.

## Status

**V100 Hub architecture — implementation phase.**

CAL v1, capability definitions, driver fingerprinting, FixEngine and verification are foundations. The next implementation focus is the hub control plane, performance/telemetry, game manager, OptiScaler management, reports, benchmarks and the lightweight GUI.

## License

Project licensing and third-party provenance are maintained under [`legal/`](legal/).
