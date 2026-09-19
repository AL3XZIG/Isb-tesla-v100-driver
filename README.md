# ISB V100 Hub

**ISB — Intelligent Systems Bureau**

Open-source V100-focused user-space control, compatibility, optimization and diagnostics platform built on top of an installed NVIDIA driver stack.

> ISB does not replace the NVIDIA kernel/user-mode driver stack. The installed NVIDIA/Google-compatible driver remains responsible for hardware access and native OS integration. ISB provides the user-facing control plane, V100-specific capability detection, diagnostics, optimization workflows, compatibility tooling and external-component management.

[Русская версия / Russian version](README_RU.md)

## What is ISB?

ISB started as a V100 Driver Fixer concept and evolved into a V100 control plane and compatibility/enhancement platform.

The goal is simple: **make Tesla V100 easier to use, tune, diagnose and get useful work out of without pretending to be a replacement NVIDIA driver.**

The main control path is:

Game / Application / Compute workload → ISB V100 Hub → providers and feature modules → NVIDIA base driver → Tesla V100.

## Main features

### Control Center / Hub

A single control plane for:

- exact GPU identity and V100 variant;
- driver/runtime/API status;
- capabilities and provenance;
- telemetry;
- performance controls;
- diagnostics;
- games and compatibility;
- OptiScaler;
- reports and verification.

GUI and CLI use the same Hub contracts. GUI code must not call low-level providers directly.

### Performance and Optimize V100

V100-aware monitoring and tuning where the installed driver exposes the required APIs:

- utilization, temperature and power;
- SM/HBM clocks and VRAM;
- ECC;
- PCIe and NVLink where available;
- running processes;
- persistence mode;
- application clocks;
- compute mode;
- supported power/performance controls;
- workload profiles.

The high-level optimization pipeline is:

**SCAN → DETECT → ANALYZE → PLAN → REVIEW → APPLY → VERIFY → RESULT**

Mutations follow **requested → applied → read-back → verified**. No unverified change is reported as successful.

### Driver Doctor / FixEngine

- hardware/driver/API fingerprinting;
- deterministic diagnosis;
- known-issue rules;
- reversible workarounds;
- dry-run plans;
- explicit apply;
- post-change verification;
- rollback;
- deterministic reports.

### Games

Planned V100-focused game management includes discovery, executable/API detection, compatibility profiles, per-game configuration, OptiScaler detection, safe optimization, backups and rollback.

Unknown compatibility remains **UNKNOWN** and is never silently treated as supported.

### OptiScaler Manager

OptiScaler remains an **external component**. ISB manages it rather than reimplementing it:

- version detection and selection;
- configuration;
- backup and hash verification;
- update and rollback/remove;
- provenance and license metadata.

### Graphics Enhancement

Software graphics features may include:

- spatial upscaling;
- resolution scaling;
- sharpening;
- dynamic resolution;
- frame-pacing telemetry;
- selected image-quality/latency controls;
- external OptiScaler integration.

Experimental work may investigate CUDA/Tensor neural upscaling, frame interpolation and software ray/lighting techniques. These are software alternatives, not native RTX hardware features.

### Diagnostics, reports and benchmarks

ISB is designed to produce evidence-backed reports and reproducible benchmarks covering CUDA, Tensor Cores, HBM, graphics, PCIe/NVLink and application-level before/after measurements.

Reports preserve provenance and distinguish real, mock, unavailable and unverified information.

## Capability model

ISB separates:

1. Hardware capability — what the physical V100 contains.
2. Base-driver capability — what the installed driver/runtime exposes.
3. Observed capability — what providers actually observe.
4. Verified capability — what evidence/tests demonstrate.
5. ISB capability — what ISB adds above the base stack.

Capability states are explicit: AVAILABLE, UNAVAILABLE, UNKNOWN, PERMISSION_DENIED and ERROR.

**UNKNOWN is not UNAVAILABLE.**

For V100, ISB must not claim native RT Cores, a dedicated Optical Flow Accelerator, MIG, DLSS hardware or other features absent from GV100/Volta. Software implementations must be labeled as software.

## Driver lifecycle

The planned Driver Manager covers:

- NVIDIA/Google driver catalog metadata;
- version and branch detection;
- compatibility matching;
- URL/artifact parsing;
- download;
- checksum/signature verification;
- local cache;
- provenance;
- explicit user-controlled installation.

ISB must not silently replace the installed driver or execute arbitrary downloaded installers.

## Release Builder and Installer

The project plans a reproducible release pipeline:

**tag → clean checkout → configure → build → test → package → SHA256/manifest → release notes → GitHub Release**

Target artifacts:

- Windows x64: ZIP and EXE;
- Linux x86_64: DEB and TAR.GZ.

Generated binaries are release artifacts, not committed build outputs.

## V100 scope

Primary qualification target: **NVIDIA Tesla V100 SXM2 16 GB**.

Also tracked separately: V100 PCIe, V100 32 GB, multi-GPU configurations, and headless compute with a separate display GPU.

Initial OS scope:

- Windows 10/11 x64;
- Linux x86-64.

Legacy Windows/BSD support is separate qualification work.

## Architecture and documentation

The repository is organized around one Hub/control plane with shared providers, FixEngine, compatibility, graphics, performance, diagnostics, verification and reporting layers.

Read these documents before making architectural changes:

- [Architecture](docs/ARCHITECTURE.md)
- [Implementation plan](docs/IMPLEMENTATION_PLAN.md)
- [Roadmap status](docs/ROADMAP_STATUS.md)
- [Tasks](docs/TASKS.md)
- [Master specification](docs/MASTER_SPEC.md)
- [Master addendum](docs/MASTER_TZ_ADDENDUM.md)
- [Current implementation](docs/CURRENT_IMPLEMENTATION.md)
- [Driver and release pipeline](docs/DRIVER_AND_RELEASE_PIPELINE.md)
- [Agent guide](docs/AGENT_GUIDE.md)

## Current implementation status

The repository is in active implementation. Established foundations include common status/result/error contracts, V100/GV100 capability definitions, Hub/control-plane architecture, qualification/evidence gating, read-only NVML integration, telemetry/tuning/optimization contracts and deterministic mock slices, FixEngine foundations, verification infrastructure, experimental graphics/render-path foundations, OptiScaler boundaries, Control Center foundations, and driver/release architecture.

The remaining work is mainly the transition from contracts, mocks and partial providers to a fully integrated real V100 stack:

**M0 Build & stabilization → M1 Capability Engine → M2 real providers → M3 real telemetry → M4 tuning/Optimize V100 → M5 Driver Doctor → M6 graphics/render path → M7 Games/OptiScaler → M8 Driver Manager → M9 Release Builder/Installer → M10 Control Center → M11 benchmarks/evidence → M12 experimental graphics → physical V100 qualification.**

The authoritative implementation-gap roadmap is [docs/ROADMAP_STATUS.md](docs/ROADMAP_STATUS.md) and [docs/TASKS.md](docs/TASKS.md).

## What ISB does not do

ISB does not magically add hardware capabilities that V100 does not physically contain. It does not replace the NVIDIA kernel driver, silently modify online/anti-cheat games, silently install arbitrary external binaries, report mock data as physical V100 evidence, or make UNKNOWN look like AVAILABLE.

The independent GV100 KMD/UMD driver remains isolated under research/alternative-driver/ and is not a dependency of the main Hub.

## Development principles

1. Hub first.
2. Evidence before claims.
3. Requested → applied → verified.
4. Unknown stays unknown.
5. Reversible changes by default.
6. GUI and CLI share one control plane.
7. External components remain external.
8. V100-specific behavior is the priority.
9. No silent global mutation.
10. Research code stays isolated.

## Contributing

Before making architectural changes, read MASTER_SPEC.md, MASTER_TZ_ADDENDUM.md, AGENT_GUIDE.md, CURRENT_IMPLEMENTATION.md, ROADMAP_STATUS.md and TASKS.md.

Do not create duplicate subsystems when an existing Hub, provider, FixEngine, capability or verification contract already owns the responsibility.

## License

Project licensing and third-party provenance are maintained under legal/.
