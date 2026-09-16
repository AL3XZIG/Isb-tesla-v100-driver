# ISB Tesla V100 Driver

**ISB — Intelligent Systems Bureau**

Open-source research and engineering project dedicated to the **NVIDIA Tesla V100 / GV100**.

ISB is building an independent, modular software stack intended to make V100 useful across **compute, AI, graphics, rendering, games, professional applications, diagnostics, and remote/streaming workloads**.

> **Hardware scope is intentionally V100-only.** Do not add dedicated support for CMP, P100, P104/P106, GeForce, RTX, or unrelated NVIDIA architectures unless the project owner explicitly changes this policy.

---

## AI / Agent Briefing

If you are an AI coding agent, researcher, architect, or code reviewer working from this repository, treat this README as the **project contract**.

### Before doing anything

1. Inspect the complete repository structure.
2. Read the existing `docs/`, `cal/`, `core/`, `hal/`, `providers/`, `drivers/`, and `tests/` material before changing architecture.
3. Determine what is **implemented**, **experimental**, **stubbed**, and **planned**. A TODO, interface, mock, or comment is not evidence that a feature works.
4. Trace dependencies and ownership before creating a new abstraction.
5. Preserve existing working behavior unless replacement is intentional and tested.
6. Add regression tests for behavioral changes.
7. If a GV100 hardware fact is uncertain, mark it **Unverified** and research or measure it. Never invent register addresses, protocols, capabilities, or results.
8. Report what was actually tested. Compilation is not hardware validation.

### Engineering behavior expected from agents

- Think like a **GPU driver engineer and systems architect**, not a generic application developer.
- Keep **GV100-specific hardware logic**, **OS integration**, and **userspace/API logic** separated.
- Prefer small, testable layers over a giant monolithic rewrite.
- Do not fake hardware capabilities just to make an API return success.
- Do not silently fall back to another GPU architecture.
- Do not copy proprietary NVIDIA source or redistribute proprietary NVIDIA binaries.
- Reuse mature open-source components only with explicit license/provenance review.
- When reverse-engineering, record source/version, hardware variant, OS, observations, experiment, and confidence.
- If an architectural assumption is wrong, fix the boundary rather than adding layers of hacks around it.

### Definition of done

A feature is not complete merely because it compiles. Where applicable it needs:

- clear ownership;
- error handling;
- tests;
- diagnostics;
- reproducible validation;
- documentation;
- hardware/runtime provenance;
- explicit capability/status reporting.

---

## Mission

Build a clean, verifiable software foundation around **GV100** and progressively expose its real hardware capabilities through modern software interfaces.

Long-term targets include:

- device discovery and initialization;
- BAR/MMIO and register access;
- HBM2/VRAM management;
- GPU virtual memory/MMU;
- DMA;
- command submission and synchronization;
- compute execution;
- CUDA/runtime interoperability where technically feasible;
- Vulkan/OpenGL;
- Windows WDDM integration;
- Direct3D application compatibility;
- professional GPU applications;
- games;
- diagnostics/profiling/benchmarking;
- remote GPU and streaming workloads;
- optional workload-aware and neural optimization layers.

The project must always distinguish **physical GV100 capabilities** from software translation, scheduling, emulation, or compensation.

---

## Hardware Scope

### Primary target

**NVIDIA Tesla V100 / GV100**

Initial development target:

- **Tesla V100 SXM2 16 GB HBM2**

Later validation may include:

- Tesla V100 SXM2 32 GB;
- Tesla V100 PCIe.

### Variant rule

Never silently mix SXM2 and PCIe assumptions or benchmark results. Record:

- exact board variant;
- memory size;
- firmware;
- host platform;
- OS/kernel/build;
- driver/runtime version;
- test configuration.

### Out of scope

Unless explicitly approved by the project owner:

- NVIDIA CMP;
- Tesla P100;
- P104/P106 and similar mining variants;
- GeForce;
- RTX-specific hardware;
- other NVIDIA architectures.

Generic abstractions are allowed when they improve code quality, but the actual hardware backend is **GV100/V100-specific**.

---

## Architecture

```text
Applications / Games / Professional Software
                    |
                    v
       ISB Compatibility / API Layer
                    |
          +---------+---------+
          |                   |
      Graphics             Compute
   Vulkan / OpenGL       CUDA / Runtime
          |                   |
          +---------+---------+
                    |
                    v
              ISB Userspace
                    |
                    v
                 ISB Core
                    |
              CAL / Diagnostics
                    |
                    v
              GV100 Provider
                    |
                    v
           Hardware Abstraction
                    |
          +---------+---------+
          |                   |
       Linux DRM           Windows WDDM
          |                   |
          +---------+---------+
                    |
                    v
              Tesla V100/GV100
```

The architecture may evolve. Existing versioned documents under `docs/` define implementation details; this README defines the project-wide direction and boundaries.

---

## Core Layers

### GV100 hardware layer

Responsible for actual GV100 behavior:

- PCIe/SXM2 interaction;
- BAR/MMIO;
- register definitions;
- engines;
- interrupts;
- memory controllers/HBM2;
- GPU MMU/address spaces;
- channels/queues;
- synchronization;
- clocks/power where safely controllable;
- firmware interfaces;
- diagnostics.

Undocumented behavior must have evidence and verification status.

### HAL

OS/hardware primitives used by the common driver core:

- memory;
- DMA;
- synchronization;
- interrupts/events;
- virtual memory primitives;
- device resources;
- logging/diagnostics.

### CAL

The **Capability Abstraction Layer** describes what the current V100 environment can actually provide.

CAL must derive capabilities from detected hardware, runtime state, OS support, and verified behavior. It must not report desired features as available merely because an interface exists.

### Providers

Concrete implementations connecting CAL/core to:

- GV100 operations;
- CUDA/runtime stacks;
- graphics runtimes;
- external compatibility components;
- diagnostics.

### OS integration

Primary long-term targets:

- **Linux x86-64**
- **Windows x86-64**

BSD is optional/future work and must not destabilize the primary Linux/Windows architecture.

Windows support must use the actual **WDDM KMD/UMD model**. Do not treat Linux DRM code, CUDA runtime code, an ICD, or an application interception layer as interchangeable with a Windows display driver.

Windows 7/8.1 are not primary targets unless explicitly promoted later.

---

## Development Roadmap

The project moves from hardware control upward.

### Phase 0 — Foundation

- CAL interfaces;
- diagnostics;
- verification tools;
- reproducibility manifests;
- GV100 hardware database;
- research notes.

### Phase 1 — Device access

- device discovery;
- BAR discovery;
- safe MMIO;
- GPU identification;
- register infrastructure;
- basic diagnostics.

### Phase 2 — Initialization

- firmware analysis;
- engine discovery;
- memory-controller discovery;
- HBM2 detection;
- GPU initialization sequence.

### Phase 3 — Memory / VM

- VRAM allocation;
- GPU virtual address spaces;
- page tables/MMU;
- DMA;
- synchronization.

### Phase 4 — Command execution

- channels/queues;
- command buffers;
- submission;
- fences/events;
- interrupts;
- engine scheduling.

### Phase 5 — Compute

- minimal compute execution;
- userspace runtime;
- reproducible compute tests;
- CUDA interoperability research where technically feasible.

### Phase 6 — Linux graphics

- DRM integration;
- Vulkan/OpenGL path;
- rendering validation.

### Phase 7 — Windows

- WDDM kernel-mode integration;
- userspace driver architecture;
- memory/synchronization integration;
- graphics API integration;
- application compatibility.

### Phase 8 — Real workloads

- games;
- professional applications;
- rendering;
- AI/ML;
- remote/streaming;
- profiling and optimization.

These phases are directional. Foundational validation must not be skipped merely to reach a higher-level feature.

---

## Windows Is a First-Class Target

ISB is **not Linux-only**.

Windows matters because V100 must eventually be usable with games and proprietary professional software as well as ordinary compute workloads.

Keep these concepts separate:

- WDDM KMD;
- Windows UMD;
- CUDA runtime;
- Vulkan/OpenGL ICDs;
- Direct3D interfaces;
- application-level interception/compatibility layers;
- external NVIDIA driver packages.

A successful experiment with one of these does not prove that the others work.

---

## Existing NVIDIA / External Software

Existing NVIDIA or cloud/legacy driver stacks may be used as **external baselines or research inputs** when technically and legally appropriate.

For every tested stack record the exact version, OS, hardware variant, APIs, observed behavior, and limitations.

A legacy driver version is a **research/compatibility baseline**, not automatically an ISB dependency.

Proprietary NVIDIA binaries are not bundled by default.

---

## Reverse Engineering

Reverse engineering is used to understand GV100 hardware and interoperable interfaces.

For important discoveries record:

- source/binary;
- exact version;
- hardware variant;
- OS;
- observation;
- experiment;
- confidence;
- independent reproduction status.

Preferred evidence order:

1. direct hardware measurement;
2. public technical documentation;
3. open-source implementations such as Nouveau/Mesa;
4. controlled observation of existing drivers;
5. binary analysis where legally appropriate;
6. inference/hypothesis.

**Inference is not a hardware fact.**

---

## Open-Source Reuse

ISB follows **reuse-first / provenance-first** engineering.

Relevant upstream projects may include Mesa, Nouveau, NVIDIA Open GPU Kernel Modules, and external graphics compatibility projects.

Before importing source:

- identify exact upstream version/commit;
- identify license;
- preserve copyright notices;
- record provenance;
- verify license compatibility;
- isolate reference-only code from production code when appropriate.

See `third_party/` and `legal/`.

---

## Graphics / Compatibility Research

ISB may eventually integrate or orchestrate external technologies such as:

- FSR;
- XeSS;
- OptiScaler;
- compatible application-level reconstruction paths;
- future ISB neural reconstruction backends.

These technologies do **not** give V100 missing silicon features.

ISB does not claim:

- RT Core hardware;
- NVIDIA Optical Flow hardware;
- proprietary NVIDIA DLSS implementation;
- automatic RTX support;
- universal frame-generation compatibility.

Software can redistribute computation or translate APIs. It cannot manufacture missing hardware blocks.

---

## Workload-Aware Optimization

Research may investigate better use of resources actually present on GV100, including:

- CUDA cores;
- Tensor Cores;
- HBM2 bandwidth/capacity;
- concurrent engines;
- asynchronous execution;
- CPU/GPU cooperation;
- external accelerators.

Optimization claims must be experimentally validated.

**Peak hardware specifications are not application benchmark results.**

Claims comparing V100 with another GPU require a defined workload, software version, configuration, methodology, and reproducible measurements.

---

## Verification / Benchmarking

Record at minimum:

- GPU and board variant;
- VRAM;
- firmware;
- host platform;
- OS/kernel/build;
- driver/runtime;
- application version;
- API;
- settings;
- workload;
- measurement method;
- run count/statistics;
- known confounders.

Eventually benchmark:

- memory bandwidth;
- compute;
- Tensor Cores;
- AI inference;
- Blender/rendering;
- Vulkan;
- OpenGL;
- Direct3D;
- games;
- professional applications;
- streaming/encoding where supported.

Use an existing NVIDIA stack as a baseline where appropriate and keep hardware/configuration identical.

---

## Status Model

- **Stable** — implemented and reproducibly validated.
- **Experimental** — functional but under active validation.
- **Research** — exploratory; no production guarantee.
- **Unverified** — plausible/observed indirectly but insufficiently validated.
- **Blocked** — waiting on hardware, documentation, firmware, legal permission, or another dependency.

Never silently promote `Experimental`, `Research`, or `Unverified` functionality to `Stable`.

---

## Repository Structure

```text
core/                  Common orchestration and GPU state
cal/                   Capability Abstraction Layer
hal/                   OS/hardware primitives
providers/             Concrete runtime/hardware providers
drivers/               Driver detection and management
graphics/              Graphics infrastructure
compute/               Compute/runtime infrastructure
neural/                Neural research/runtime
integrations/          External software integrations
research/              Research-only experiments
benchmarks/            Reproducible workloads
diagnostics/           Diagnostics and reports
performance/           Performance measurement
models/                Model metadata
installer/             Installation and verification
control-center/        Optional GUI control plane
cli/                   Command-line tools
profiles/              Hardware/workload profiles
manifests/             Reproducibility manifests
tests/                 Unit/integration/system tests
tools/                 Development/research tools
third_party/           Upstream references/provenance
docs/                  Technical documentation
legal/                 Licensing/proprietary software policy
.github/               CI and automation
```

Do not create a new top-level directory merely for convenience. Put code in the layer that actually owns its responsibility.

---

## Documentation

Documentation is part of implementation.

Important technical areas should be documented under `docs/`, including:

- architecture;
- GV100 hardware;
- registers;
- memory/MMU;
- engines;
- firmware;
- Linux;
- Windows;
- graphics;
- compute;
- reverse engineering;
- verification;
- benchmarks;
- known issues;
- build/development.

GitHub Wiki can be used as a navigational encyclopedia, but important technical knowledge should remain versioned in `docs/`.

---

## AI Coding Rules

### Do

- inspect before modifying;
- reuse existing abstractions;
- keep modules focused;
- validate inputs/state;
- return meaningful errors;
- add tests;
- document hardware behavior;
- isolate experimental features;
- preserve third-party provenance;
- use deterministic/reproducible tests;
- report actual test results.

### Do not

- fabricate registers or hardware behavior;
- invent APIs;
- equate compilation with hardware support;
- present mocks as real hardware;
- silently support another GPU family;
- copy proprietary NVIDIA code;
- bundle proprietary NVIDIA binaries;
- hide failures behind unconditional success;
- delete tests because they expose a bug;
- perform a giant rewrite without understanding current architecture;
- optimize before establishing a baseline.

### When uncertain

Do not guess. State:

1. what is known;
2. what is inferred;
3. what is missing;
4. what source, experiment, or hardware test would resolve it.

---

## Guiding Principle

The project follows:

**evidence → hardware understanding → abstraction → implementation → verification → optimization → compatibility**

Not:

**guess → implement → declare support**.

The objective is not to produce a huge codebase quickly. The objective is to build a V100 stack whose behavior can be understood, tested, reproduced, and trusted.

---

## License

See [`legal/`](legal/) for the project license and third-party component policy.

Proprietary NVIDIA software and firmware are not redistributed unless the repository explicitly documents an appropriate legal basis.

---

## Project Status

**Architecture / Experimental Development**

The repository is actively evolving. Current repository contents and versioned documents under `docs/` are the source of truth for implementation state. This README defines the mission, boundaries, engineering rules, and expectations for contributors and AI agents.
