# ISB V100 — Consolidated Product & Technical Specification

> **Status:** normative project specification  
> **Scope:** Tesla V100 / GV100 only  
> **Primary product:** ISB V100 Hub / Control Center / CLI  
> **Implementation rule:** this document describes the intended system and consolidates the project's accumulated requirements. It does **not** imply that every item is already implemented. For implementation state, use `docs/CURRENT_IMPLEMENTATION.md`, source code and CI.

---

## 1. Project identity

**ISB — Intelligent Systems Bureau V100 Hub**

ISB is a V100-focused user-space control, compatibility, diagnostics and optimization platform built above an installed NVIDIA/base driver stack.

The product goal is to make Tesla V100 understandable and useful on a modern workstation for:

- compute;
- AI/ML;
- Tensor Core workloads;
- graphics and rendering;
- games;
- professional applications;
- diagnostics;
- performance tuning;
- remote/streaming workloads;
- compatibility workarounds.

ISB is an **end-user control and compatibility layer**, not a claim that ISB already replaces NVIDIA's proprietary driver.

The independent KMD/UMD/alternative-driver research track remains isolated under `research/alternative-driver/` and is not a prerequisite for the mainline Hub.

---

## 2. Hardware scope

### 2.1 Primary target

- NVIDIA Tesla V100 / GV100
- initial qualification target: Tesla V100 SXM2 16 GB HBM2

### 2.2 Additional qualification targets

- Tesla V100 SXM2 32 GB
- Tesla V100 PCIe

SXM2 and PCIe must never be silently treated as equivalent. Benchmarks and hardware observations must record the exact variant.

At minimum, hardware evidence should record:

- board variant;
- VRAM size;
- firmware/VBIOS where available;
- host platform;
- OS/kernel/build;
- NVIDIA/base driver version;
- CUDA/runtime version;
- relevant API/runtime versions;
- test configuration.


### 2.4 Verified V100 SXM2 32 GB baseline

For the Tesla V100 SXM2 32 GB qualification target, the project should record the following baseline from NVIDIA's V100 documentation:

- Volta / GV100 architecture;
- 5,120 CUDA cores;
- 640 first-generation Tensor Cores;
- 32 GB HBM2;
- approximately 900 GB/s HBM2 bandwidth;
- 15.7 TFLOPS FP32;
- 7.8 TFLOPS FP64;
- 125 TFLOPS Tensor performance;
- up to 300 W maximum power consumption for the SXM2 variant;
- NVLink interconnect, with 300 GB/s bidirectional interconnect bandwidth for the V100 NVLink configuration.

These are **hardware/reference specifications**, not application benchmark results. Exact observed clocks, power limits and active NVLink links must come from the installed module, firmware, base driver and runtime evidence.

The SXM2 module is a server accelerator with no display outputs. In a workstation configuration using an SXM2-to-PCIe adapter, PCIe host connectivity and NVLink topology must be treated as separate observations; the presence of NVLink capability does not prove that an adapter exposes or connects an active NVLink fabric.

### 2.3 Explicitly out of scope

Unless the project owner changes the policy:

- NVIDIA CMP;
- Tesla P100;
- P104/P106 and similar mining variants;
- GeForce;
- RTX-specific hardware;
- unrelated NVIDIA architectures.

Generic interfaces are allowed when they improve code quality, but concrete hardware semantics remain V100/GV100-specific.

---

## 3. Physical capability vs software capability

ISB must always separate three capability layers.

### 3.1 Hardware capability

What physically exists on GV100.

Relevant facts include:

- Volta / SM70;
- HBM2;
- ECC;
- Tensor Cores;
- no RT Cores;
- no dedicated Optical Flow Accelerator;
- no MIG;
- no display outputs on the V100 module itself.

### 3.2 Base-driver capability

What the installed NVIDIA/Google-compatible driver and operating system expose.

Examples:

- CUDA/runtime;
- NVML management;
- Vulkan;
- OpenGL;
- OpenGL;
- Linux DRM/PRIME/offload;
- driver-specific graphics paths.

A base-driver feature is not automatically an ISB-owned feature.

### 3.3 ISB capability

What ISB adds above the base stack:

- planning;
- orchestration;
- diagnostics;
- FixEngine workarounds;
- profiles;
- telemetry presentation;
- verification;
- reports;
- game/application compatibility;
- OptiScaler orchestration;
- render-path planning;
- software graphics enhancement;
- experimental neural/graphics features.

---

## 4. Canonical architecture

The mandatory user-facing path is:

```
User
 |
 +----------------------+
 |                      |
 CLI              Control Center
 |                      |
 +----------+-----------+
            |
        ISB Hub
            |
    +-------+--------+----------------+
    |       |        |                |
  CAL   FixEngine Performance   Integrations
    |       |        |                |
    +-------+--------+----------------+
            |
        Providers
    /     |      |      \
 NVML  CUDA   Vulkan   OS APIs
            |
      Base Driver / OS
            |
        Tesla V100
```

### Ownership rules

- **common/** — shared status/result/error/provenance primitives.
- **cal/** — normalized capability model.
- **providers/** — raw runtime/platform observations and provider-specific access.
- **fixer/** — compatibility rules and fix plans.
- **hub/** — user-facing orchestration and transaction control.
- **cli/** — headless presentation.
- **control-center/** — Qt GUI presentation.
- **performance/** — telemetry/tuning domain.
- **games/** — game discovery and compatibility domain where present.
- **integrations/** — external software boundaries such as OptiScaler.
- **graphics/** — render-path/upscaler infrastructure.
- **neural/** — V100-specific neural/software experiments.
- **diagnostics/** / **verification/** — evidence and post-change verification.
- **reports/** / **manifests/** — deterministic evidence output.
- **drivers/** — external base-driver metadata/lifecycle.
- **installer/** — installation/configuration packaging.
- **research/** — experimental and alternative-driver work.
- **third_party/** / **legal/** — provenance and licensing.

Do not create a second control plane, second FixEngine, second capability model or direct provider calls from frontends.

---

## 5. Core status semantics

Unknown is not Available.

All layers must preserve states such as:

- Available;
- Unknown;
- Unavailable;
- Unsupported;
- PermissionDenied;
- Error;
- Experimental;
- Mock/Synthetic.

Examples:

- unknown temperature → `Unknown`, never `0 °C`;
- unavailable NVML → `Unavailable`, never fabricated telemetry;
- unsupported control → `Unsupported`, never a fake successful write;
- permission failure → `PermissionDenied`;
- mock telemetry → visibly marked `MOCK / DEMO DATA`.

The distinction must survive:

**Provider → CAL → Hub → CLI/GUI → Report**

---

## 6. Hub control plane

The Hub is the single orchestration boundary for user-facing workflows.

Conceptual operations:

- scan;
- inspect;
- status;
- capabilities;
- telemetry;
- profile;
- optimize;
- plan;
- apply;
- rollback;
- verify;
- benchmark;
- diagnose;
- report;
- driver discovery;
- driver download;
- cache management.

The Hub coordinates modules and providers. It must not become a dumping ground for raw NVML/CUDA/Vulkan/Linux platform implementation.

### 6.1 Transaction model

Mutating operations use:

```
READ
  ↓
VALIDATE
  ↓
PLAN
  ↓
USER REVIEW
  ↓
APPLY
  ↓
READ BACK
  ↓
VERIFY
  ↓
RESULT
```

Where technically possible:

```
BACKUP → MUTATE → VERIFY → ROLLBACK ON FAILURE
```

No silent hardware changes.

---

## 7. CAL — Capability Abstraction Layer

CAL describes what the current V100 environment can actually provide.

CAL must derive capabilities from:

- detected hardware;
- provider observations;
- OS;
- installed base driver;
- runtime/API availability;
- permissions;
- verified behavior.

CAL must not report a desired feature as available merely because an interface exists.

### Capability provenance

Every important capability should identify its source, for example:

- hardware;
- NVIDIA base driver;
- Google/base compatibility package;
- NVML;
- CUDA;
- Vulkan;
- Linux DRM/Vulkan/OpenGL;
- ISB;
- external integration;
- experimental;
- mock.

---

## 8. Providers

Provider layer is responsible for concrete external/runtime access.

Expected provider families include:

- NVML;
- CUDA;
- Vulkan;
- Linux graphics stack;
- Linux platform/runtime detection;
- NVIDIA driver source;

- local driver cache.

Providers must:

- preserve provenance;
- expose errors explicitly;
- expose unavailable state explicitly;
- avoid GUI policy;
- avoid profile policy;
- avoid hidden compatibility workarounds.

The current repository already contains provider-oriented infrastructure; agents must inspect the actual implementation before adding duplicates.

---

## 9. Telemetry and performance

The performance subsystem should expose, where the provider supports it:

- GPU utilization;
- memory/HBM utilization;
- temperature;
- power draw;
- power limit;
- SM clock;
- memory clock;
- VRAM used/free/total;
- ECC corrected/uncorrected state;
- PCIe link generation/width;
- PCIe traffic where available;
- NVLink state/topology where present;
- active processes where permitted;
- throttling/performance-state information.

### 9.1 History

The UI/CLI may keep lightweight bounded history for:

- temperature;
- utilization;
- power;
- clocks.

Do not introduce a heavy always-on telemetry daemon for the basic product.

Polling interval must be configurable.

---

## 10. Tuning and hardware controls

Potential controls include:

- Persistence Mode;
- Application Clocks;
- Power Limit;
- Compute Mode;
- Auto Boost semantics where the base driver exposes them;
- other V100-supported controls discovered through capability evidence.

A control is displayed/enabled only when provider evidence supports it.

Each control must expose:

- support state;
- writable state;
- current value;
- requested value;
- resulting value;
- verification state;
- reversibility;
- restart requirement;
- reset requirement;
- permission requirement;
- driver dependency.

A plan must never assume that every NVIDIA control exists on every V100/driver/OS combination.

---

## 11. Profiles

Target profiles:

- Balanced;
- Gaming;
- Compute;
- AI/Tensor;
- Maximum Performance;
- Low Power;
- Custom.

A profile is a **policy/plan**, not an automatic promise.

Selecting a profile must first produce a preview:

```
Requested → Planned → User Review → Applied → Verified
```

The preview must show:

- supported changes;
- unsupported changes;
- unknown controls;
- risks;
- permission requirements;
- reset/restart requirements;
- verification steps.

---

## 12. Optimize V100

The primary product workflow is:

```
SCAN
 → DETECT
 → ANALYZE
 → PLAN
 → USER REVIEW
 → APPLY
 → VERIFY
 → RESULT
```

### Detected

Show observed facts:

- GPU identity;
- variant;
- VRAM;
- driver;
- runtime;
- available APIs;
- health;
- current configuration;
- current capabilities.

### Recommended changes

Show only changes supported by the current evidence.

### Skipped

Explain why something was not changed:

- unsupported;
- unknown;
- permission denied;
- driver-dependent;
- reset required;
- restart required;
- unsafe/non-reversible;
- provider unavailable.

### Risks

Explicitly expose risks before APPLY.

### Apply

Requires explicit user confirmation.

### Verify

After mutation, re-read the state.

A result is not successful merely because a command returned without an error.

---

## 13. Driver Doctor / FixEngine

FixEngine is the single compatibility-rule and workaround engine.

Input:

```
Environment + ErrorEvent + Capabilities
```

Output:

```
Proposed Fix Plan
```

The Hub performs:

- dry-run;
- approval;
- apply;
- rollback;
- verification;
- report.

Rules must be:

- deterministic;
- explainable;
- testable;
- reversible where possible.

The GUI and CLI must not implement a second rule engine.

---

## 14. Driver Manager and external base drivers

ISB manages metadata/lifecycle around an installed NVIDIA/base driver rather than replacing it.

Responsibilities:

1. detect installed driver;
2. fingerprint driver/runtime environment;
3. discover compatible releases;
4. parse official release URLs/pages;
5. normalize vendor/version/branch/OS/package/GPU metadata;
6. download explicitly selected artifacts;
7. verify checksums/signatures where available;
8. maintain a local cache;
9. expose driver information through Hub/CLI/GUI;
10. support explicit installation workflows only when a real installation backend exists;
11. verify the resulting environment.

### Download is not installation

These are separate operations:

```
DISCOVER → SELECT → DOWNLOAD → VERIFY → CACHE
                                      ↓
                                  INSPECT
                                      ↓
                              EXPLICIT INSTALL
                                      ↓
                                  VERIFY
```

ISB must never silently replace the installed NVIDIA Linux driver.

Proprietary NVIDIA/Google binaries are external inputs and are not redistributed without a documented legal basis.

---

## 15. Driver sources

Initial source classes:

- NVIDIA Data Center/Tesla releases;
- NVIDIA vGPU/vWS/GRID sources where applicable;
- Google-provided NVIDIA GPU driver sources where applicable.

For every source record:

- vendor;
- exact version;
- branch/family;
- release date;
- OS;
- architecture;
- package type;
- supported GPU family;
- original URL;
- final download URL;
- checksum/signature data;
- provenance;
- verification state.

Unknown metadata remains Unknown.

---

## 16. Games

Game management is intentionally a compatibility manager, not a giant launcher.

Potential discovery sources:

- Steam;
- Epic;
- GOG;
- standalone installations.

A game record may contain:

- title/id;
- installation path;
- executable;
- graphics API;
- upscaler signals;
- compatibility state;
- selected profile;
- managed files;
- backup state;
- rollback state;
- anti-cheat state.

Unknown compatibility or anti-cheat state must remain visible and default to dry-run/manual approval.

Bulk workflows use:

```
SCAN → DETECT → COMPATIBILITY → DRY-RUN → APPROVAL
     → INSTALL/MODIFY → VERIFY → LOG → ROLLBACK
```

No anti-cheat bypass is part of ISB.

---

## 17. OptiScaler Manager

OptiScaler remains an external component.

ISB integration may provide:

- detection;
- installed version;
- available version;
- compatibility evaluation;
- per-game state;
- configuration;
- install;
- update;
- verify;
- rollback;
- backup;
- hashes;
- provenance;
- logs.

Boundary:

```
ISB
 ↓
OptiScaler integration adapter
 ↓
External OptiScaler
```

Do not rewrite OptiScaler inside the driver or Hub.

Do not silently download arbitrary binaries.

If redistribution/offline bundling is ever introduced, preserve upstream license and provenance obligations.

---

## 18. Graphics and render path

The primary workstation use case may be:

- V100 is headless;
- another GPU/iGPU drives the display;
- V100 performs compute/render work.

The system must distinguish:

- display GPU;
- compute GPU;
- render GPU;
- application-selected adapter;
- OS routing state.

A render-path plan may configure routing only through documented/supported mechanisms.

### Linux

Investigate and use real:

- DRM;
- PRIME Render Offload;
- D3D;
- Vulkan/OpenGL;
- supported driver mechanisms.

Do not invent registry keys or claim universal routing.

### Linux

Investigate and use real:

- PRIME render offload;
- Vulkan device selection;
- X11/Wayland routing;
- documented environment variables and runtime mechanisms.

Universal routing must never be assumed.

---

## 19. Graphics enhancement

Potential capability classes:

### Native/application

- application-native scaling;
- application-native reconstruction.

### External compatibility

- FSR;
- XeSS;
- OptiScaler;
- other separately licensed integrations.

### ISB software

- spatial scaling;
- Lanczos;
- CAS-style sharpening;
- dynamic resolution;
- frame pacing;
- software denoising/reconstruction.

### Experimental research

- frame interpolation;
- neural reconstruction;
- software lighting/ray techniques;
- future V100-specific rendering research.

Every feature must state whether it is:

- native hardware;
- base-driver capability;
- external component;
- ISB software;
- experimental;
- mock.

ISB must never claim:

- native RT Cores;
- native DLSS hardware;
- dedicated Optical Flow hardware;
- universal frame generation.

Software can redistribute computation or translate APIs; it cannot create missing silicon blocks.

---

## 20. Neural / AI optimization

The project may use V100-specific software optimization around resources actually present:

- CUDA cores;
- Tensor Cores;
- HBM2 capacity;
- HBM2 bandwidth;
- concurrent engines;
- asynchronous execution;
- CPU/GPU cooperation;
- external accelerators.

Potential workloads include:

- local inference;
- AI/Tensor workloads;
- denoising/reconstruction;
- model-specific optimization;
- workload-aware scheduling.

All performance claims require reproducible measurement.

Peak hardware specifications are not application benchmark results.

---

## 21. Benchmarking

Benchmark subsystem should cover, where supported:

- HBM bandwidth;
- CUDA compute;
- Tensor Core throughput;
- PCIe/interconnect;
- Vulkan;
- OpenGL;
- Direct3D;
- Blender/rendering;
- AI inference;
- games;
- professional applications;
- streaming/encoding.

Every benchmark must record:

- exact GPU/board;
- VRAM;
- firmware;
- host;
- OS/kernel/build;
- driver/runtime;
- application version;
- API;
- settings;
- workload;
- measurement method;
- run count/statistics;
- known confounders;
- ISB revision.

Before/after optimization comparisons must use the same hardware and reproducible configuration.

---

## 22. Reports and evidence

Diagnostics and verification are evidence infrastructure.

Reports distinguish:

- observed facts;
- capabilities;
- applied changes;
- verification results;
- benchmark measurements;
- experimental hypotheses;
- mock/synthetic evidence.

A deterministic report bundle may contain:

```
manifest.json
gpu.json
driver.json
capabilities.json
performance.json
games.json
optiscaler.json
errors.json
logs/
```

Synthetic evidence must never be presented as physical hardware evidence.

---

## 23. CLI specification

The CLI is a first-class headless frontend over Hub.

It must not duplicate business logic.

### Desired command families

- `status`
- `info`
- `inspect`
- `capabilities`
- `telemetry`
- `monitor`
- `profile list`
- `profile plan <name>`
- `optimize`
- `diagnose`
- `doctor`
- `verify`
- `benchmark`
- `report`
- `graphics status`
- `graphics list`
- `render status`
- `render verify`
- `about`

Exact command names must follow existing source conventions; agents must inspect the actual CLI before adding aliases.

### CLI UX

The CLI should feel like an ISB V100 Control Center:

- compact branded startup banner;
- clean sections;
- readable key/value tables;
- status badges;
- progress stages for real operations;
- useful errors;
- no fake progress;
- no giant ASCII art;
- no NVIDIA branding imitation.

Color policy:

- auto for TTY;
- always;
- never.

Output modes should support:

- human-readable terminal output;
- machine-readable JSON where the underlying operation supports it.

Terminal compatibility:

- Linux terminal;
- SSH;
- tmux;
- 
- PowerShell;
- cmd where practical.

Non-interactive output must remain parseable and must not contain decorative escape sequences when color is disabled.

---

## 24. Control Center GUI specification

Preferred technology when no repository decision overrides it:

**Qt 6 Widgets**

No Electron/Chromium.

The GUI is a thin presentation layer over Hub.

### Navigation

- Home;
- Performance;
- Games;
- Tools.

Prominent action:

**Optimize V100**

### Home

Cards:

- GPU;
- Driver;
- Health;
- Performance;
- Problems.

Show:

- Tesla V100;
- variant;
- VRAM;
- architecture;
- compute capability;
- driver;
- CUDA;
- graphics API state;
- temperature;
- utilization;
- memory utilization;
- power;
- clocks;
- health;
- active profile;
- problems.

Missing values use explicit status text.

### Performance

Show telemetry and bounded history for:

- temperature;
- utilization;
- power;
- clocks.

### Tuning

Show controls only when supported.

Each control exposes:

- support;
- writable;
- current;
- requested;
- resulting;
- verification;
- reversibility;
- restart/reset requirements.

### Profiles

Profiles create plans before mutation.

### Games

Show:

- game;
- executable;
- graphics API;
- upscaler;
- OptiScaler;
- compatibility;
- profile.

### Tools

Include:

- Driver Doctor;
- Benchmark;
- Support Report;
- OptiScaler Manager;
- driver tools where implemented.

### Design

Desired style:

- dark;
- clean;
- technical;
- professional;
- readable;
- V100/ISB-specific identity;
- no acid RGB gamer aesthetic.

---

## 25. Mock mode

Mock mode is required for development without physical V100 hardware.

Mock mode must:

- use deterministic fixture data;
- expose realistic status/error cases;
- clearly display `DEMO / MOCK DATA`;
- never be reported as hardware qualification.

Useful mock scenarios include:

- normal available V100;
- missing provider;
- permission denied;
- unsupported control;
- provider error;
- partial capability;
- failed apply;
- verification failure;
- rollback result.

---

## 26. Installers and release builder

ISB requires reproducible release engineering.

### Windows

Initial artifacts:

- `.zip`;
- `.exe`.

### Linux

Initial artifacts:

- `.deb`;
- `.tar.gz`.

Artifact naming:

```
isb-<version>-<platform>-<arch>.<extension>
```

Every release should contain:

- SHA256SUMS;
- machine-readable release manifest;
- release notes;
- source commit/tag;
- provenance metadata.

Tagged release pipeline:

```
CONFIGURE
 → BUILD
 → TEST
 → PACKAGE
 → CHECKSUM
 → MANIFEST
 → PUBLISH
```

Any failed required stage blocks publication.

---

## 27. Offline/local operation

Basic local functionality should work without Internet when local dependencies are available:

- inspection;
- telemetry;
- diagnostics;
- profiles;
- local reports;
- local verification.

Network access is explicit for:

- driver discovery;
- driver downloads;
- OptiScaler downloads;
- game/application metadata updates;
- ISB updates.

External dependencies may be cached or packaged only when licensing permits and provenance is preserved.

---

## 28. Reverse engineering and research

Reverse engineering is a research method for understanding GV100 and interoperable interfaces.

For important findings record:

- source/binary;
- exact version;
- hardware variant;
- OS;
- observation;
- experiment;
- confidence;
- independent reproduction status.

Evidence priority:

1. direct hardware measurement;
2. public technical documentation;
3. open-source implementations;
4. controlled observation;
5. legally appropriate binary analysis;
6. inference.

Inference is not a hardware fact.

The alternative-driver research track must never become an accidental runtime dependency of the Hub.

---

## 29. Proprietary software and provenance

ISB may use external driver stacks as baselines or runtime dependencies where technically and legally appropriate.

The repository must not:

- copy proprietary NVIDIA source;
- redistribute proprietary NVIDIA binaries without a documented legal basis;
- silently vendor OptiScaler or other third-party code;
- hide third-party provenance.

Before reusing upstream source:

- record exact version/commit;
- record license;
- preserve copyright;
- verify compatibility;
- isolate reference code where appropriate.

---

## 30. Safety and mutation policy

Never implement:

- silent overclocking;
- silent power-limit changes;
- silent registry changes;
- silent global game patching;
- anti-cheat bypass;
- blind driver replacement;
- proprietary binary modification as the default mechanism.

All mutation must be explicit, logged and verifiable.

Where possible:

```
BACKUP
 → APPLY
 → READ BACK
 → VERIFY
 → ROLLBACK IF FAILED
```

---

## 31. Engineering requirements

Language/build:

- C++17;
- CMake;
- RAII;
- const-correctness;
- dependency injection;
- narrow interfaces;
- deterministic serialization;
- meaningful errors;
- minimal dependencies.

Avoid:

- global mutable state;
- giant monolithic GUI classes;
- duplicate business logic;
- fake backend APIs;
- accidental transitive include dependencies.

Public headers must be self-contained and targets must declare their real include/link dependencies.

---

## 32. Testing and qualification

Every significant feature should have:

- positive-path tests;
- negative-path tests;
- unknown/error tests;
- deterministic serialization tests where relevant;
- rollback tests where relevant;
- verification tests;
- report/log tests.

Hardware qualification is separate from software CI.

A clean build is not physical V100 qualification.

A feature can be marked Stable only after the required reproducible validation exists.

Suggested status vocabulary:

- Stable;
- Experimental;
- Research;
- Unverified;
- Blocked;
- Mock.

---

## 33. Definition of done

A feature is complete only when applicable:

- ownership is clear;
- existing abstractions are reused;
- real implementation exists;
- errors are handled;
- unknown states are preserved;
- tests exist;
- documentation exists;
- provenance exists;
- post-change verification exists;
- rollback is implemented or explicitly declared impossible;
- build and CI pass;
- hardware/runtime dependencies are documented.

Final implementation reports must state:

- files created;
- files changed;
- existing interfaces reused;
- adapters added;
- real functionality;
- mock functionality;
- build result;
- test result;
- warnings;
- untested hardware/OS cases;
- remaining dependencies.

---

## 34. Roadmap

The consolidated direction is:

### Stage 0 — Foundation
CAL, common contracts, diagnostics, verification, manifests, hardware knowledge.

### Stage 1 — Provider evidence
Real NVML/CUDA/Vulkan/OS observations with provenance and correct unavailable states.

### Stage 2 — Capability engine
Reconcile hardware, base-driver and ISB capability layers.

### Stage 3 — Telemetry
Live V100 telemetry and bounded history.

### Stage 4 — Safe tuning
Provider-backed read/write/read-back verification.

### Stage 5 — FixEngine
Deterministic compatibility rules and dry-run/apply/rollback.

### Stage 6 — Optimize V100
SCAN → DETECT → ANALYZE → PLAN → REVIEW → APPLY → VERIFY.

### Stage 7 — Driver lifecycle
Discovery, URL parsing, download, verification, cache and explicit installation boundary.

### Stage 8 — Games
Discovery and compatibility state.

### Stage 9 — OptiScaler
External integration with backup, verification and rollback.

### Stage 10 — Graphics/render path
Headless/multi-GPU routing and software enhancement.

### Stage 11 — Reports/benchmarks
Evidence bundles and reproducible performance measurement.

### Stage 12 — CLI
Full Control Center-quality terminal UX.

### Stage 13 — Qt Control Center
Full Hub-backed GUI.

### Stage 14 — Installer/release
Linux packaging and verified GitHub Releases.

### Stage 15 — Pre-test freeze
No new product features; stabilize.

### Stage 16 — Test gate
Clean build, tests, static checks, documentation and regression validation.

### Stage 17 — Physical V100 qualification
Real V100 evidence across the supported hardware/OS matrix.

---

## 35. Source-of-truth rules

For **implementation state**, use this precedence:

1. source code;
2. CI/test evidence;
3. `docs/CURRENT_IMPLEMENTATION.md`;
4. `docs/TASKS.md` / `docs/IMPLEMENTATION_PLAN.md`;
5. historical audit documents.

For **product intent**, use:

1. this consolidated specification;
2. `docs/PRODUCT_REQUIREMENTS.md`;
3. `README.md`.

When documents conflict with source, do not guess. Inspect the code, update the documentation and record the reason for the change.

---

## 36. Related documents

- `README.md` — project identity and global boundaries.
- `docs/AGENT_GUIDE.md` — operational rules for coding agents.
- `docs/CURRENT_IMPLEMENTATION.md` — conservative implementation snapshot.
- `docs/ARCHITECTURE.md` — ownership and dependency architecture.
- `docs/IMPLEMENTATION_PLAN.md` — implementation sequence.
- `docs/TASKS.md` — stabilization and task roadmap.
- `docs/COMPONENT_TASKS.md` — component-level work.
- `docs/DRIVER_AND_RELEASE_PIPELINE.md` — driver lifecycle and release engineering.
- `docs/RENDER_PATH.md` — render-path requirements.
- `docs/UPSTREAM_INTEGRATION.md` — external integration/provenance rules.

This document is the consolidated specification so that an AI agent can understand the complete intended ISB V100 product without relying on scattered task descriptions alone.
### 19.1 V100 software upscaling / reconstruction boundary

V100 graphics enhancement must distinguish hardware capability from software compatibility.

The V100's Tensor Cores and CUDA compute resources can be used by software reconstruction/upscaling implementations, but this does **not** make the V100 an RTX/DLSS hardware device. ISB may therefore manage external or software upscaling paths such as FSR, XeSS and OptiScaler when the particular game, API, integration and external component support them.

For ISB capability reporting:

- FSR/XeSS/OptiScaler support is an **external/application/software compatibility capability**, not a V100 hardware feature;
- availability must be detected per game/API/integration rather than assumed globally;
- native DLSS hardware must remain unavailable on V100;
- native RT cores and dedicated Optical Flow hardware must remain unavailable;
- successful software upscaling must be reported separately from native NVIDIA hardware acceleration.

The project must not turn a third-party database or a GPU specification site's "supports FSR/XeSS" label into proof that every game supports the technology. Game-specific verification remains authoritative.


