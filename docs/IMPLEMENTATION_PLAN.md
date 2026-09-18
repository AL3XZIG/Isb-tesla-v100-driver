# ISB V100 Hub — Implementation Plan From Current State

## Evidence-based architecture gate

Do not infer ownership from directory names or old specifications. The current repository must be audited before architectural migration.

Confirmed baseline:
- `core/` exists and contains `core/TASK.md`.
- `core/TASK.md` specifies provider-neutral orchestration/runtime state and explicitly does not add production implementation by itself.
- `common/` contains the implemented `Status`, `Result<T>` and `ErrorCode` primitives used by current code.
- The existence of `core/TASK.md` does not prove that `core` owns foundation primitives.
- Whether the repository actually implements separate driver and user-control planes must be established from code and dependency ownership, not assumed.

### Stabilization rule

The first task is an ownership audit. Do not perform a blind `common → core` migration.

The intended user-facing boundary is:

`Panel → Hub → Providers`

Hub is the user-space control plane. Do not introduce a second `ControlPlane` abstraction or an `isb-contracts` library unless a concrete wire boundary/second consumer later requires it.

If the repository audit confirms a separate low-level driver plane, document it as a distinct plane with explicit ownership rules. Otherwise treat `core/TASK.md` as a specification that may be superseded.


This plan starts from the current `main` state. It is intentionally **pre-test**: implementation and static/code review tasks come first. Do not start hardware testing or declare a feature stable until the final test gate is reached.

## Current baseline

The repository already contains:

- CAL v1 capability contracts;
- V100/GV100 capability definitions;
- common contracts and an optional NVML provider;
- headless Hub/CLI foundation;
- FixEngine foundations;
- diagnostics/verification foundations;
- experimental graphics, compute and benchmark layers;
- optional Qt6 Control Center foundation;
- root CMake component registration cleaned up;
- deterministic/mock control-plane behavior.

The current Control Center is still a contract/mock layer: real provider-backed telemetry and mutation are not complete. The GUI must not become the place where hardware/API logic is implemented.

## Non-negotiable architecture

```text
CLI / Qt Control Center
        |
        v
   Hub / Control Plane
        |
  +-----+-----+----------------+
  |           |                |
Telemetry   Tuning        Optimization
  |           |                |
  +-----------+----------------+
              |
        Provider Layer
              |
   NVML / CUDA / Vulkan / DXGI
              |
       Installed base driver
              |
           V100/GV100
```

Rules:

1. GUI and CLI use the same Hub contracts.
2. GUI never calls NVML/CUDA/Vulkan/DXGI directly.
3. Hardware facts, driver/API capabilities and ISB-added capabilities are separate.
4. `Unknown` is never silently converted to `Unavailable` or `Available`.
5. No mutation without an explicit user-approved operation.
6. Every mutation follows `requested -> applied -> read-back -> verified`.
7. Unsupported operations must be safe no-ops with explicit status.
8. Provider failures must preserve error/provenance information.
9. External components such as OptiScaler remain external and are never silently bundled.
10. Experimental functionality stays behind explicit experimental boundaries.

---

# Phase 0 — Repository and build integrity

## 0.1 Audit target graph

Review every current `CMakeLists.txt` participating in the default build.

Required actions:

- remove duplicate target names;
- remove duplicate `add_subdirectory()` registrations;
- ensure every linked target is defined before use;
- ensure optional dependencies are genuinely optional;
- ensure tests are only created when `BUILD_TESTING` is enabled;
- ensure include directories are correct for a clean checkout;
- ensure no module depends accidentally on another experimental module;
- ensure root CMake does not silently hide a required core module.

## 0.2 Establish component classification

Classification must follow the actual repository and dependency graph. Do not label `core/` as the foundation owner until #29 establishes that.

Classify modules as:

- **stable/core** — `common`, `cal`, `hub`, `cli`, core control-plane contracts;
- **provider** — NVML/CUDA/Vulkan/DXGI/platform adapters;
- **experimental** — graphics/compute/research code;
- **frontend** — `control-center`;
- **evidence** — diagnostics/verification/reports/benchmarks.

Do not create duplicate implementations merely to satisfy the target directory diagram.

## 0.3 Clean public API boundaries

For each public header:

- include what it uses;
- avoid accidental transitive includes;
- use explicit namespaces;
- use RAII for resources;
- avoid globals and hidden singletons;
- avoid platform-specific types in portable contracts;
- use stable enums for machine-readable state;
- keep serialization deterministic.

---

# Phase 1 — Foundation ownership and Hub control-plane consolidation

## 1.1 Resolve foundation ownership

Audit `core/TASK.md`, production `core/` code, `common/` headers and all consumers of `Status`, `Result<T>`, `ErrorCode` and `Provenance`.

Choose one explicit owner per primitive. If `core/TASK.md` is obsolete, mark it superseded. Do not create a third compatibility layer.

## 1.2 Unify duplicated models

Create or reconcile common contracts for:

- `Environment`;
- `GpuIdentity`;
- `CapabilitySnapshot`;
- `ErrorEvent`;
- `Operation`;
- `VerificationResult`;
- `Provenance`.

Before adding a new struct, search the repository for an existing equivalent and reuse or adapt it.

Do not maintain two incompatible representations of the same concept.

## 1.3 Standardize capability state

Use one semantic model throughout the Hub:

- `AVAILABLE`;
- `UNAVAILABLE`;
- `UNKNOWN`;
- where operationally required, distinguish `PERMISSION_DENIED` and `ERROR` from capability itself.

A permission error is not proof that hardware is unsupported.

## 1.4 Complete operation model

Implement a common operation record containing at minimum:

- operation id;
- operation type;
- target GPU/application;
- requested values;
- planned changes;
- risk level;
- privilege requirement;
- restart/reset requirement;
- current state;
- applied state;
- read-back state;
- verification state;
- rollback information;
- timestamp/provenance.

The model must support:

`PLAN -> USER REVIEW -> APPLY -> READ BACK -> VERIFY -> RESULT`.

## 1.5 Complete deterministic serialization

All externally persisted contracts must have deterministic serialization.

Fix the current Control Center JSON helper so that JSON strings correctly escape at least:

- quotation marks;
- backslashes;
- newline;
- carriage return;
- tab;
- backspace;
- form feed;
- other required control characters.

Prefer a single project JSON utility instead of hand-written serializers scattered through modules.

---

## 1.6 Consolidate the user-control entry point

Audit current `control-center::ControlPlane` and all Control Center call sites, including Qt `main.cpp`, src, include and tests.

Target boundary:

`Panel → Hub → Providers`

Panel must not include or link provider implementations. Hub receives providers through injection. MockProvider is used below the real Hub path.

Only after this consolidation should observation contracts be unified.

# Phase 2 — Real read-only provider layer

This is the most important implementation phase before tuning or GUI completion.

## 2.1 Observation contract audit

Search repository-wide for `Observed<T>` and equivalent wrappers. Confirm actual duplicates before creating a common abstraction. Generic observation semantics may be shared; provider-specific raw observation payloads remain provider-specific.

## 2.2 NVML provider

Implement a production read-only NVML adapter behind the existing provider contract.

Collect where exposed:

- GPU name;
- UUID/index;
- PCI bus identity;
- memory total/used/free;
- temperature;
- power draw;
- power limit;
- SM clock;
- memory clock;
- utilization;
- ECC state and counters;
- persistence mode;
- compute mode;
- process count/process information where permissions allow;
- PCIe generation/width/throughput where exposed;
- NVLink state/topology where exposed.

Every field must carry availability/provenance state.

## 2.3 Exact V100 identification

Detect rather than assume:

- GV100/Volta;
- V100 SXM2;
- V100 PCIe;
- 16 GB vs 32 GB where reliable.

Do not infer variant solely from a user-selected profile.

## 2.4 CUDA provider

Add read-only CUDA runtime/driver probing:

- CUDA driver version;
- runtime version where available;
- compute capability;
- device attributes;
- Tensor Core-relevant hardware attributes;
- P2P capability;
- device visibility/access errors.

CUDA absence must be represented as a detected environment state, not as a compile-time assumption.

## 2.5 Vulkan provider

Implement read-only Vulkan probing where available:

- loader availability;
- instance version;
- physical-device enumeration;
- device/API properties;
- extensions;
- relevant feature flags;
- provenance.

No Vulkan feature may be claimed from V100 hardware alone.

## 2.6 Windows graphics provider

Implement a platform boundary for Windows:

- DXGI adapter identity;
- D3D feature information where safely available;
- DirectCompute availability where actually exposed;
- driver version/provider information;
- adapter memory information where exposed.

Never hard-code a particular NVIDIA/Google driver version as universally compatible.

## 2.7 Linux provider boundary

Provide Linux-specific environment detection without polluting portable contracts:

- kernel version;
- loaded NVIDIA driver information;
- device node/access state;
- relevant library versions;
- permissions.

---

# Phase 3 — Capability engine

## 3.1 Separate capability layers

For each feature record:

```text
hardware capability
base-driver capability
runtime/library capability
ISB capability
verification status
```

Examples:

- Tensor Cores: hardware available on GV100;
- DLSS: not native V100 hardware capability;
- RT Cores: unavailable on V100;
- Optical Flow hardware: unavailable on V100;
- MIG: unavailable on V100.

Software alternatives must be represented separately from native hardware support.

## 3.2 Capability-aware gating

Create reusable predicates for controls/features.

A control is editable only when:

1. hardware/feature is applicable;
2. provider supports the operation;
3. permissions are sufficient;
4. required runtime/API exists;
5. current state is sufficiently known.

Unknown must result in a disabled/indeterminate action, not a speculative action.

## 3.3 Capability provenance

Record source for each observation:

- NVML;
- CUDA;
- Vulkan;
- DXGI/D3D;
- OS;
- configuration;
- benchmark;
- synthetic fixture.

Synthetic data must be explicitly marked `synthetic: true`.

---

# Phase 4 — Telemetry and state model

## 4.1 Complete telemetry contract

Normalize provider results into the Hub telemetry snapshot.

Fields should include:

- utilization;
- temperature;
- power draw;
- power limit;
- SM clock;
- memory clock;
- HBM usage;
- ECC;
- PCIe;
- NVLink;
- process count;
- driver/runtime status.

Every value needs state/provenance metadata.

## 4.2 Time-series support

Add optional sampled telemetry history for:

- temperature;
- utilization;
- power;
- clocks;
- throttling indicators.

Do not make an always-on daemon mandatory for the first version.

## 4.3 Thermal Guard

Derive, where evidence supports it:

- thermal throttling;
- power throttling;
- clock drops;
- abnormal ECC behavior.

Never diagnose a cause when telemetry only establishes correlation.

---

# Phase 5 — Tuning backend

Only implement controls after read-only provider paths are reliable.

## 5.1 Control descriptors

Each control must expose:

- id;
- name;
- type;
- capability state;
- current value;
- desired value;
- valid range/options;
- permission requirement;
- persistence semantics;
- restart/reset requirement;
- reversibility;
- provider source.

## 5.2 Implement supported controls

Provider-backed operations, where the installed driver exposes them:

- persistence mode;
- power limit;
- application clocks;
- compute mode;
- other V100-supported management controls discovered by the provider.

Do not invent a control because a UI element exists.

## 5.3 Apply transaction

Implement:

```text
READ CURRENT
  -> VALIDATE REQUEST
  -> CREATE PLAN
  -> USER APPROVAL
  -> APPLY ONE STEP
  -> READ BACK
  -> VERIFY
  -> NEXT STEP
```

Default failure policy for hardware tuning: **StopOnFailure**.

## 5.4 Rollback

For reversible settings:

- capture original state;
- apply change;
- verify;
- restore previous state if requested or required after failure.

For non-reversible operations, explicitly mark rollback unavailable.

## 5.5 Profiles

Profiles remain policy templates:

- Balanced;
- Gaming;
- Compute;
- AI/Tensor;
- Maximum Performance;
- Low Power;
- Custom.

A profile does not mean “known optimal”. It produces desired policy values only when provider evidence permits.

Selecting a profile must not silently mutate hardware.

---

# Phase 6 — FixEngine / Driver Doctor integration

## 6.1 Integrate existing FixEngine

Do not create a second diagnostic/rule engine.

Connect:

```text
fingerprint -> environment/error evidence -> FixEngine -> plan -> executor -> verification
```

## 6.2 Preserve tri-state logic

Fix matching must distinguish:

- predicate false;
- predicate unknown;
- predicate true.

A false match may be skipped.
An unknown match must remain unknown and must not become a planned action.

## 6.3 Stable diagnostics

Standardize error/reason codes.

Each diagnosis must include:

- observed evidence;
- matched rule;
- confidence/state;
- proposed action;
- required privilege;
- rollback path;
- verification method.

## 6.4 Compatibility database

Add machine-readable records for:

- GPU;
- driver version/range;
- OS;
- API;
- application;
- known issue;
- workaround;
- maturity;
- provenance.

No undocumented “magic” fixes.

---

# Phase 7 — Optimization engine

## 7.1 Complete Optimize V100 planner

Pipeline:

```text
SCAN
DETECT
ANALYZE
PLAN
USER REVIEW
APPLY
VERIFY
RESULT
```

Planner output must distinguish:

- applicable;
- already satisfied;
- unsupported;
- unknown;
- permission denied;
- risky;
- requires restart;
- experimental.

## 7.2 Application profiles

Define a stable schema containing:

- executable/path;
- application id/name;
- graphics API;
- selected profile;
- upscaler;
- frame-generation mode;
- compatibility state;
- anti-cheat state;
- modifications;
- backup/rollback information.

## 7.3 No launcher responsibilities

Optimization is not a game launcher.

Keep discovery, compatibility, planning and configuration separate from process launching.

---

# Phase 8 — Game compatibility layer

## 8.1 Discovery providers

Implement adapters for:

- Steam;
- Epic;
- GOG;
- standalone installations.

Discovery must be best-effort and must never fail the whole Hub when one source is absent.

## 8.2 Graphics API detection

Detect where possible:

- DX11;
- DX12;
- Vulkan;
- relevant runtime/upscaler signals.

Unknown is a valid result.

## 8.3 Anti-cheat safety

Represent:

- PRESENT;
- ABSENT;
- UNKNOWN.

Unknown defaults to manual review/dry-run.

Never silently patch an online or anti-cheat executable.

---

# Phase 9 — OptiScaler manager

OptiScaler remains an external component.

## 9.1 Manager contracts

Implement:

- installed detection;
- version detection;
- provenance;
- compatibility evaluation;
- install;
- update;
- remove;
- rollback;
- backup;
- hash verification;
- per-game configuration.

## 9.2 Safety

Before modifying an application:

1. confirm target path;
2. create backup;
3. record hashes;
4. require explicit approval;
5. perform operation;
6. verify files;
7. record result;
8. provide rollback.

Do not silently download arbitrary binaries.

## 9.3 Licensing

Store source/license/SPDX/provenance metadata.

Do not copy OptiScaler implementation into ISB.

---

# Phase 10 — Graphics backend

## 10.1 Stable graphics contracts

Define backend-independent interfaces for:

- render scale;
- resolution scaling;
- sharpening;
- DRS;
- VSync/FPS settings where safe;
- frame-pacing telemetry;
- external upscaler integrations.

## 10.2 Upscaling states

Represent explicitly:

- Native;
- DLSS compatibility path where applicable;
- FSR;
- XeSS;
- OptiScaler;
- experimental ISB software reconstruction.

Never label software reconstruction as native DLSS.

## 10.3 Prefer supported mechanisms

Priority:

1. documented application configuration;
2. documented loader/layer/plugin mechanism;
3. external compatibility component;
4. experimental interception only when isolated and reversible;
5. binary patching only as an explicitly experimental last resort.

---

# Phase 11 — Reports and evidence

## 11.1 Versioned report schema

Implement deterministic report bundle:

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

Include:

- schema version;
- ISB version/commit;
- generation timestamp;
- environment hash;
- exact GPU identity;
- driver/runtime provenance;
- operation history;
- synthetic/real evidence markers.

## 11.2 Baseline/candidate comparison

Support diffing:

```text
baseline -> candidate -> verified result
```

Performance claims must reference reproducible evidence.

---

# Phase 12 — CLI completion

The CLI must expose the same operations as the Hub contracts.

Minimum commands/concepts:

- inspect/status;
- capabilities;
- telemetry;
- profile list/show;
- optimize plan;
- diagnose;
- report;
- verify;
- provider information.

Mutation commands must have explicit apply semantics.

The CLI must never duplicate backend logic.

---

# Phase 13 — Control Center GUI completion

Only start this phase after the Hub/provider contracts above are stable.

## Layout

Persistent narrow sidebar:

1. `GPU`
2. `TUNING`
3. `OPTIMIZATION`
4. `GRAPHICS`

Global compact status header:

- GPU;
- temperature;
- utilization;
- power;
- clocks;
- driver state.

Avoid huge cards, giant buttons and dashboard clutter.

## GPU page

Display:

- exact identity;
- architecture;
- variant;
- memory;
- ECC;
- TDP/power;
- PCIe;
- NVLink;
- Tensor Cores;
- API/runtime state;
- current telemetry.

Use `Available / Unsupported / Unknown / Permission denied / Error` consistently.

## TUNING page

Display only controls supported by current provider evidence.

Use:

`Requested -> Plan -> Review -> Apply -> Read Back -> Verify`.

## OPTIMIZATION page

Separate:

- Games;
- Applications.

Show:

- executable;
- API;
- profile;
- upscaler;
- OptiScaler state;
- compatibility;
- optimization plan.

## GRAPHICS page

Expose only capabilities actually available through the selected application/backend.

Native hardware support and software compatibility paths must be visually distinct.

---

# Phase 14 — Installer / deployment

After the backend and GUI are functional, implement deployment.

## Windows

Provide:

- prerequisite detection;
- install location;
- provider/runtime detection;
- configuration;
- rollback/uninstall;
- logs.

## Linux

Provide:

- package/install instructions;
- dependency detection;
- NVIDIA driver/runtime detection;
- permissions guidance;
- uninstall/rollback.

The installer must never replace or overwrite the NVIDIA base driver automatically.

---

# Phase 15 — Architecture and documentation synchronization

Before code freeze:
- document verified `core`/ `common` ownership;
- document whether multiple architectural planes are actually implemented;
- document `Panel → Hub → Providers`;
- document Hub as the only user-space control plane;
- mark superseded specifications;
- synchronize README, TASKS, IMPLEMENTATION_PLAN and GUIDE.

# Phase 16 — Pre-test code freeze

Before any hardware qualification:

- remove dead code introduced during migration;
- remove duplicate models;
- remove duplicate serializers;
- ensure warnings are understood;
- ensure all public headers are self-contained;
- ensure no GUI-only implementation of backend logic;
- ensure mock fixtures are explicitly synthetic;
- ensure experimental code cannot silently execute in stable paths;
- ensure no operation reports success without read-back/verification;
- ensure report schema is versioned;
- ensure CMake options are documented.

Only after this phase is complete may the project enter the test phase.

---

# Final test gate — starts only after implementation and stabilization phases

## Clean build

```bash
rm -rf build
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
```

## Static hygiene

```bash
git diff --check
```

If available, additionally run the project's configured compiler warnings/static analysis.

## First hardware qualification

Only then test on the real Tesla V100 environment.

Record:

- exact V100 variant;
- OS/kernel;
- NVIDIA driver;
- CUDA;
- NVML;
- Vulkan/D3D state;
- firmware/BIOS information where relevant;
- baseline telemetry;
- operation results;
- verification results;
- report bundle.

## Stability rule

A feature is not called stable merely because it builds. It requires:

1. real capability detection;
2. positive path;
3. negative/unsupported path;
4. unknown/error path;
5. provenance;
6. deterministic behavior;
7. rollback or explicit non-reversible declaration;
8. post-operation verification;
9. report/log coverage;
10. no false V100 hardware claims.

---

# Recommended execution order

```text
0  Build/CMake integrity + #29 foundation/architecture audit
1  #31 Hub canonical control API
2  #30 Observation contract consolidation
3  #32 Capability ownership/engine
4  Real read-only providers
5  Telemetry
6  Tuning backend
7  FixEngine integration
8  Optimization engine
9  Game compatibility
10 OptiScaler manager
11 Graphics backend
12 Reports/evidence
13 CLI completion
14 Qt Control Center
15 Installer/deployment
16 #37 Architecture/docs synchronization
17 Pre-test code freeze
18 TEST GATE
19 Real V100 qualification
```

The key principle is: **audit ownership first, make Hub the single user-control entry point, consolidate observations second, then build features. Hardware qualification starts only after the implementation and test gates.**
