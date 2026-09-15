# ISB Driver Fixer Architecture

## 1. Purpose

ISB is a **driver compatibility, diagnostics, workaround and extension layer**. The first hardware target is NVIDIA Tesla V100 / GV100.

The MVP intentionally does **not** require an independent NVIDIA kernel-mode driver. An installed NVIDIA-compatible base stack remains responsible for the native OS driver model, device initialization, command submission and hardware access. ISB adds a controlled user-space layer around that stack.

The long-term independent KMD/UMD effort remains under `research/alternative-driver/` and is not a prerequisite for the Fixer.

## 2. Core model

```text
 Application / Game / Professional Software
                    |
                    v
          ISB Compatibility Layer
                    |
                    v
             ISB Fix Engine
        +-----------+-----------+
        |           |           |
     Detect      Diagnose    Policy/Rules
        |           |           |
        +-----------+-----------+
                    |
          Workaround / Shim /
          Configuration / Profile
                    |
                    v
             Base Driver Stack
                    |
          Windows WDDM / Linux
          native driver model
                    |
                    v
              V100 / GV100
```

The key invariant is that ISB should intervene only where there is a reason to do so. Normal paths should continue through the base driver unchanged.

## 3. Three capability layers

ISB must never conflate these three things:

### 3.1 Hardware capability

What the physical GPU contains or supports.

Example for V100:

- GV100 / Volta / SM70;
- first-generation Tensor Cores;
- HBM2 with ECC support;
- no RT Cores;
- no dedicated Optical Flow Accelerator;
- no MIG;
- no display outputs on the V100 itself.

### 3.2 Base-driver capability

What the currently installed driver/runtime actually exposes on the selected OS and API.

Examples:

- CUDA availability/version;
- Vulkan extensions;
- OpenGL support;
- Direct3D/DXGI support;
- DirectCompute availability;
- management interfaces;
- driver-specific feature paths.

A capability exposed by a Google Compute Engine / vGPU-class package, for example, is a **base-driver capability** until ISB independently implements an equivalent feature.

### 3.3 ISB-added capability

Functionality implemented by ISB above the base stack.

Examples:

- automatic workaround selection;
- application profiles;
- API configuration;
- compatibility shims;
- external upscaler orchestration;
- diagnostics and regression verification;
- user-space neural reconstruction;
- custom monitoring/control features.

This separation is fundamental to the project's credibility.

## 4. Driver-base abstraction

The installed driver is represented as a `BaseDriverProfile` containing at least:

- vendor;
- package/version;
- OS/platform;
- GPU model and exact variant;
- driver branch/type where known;
- exposed APIs;
- exposed extensions/features;
- provenance;
- verification state.

ISB does not assume that two packages with the same NVIDIA GPU support the same software interfaces.

A base-driver profile can therefore say, for example:

```yaml
gpu:
  architecture: Volta
  model: Tesla V100
  variant: V100_SXM2
base_driver:
  vendor: NVIDIA
  family: google_compute_engine
  version: "<observed-version>"
  capabilities:
    directcompute:
      state: available
      provenance: runtime_probe
```

The actual version and capability state must come from a probe or a verified manifest; placeholders must never be presented as facts.

## 5. Fix Engine

The Fix Engine evaluates the current environment against deterministic rules.

```text
Environment + Evidence
          |
          v
       Rule Match
          |
   +------+------+
   |             |
 no match      match
   |             |
 normal       workaround
 path         plan
```

A rule contains:

- stable identifier;
- affected GPU/variant;
- OS;
- API/runtime;
- driver range or feature fingerprint;
- trigger/condition;
- evidence reference;
- workaround action;
- verification procedure;
- rollback action;
- maturity (`stable`, `experimental`, `research`).

Rules must be deterministic and explainable. The engine must report **why** a workaround was selected.

## 6. Workaround actions

Initial action types should be user-space and reversible:

- set/unset runtime environment variables;
- select a validated API/feature path;
- enable/disable a problematic extension path where the application permits it;
- select a compatibility profile;
- configure DXVK/VKD3D/OptiScaler or another external component;
- replace a known-broken user-space component when the replacement is independently supplied and compatible;
- enable an ISB interception shim for a documented API boundary;
- collect diagnostics and stop rather than applying an unsafe fix.

No rule may claim to repair kernel/hardware behavior unless the selected mechanism actually provides such control.

## 7. Runtime interception

The preferred interception model is narrow rather than global:

```text
Application
    |
    v
ISB Shim
  /     \
normal   affected operation
  |           |
  v           v
NVIDIA     workaround
  |           |
  +-----+-----+
        |
        v
       GPU
```

Interception should be API-specific and opt-in during development. Every shim requires tests proving that normal calls are preserved and that the workaround does not alter unrelated behavior.

Possible boundaries include:

- Vulkan loader/layer interfaces;
- Direct3D/DXGI user-space compatibility paths;
- CUDA runtime-facing configuration or wrappers;
- OpenGL user-space integration where technically and legally appropriate;
- external upscaler/interception frameworks.

ISB should prefer documented loader/plugin/layer mechanisms over binary patching.

## 8. DirectCompute and feature extension strategy

DirectCompute is an important example of the new project's purpose.

If a particular base NVIDIA driver exposes DirectCompute on V100, ISB should:

1. detect that capability;
2. record the exact driver/API provenance;
3. preserve the working path;
4. provide diagnostics and compatibility policy around it;
5. build custom ISB features on top of the exposed compute path where APIs permit it;
6. regression-test the combination.

ISB must **not** state that it implemented DirectCompute merely because it selected a driver that already exposes it.

This same model applies to Vulkan extensions, CUDA features and other driver-specific functionality.

## 9. Driver compatibility database

The database is the project's central knowledge layer:

```text
database/
├── drivers/
├── gpus/
├── applications/
└── issues/
```

An issue record should contain:

```yaml
id: ISB-DRV-0001
status: confirmed
gpu:
  model: Tesla V100
  variant: V100_SXM2
platform:
  os: windows
api: vulkan
base_driver:
  version: "<verified-version>"
symptom: "<reproducible symptom>"
trigger:
  type: "<observed condition>"
workaround:
  action: "<reversible action>"
evidence:
  - "<IDR/benchmark/test reference>"
verification:
  status: pass
```

No issue should enter the confirmed database without reproducible evidence.

## 10. Regression matrix

The Fixer must be tested against combinations, not isolated driver versions:

```text
GPU variant × OS × base driver × API × application × ISB rule set
```

For every known issue the harness should be able to report:

- affected combination;
- baseline result;
- workaround result;
- regression status;
- evidence/manifest;
- rollback result.

A workaround that fixes one version but breaks another must be represented as a version-specific rule, not generalized.

## 11. Diagnostics

The existing CAL and Verification Tools remain foundations.

IDR should be extended to capture:

- exact GPU and SXM2/PCIe variant;
- OS and kernel/build;
- base-driver package/version;
- API/runtime versions;
- exposed capabilities;
- ISB version/configuration;
- active workaround IDs;
- environment/configuration;
- relevant errors and timestamps;
- reproduction metadata.

Diagnostics should make a driver bug report reproducible by another machine or test runner where possible.

## 12. Driver Manager

The Driver Manager no longer assumes that ISB must install an independent driver.

Its primary responsibilities are:

1. detect installed base drivers;
2. fingerprint their capabilities;
3. verify compatibility;
4. create configuration/rollback state;
5. select a validated base stack when multiple stacks are supported;
6. apply ISB configuration/workarounds;
7. verify the resulting runtime;
8. roll back ISB changes on failure.

Proprietary NVIDIA/Google driver packages are external inputs. ISB must not redistribute them without appropriate rights.

## 13. Control Center / CLI

The control plane should expose the same operations through stable contracts:

```text
scan
  -> driver/base-stack detection
  -> capability fingerprint
  -> diagnostics

fix
  -> rule evaluation
  -> proposed changes
  -> explicit apply
  -> verification

rollback
  -> restore ISB-managed state

verify
  -> API/feature probes
  -> regression checks

report
  -> IDR / driver bug report
```

A GUI is optional. The CLI remains usable without it.

## 14. Custom feature layer

ISB is not limited to bug fixes. Once the base driver exposes a usable API, ISB can add higher-level features without replacing the kernel driver.

Candidate feature groups:

- workload-aware CUDA/Tensor scheduling;
- neural super-resolution/reconstruction;
- denoising;
- external upscaler orchestration;
- compatibility profiles;
- frame-pacing and telemetry tools;
- application-specific feature policy;
- remote/headless management;
- research rendering paths.

Each feature must declare its dependency on a base-driver/API capability.

Example:

```text
ISB Neural SR
     |
 requires: CUDA + Tensor Cores
     |
 base driver exposes CUDA
     |
 V100 Tensor Cores execute supported kernels
```

## 15. Full alternative driver track

A complete independent driver remains technically interesting but is explicitly isolated:

```text
Mainline
  ISB Driver Fixer
       |
       +-- compatibility
       +-- workarounds
       +-- extensions
       +-- diagnostics

Research
  Independent Driver
       |
       +-- HAL
       +-- GPUVM
       +-- command submission
       +-- KMD/UMD
       +-- graphics/compute runtime
```

Research code must not become an accidental dependency of the Fixer.

## 16. Security and safety

The Fixer can manipulate driver/runtime configuration and may require administrator/root privileges. Therefore:

- changes must be explicit and auditable;
- rules must declare privilege requirements;
- configuration must be backed up before mutation;
- rollback must be available;
- unverified rules must not auto-apply in stable mode;
- external binaries remain separately sourced and verified;
- binary patching of proprietary drivers is not a default mechanism.

## 17. Hardware and OS scope

Initial qualification target:

- Windows 10/11 x64;
- Linux x86-64;
- Tesla V100 16 GB, with SXM2 and PCIe tracked separately.

Windows is particularly important for gaming/professional compatibility. Linux is important for development, compute and server validation.

Legacy Windows/BSD targets remain separate qualification work and are not implied by the initial MVP.

## 18. Architectural invariants

1. Fix first; replacement driver is not an MVP dependency.
2. Base-driver capability and ISB-added capability are separate.
3. Hardware capability and software exposure are separate.
4. Normal application paths should remain untouched unless a rule requires intervention.
5. Every workaround is evidence-backed, explainable and reversible.
6. Every supported combination is qualified by concrete GPU × OS × driver × API state.
7. Proprietary driver packages remain external inputs unless redistribution rights are established.
8. Documented API/layer mechanisms are preferred over binary patching.
9. Experimental features cannot silently become stable dependencies.
10. A future independent KMD/UMD remains isolated research work.
