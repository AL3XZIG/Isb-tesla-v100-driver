# ISB V100 Hub Architecture

## 1. Purpose

ISB is a **V100-focused user-space control, compatibility, optimization and diagnostics hub** built above an installed NVIDIA driver stack.

The MVP does not require an independent NVIDIA kernel-mode or user-mode driver. The installed base stack remains responsible for native OS driver integration, device initialization, command submission and hardware access. ISB adds a controlled user-space control plane, compatibility layer, diagnostics, optimization workflows and external integrations.

The independent KMD/UMD effort remains isolated under `research/alternative-driver/` and is never a prerequisite for the Hub.

## 2. Product model

The product is one lightweight control plane with two frontends:

```text
                 User
                  |
        +---------+---------+
        |                   |
   Control Center          CLI
        |                   |
        +---------+---------+
                  |
             ISB Hub API
                  |
      +-----------+-----------+
      |           |           |
   Diagnose    Optimize     Manage
      |           |           |
   FixEngine  Performance  Games/OptiScaler
      |           |           |
      +-----------+-----------+
                  |
           Provider Layer
     NVML / CUDA / Vulkan / DXGI
                  |
          Base Driver Stack
                  |
             V100 / GV100
```

The GUI must not contain a second implementation of the business logic. The CLI and GUI consume the same hub contracts and produce the same structured audit/provenance records.

## 3. Core invariants

1. ISB is not the NVIDIA base driver.
2. Hardware capability, base-driver capability and ISB-added capability are separate.
3. Unknown is not available.
4. Normal application paths remain untouched unless a rule/profile requires intervention.
5. Mutating operations are explicit, logged and reversible where technically possible.
6. Every claimed improvement requires measurement and provenance.
7. External components remain replaceable and license-audited.
8. The GUI never duplicates hub logic.
9. V100-specific behavior is preferred over an unnecessarily generic GPU abstraction.
10. The alternative-driver research track remains isolated.

## 4. Capability layers

### 4.1 Hardware capability

What the physical GPU contains.

For V100/GV100:

- Volta / SM70;
- first-generation Tensor Cores;
- ECC HBM2;
- no RT Cores;
- no dedicated Optical Flow Accelerator;
- no MIG;
- no display outputs on the V100 itself.

SXM2/PCIe and 16/32 GB variants must be detected explicitly.

### 4.2 Base-driver capability

What the installed driver/runtime exposes on the current OS and API.

Examples:

- CUDA and runtime version;
- NVML management controls;
- Vulkan extensions;
- OpenGL;
- Direct3D/DXGI;
- DirectCompute;
- driver-specific feature paths.

A capability exposed by a particular NVIDIA/Google package is recorded as a **base-driver capability** with provenance. ISB does not claim ownership of that implementation.

### 4.3 ISB capability

Functionality added by the Hub:

- optimization planning;
- performance profiles;
- compatibility profiles;
- game discovery;
- OptiScaler orchestration;
- FixEngine workarounds;
- diagnostics and reports;
- verification/regression workflows;
- user-space neural/graphics experiments;
- telemetry and benchmark presentation.

## 5. Hub control plane

`hub/` is the orchestration boundary for user-facing operations. It should remain thin: it coordinates modules and providers but does not become a second implementation of CAL, FixEngine or provider logic.

The Hub exposes stable operations such as:

```text
scan
inspect
status
optimize
profile
apply
rollback
verify
benchmark
report
```

A typical optimization transaction is:

```text
SCAN
  -> DETECT
  -> CAPABILITY CHECK
  -> DIAGNOSE
  -> BUILD PLAN
  -> DRY-RUN
  -> USER APPROVAL
  -> APPLY
  -> VERIFY
  -> LOG / REPORT
```

Failure during or after APPLY must produce a rollback path whenever the affected operation is reversible.

## 6. Performance subsystem

`performance/` owns V100 telemetry and management operations.

Read-only data should include, where exposed:

- utilization;
- temperature;
- power draw and limit;
- SM/memory clocks;
- PCIe traffic/link state;
- ECC state/errors;
- process list;
- throttling reasons;
- NVLink state/topology where present.

Write operations may include, only when the base management API supports them and permissions allow:

- persistence mode;
- application clocks;
- power limit;
- compute mode;
- supported workload/power profiles.

Every mutable setting has a lifecycle:

```text
requested -> applied -> verified
```

The result must also identify whether it is temporary, persistent, reset-dependent or restart-dependent.

## 7. Game subsystem

`games/` owns discovery and compatibility state.

Discovery should support, where detectable:

- Steam;
- Epic;
- GOG;
- standalone installations.

A game record can contain:

- title/id;
- installation path;
- executable;
- graphics API;
- upscaler/runtime signals;
- anti-cheat state;
- compatibility state;
- selected profile;
- managed files and backups.

Compatibility states are explicitly tri-state or richer. Unknown must never become an implicit install permission.

Bulk operations use:

```text
SCAN -> DETECT -> COMPATIBILITY -> DRY-RUN -> APPROVAL
     -> INSTALL -> VERIFY -> LOG -> ROLLBACK
```

Online/anti-cheat applications require conservative handling; unknown anti-cheat state defaults to dry-run/manual approval.

## 8. OptiScaler integration

OptiScaler is an external managed component under `integrations/`.

ISB may:

- detect installed versions;
- select a compatible version;
- configure per-game profiles;
- install/update/rollback supported integration files;
- back up and hash files;
- verify installation;
- maintain provenance/license metadata.

ISB must not silently download arbitrary binaries or reimplement/copy OptiScaler source into unrelated ISB modules. Distribution must comply with the external project's license and source obligations.

## 9. FixEngine integration

The existing FixEngine remains the single rule/plan engine.

The Hub should consume it rather than introduce a second compatibility-rule system:

```text
Environment + ErrorEvent + Capabilities
                 |
             FixEngine
                 |
          Proposed Fix Plan
                 |
           Hub transaction
        /         |          \
    dry-run      apply      rollback
        |         |            |
        +---------+------------+
                  |
               verify
```

Rules must remain deterministic and explainable. The Hub owns user-facing orchestration; FixEngine owns rule matching and action planning.

## 10. Diagnostics and evidence

`diagnostics/` and `verification/` are evidence infrastructure.

A report should distinguish:

- observed facts;
- detected capabilities;
- applied changes;
- verification results;
- benchmark measurements;
- hypotheses or experimental features.

A deterministic report bundle should use a versioned manifest and may contain:

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

Synthetic fixtures are explicitly marked synthetic and cannot be presented as hardware evidence.

## 11. Benchmarks

`benchmarks/` measures effects of profiles, workarounds and integrations.

Initial classes:

- graphics FPS/frame-time and 1% low;
- CUDA compute;
- Tensor Core throughput;
- HBM bandwidth;
- PCIe/interconnect;
- application-level before/after tests.

Correctness must be checked before performance claims. Results identify exact hardware, driver, workload, configuration and revision.

## 12. Graphics and neural features

`graphics/` contains API/graphics compatibility infrastructure.

`neural/` contains V100-specific neural reconstruction, denoising and related experiments.

External upscalers such as OptiScaler are orchestrated through `integrations/`, not reimplemented inside `neural/`.

V100 has no RT Cores or dedicated Optical Flow Accelerator. Software reconstruction/frame-generation paths must therefore be labeled according to their actual implementation rather than described as native V100 hardware features.

## 13. Profiles

`profiles/` stores declarative configuration for:

- V100 hardware variants;
- base drivers;
- workload modes;
- games/applications;
- graphics/OptiScaler settings;
- performance preferences.

Profiles describe desired state. Providers and the Hub determine whether that state can actually be applied and verified.

## 14. Provider layer

`providers/` owns platform/runtime-specific access.

Likely providers include:

- NVML;
- CUDA;
- Vulkan;
- DXGI/D3D;
- Windows platform APIs;
- Linux platform APIs.

Providers report unavailable/unknown states instead of manufacturing capabilities.

## 15. Repository structure

```text
core/                  shared low-level state and contracts
cal/                   capability abstraction
capabilities/           V100/GV100 declarative capability data
providers/              runtime/platform probes and management APIs
fix/                    single FixEngine implementation
hub/                    user-facing orchestration/control plane
games/                  discovery and game compatibility
performance/            telemetry, tuning and throttling analysis
integrations/           OptiScaler, DXVK/VKD3D and external components
benchmarks/             reproducible measurements
diagnostics/            fingerprints, IDR and evidence
verification/           regression and post-change verification
reports/                deterministic report bundles
profiles/               declarative hardware/driver/app/workload profiles
control-center/         lightweight GUI frontend
cli/                    headless frontend
installer/              installation/configuration/rollback
compute/                CUDA/DirectCompute compatibility and compute features
graphics/               graphics compatibility infrastructure
neural/                 V100 neural feature experiments
database/               known drivers, applications and issues
manifests/              reproducibility manifests
drivers/                base-driver metadata/lifecycle
research/               isolated alternative-driver research
third_party/            audited external references
legal/                  licensing and provenance
tests/                  automated tests
docs/                   architecture, tasks and protocols
```

Existing modules should be migrated toward this ownership model incrementally. Do not perform a large mechanical move merely to make the tree look different; preserve working foundations and move code when its new ownership is implemented and tested.

## 16. Control Center

The GUI is a thin presentation layer with tabs/views corresponding to the Hub contracts:

- **Home** — identity, health, driver and issues;
- **Performance** — telemetry and supported tuning;
- **Games** — discovery and per-game compatibility;
- **Tools** — OptiScaler, benchmarks, diagnostics and reports;
- **Optimize V100** — explicit analysis/plan/apply/verify workflow.

The GUI must remain lightweight. A C++ backend and lightweight GUI toolkit are preferred over an Electron-style runtime. No separate always-on service is required for core functionality.

## 17. Driver Manager

`drivers/` manages metadata and lifecycle around external base stacks, not an independent ISB driver.

Responsibilities:

1. detect installed base drivers;
2. fingerprint capabilities;
3. record provenance;
4. expose compatible base-stack information;
5. coordinate configuration/rollback state;
6. verify the resulting runtime.

Proprietary NVIDIA/Google packages remain external inputs unless redistribution rights are established.

## 18. Security and safety

- Mutations are explicit and auditable.
- Administrator/root requirements are declared.
- Backups precede file/config mutations.
- Rollback is first-class.
- Unverified rules do not auto-apply in stable mode.
- External binaries are separately sourced and verified.
- Binary patching of proprietary drivers is not a default mechanism.
- Game integration must account for anti-cheat and online safety.

## 19. Full alternative-driver track

The independent driver remains separate:

```text
Mainline: ISB V100 Hub
  control plane
  compatibility
  optimization
  integrations
  diagnostics
  verification

Research: independent driver
  KMD / UMD
  GPUVM
  command submission
  graphics / compute runtime
```

Research code must never become an accidental runtime dependency of the Hub.

## Current implementation boundary

The build currently contains CAL plus a small `hub/` orchestration library and `cli/` frontend. `MockProvider` supplies explicitly synthetic V100 fixture evidence; `UnavailableProvider` preserves unknown states when optional NVML/CUDA/Vulkan/OS providers are absent. The CLI delegates to `Hub`; it owns no hardware business logic. Mutation is deliberately unavailable until a provider can apply and read back a setting.
