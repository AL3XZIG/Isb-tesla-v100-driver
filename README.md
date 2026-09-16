# ISB Driver Fixer

**ISB — Intelligent Systems Bureau**

Open-source compatibility, diagnostics, workaround and extension layer for NVIDIA GPU driver stacks, with **Tesla V100 / GV100** as the first target.

> ISB does not start by replacing the NVIDIA kernel driver. It sits above or beside an installed driver, detects broken paths, applies evidence-backed compatibility workarounds, and adds user-space features where the underlying driver exposes the required API.

## Why this direction

A complete replacement GPU kernel + user-mode driver stack is a multi-year project. It is not the MVP.

The practical target is a **Driver Fixer** that can make an existing NVIDIA stack more usable on hardware such as V100:

```text
Game / Application
        |
        v
   ISB Fixer
   /   |    \
  /    |     \
Detect Diagnose Workaround / Extension
              |
              v
       NVIDIA Driver Stack
              |
              v
          V100 / GV100
```

ISB can therefore work with different NVIDIA driver bases when they expose different useful functionality. For example, a validated Google Compute Engine / vGPU-class driver can be treated as a **base stack** when it exposes an API path useful to V100; ISB does not claim that ISB itself implements that underlying API.

## MVP

The first concrete milestone is:

**one real V100 driver problem → deterministic diagnosis → one reproducible fix/workaround → regression test.**

The MVP does **not** require a custom WDDM or Linux kernel driver.

### Initial feature classes

- Driver/GPU/API/environment detection.
- Driver capability fingerprinting and provenance.
- Known-bug and hardware-quirk database.
- Rule-based workaround engine.
- Application compatibility profiles.
- Runtime configuration and feature selection.
- Optional user-space interception/shims for supported APIs.
- Driver health and fault diagnostics.
- Regression testing across driver versions.
- Custom features layered on top of an existing driver where technically possible.
- Control Center and CLI for inspection, applying fixes, rollback and verification.

## Driver-base model

ISB treats the installed NVIDIA stack as a **base driver**, not as something to blindly replace.

```text
                 ISB
        +----------+----------+
        |          |          |
     Detect     Fix/Work   Extensions
        |          |          |
        +----------+----------+
                   |
              Base Driver
          +--------+--------+
          |        |        |
       NVIDIA   Google   other validated
       stack     stack       stack
                   |
                  GPU
```

A base stack may expose functionality that another package does not. ISB records this as an observed software capability, with provenance and verification status.

**Important:** if DirectCompute is exposed by a particular NVIDIA/Google driver package, ISB can detect, preserve, configure and build features around that path. ISB does not manufacture DirectCompute merely by installing a user-space layer.

## Architecture

```text
Application / Game / Tool
            |
     Compatibility Layer
            |
      ISB Fix Engine
   +--------+---------+
   |        |         |
 Detect  Diagnose  Workarounds
   |        |         |
   +--------+---------+
            |
   Runtime Interception
   / Configuration / Profiles
            |
       Base Driver
            |
       OS Driver Model
            |
        GPU / GV100
```

Detailed architecture: [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md).

## Repository structure

```text
core/                  normalized state and orchestration
cal/                   capability abstraction
hal/                   low-level platform primitives
providers/             NVIDIA/API/platform providers
fixer/                 detection, diagnosis, rules, workarounds, shims
compatibility/         API-specific compatibility logic
database/              drivers, GPUs, applications and known issues
diagnostics/           IDR, crash, fault and evidence collection
drivers/               base-driver detection, staging and rollback
integrations/          DXVK, VKD3D, OptiScaler and other external components
graphics/              graphics compatibility infrastructure
compute/               CUDA/DirectCompute/compute compatibility
neural/                SR/reconstruction/denoising research
benchmarks/            reproducible measurements
profiles/              hardware/application/driver profiles
manifests/             reproducibility manifests
installer/              install/configure/verify/rollback workflow
control-center/        optional GUI
cli/                   command-line interface
tests/                 unit/compatibility/regression/hardware tests
research/              experimental alternative-driver work
third_party/           audited external references
docs/                  architecture and protocols
legal/                 licensing and proprietary-component policy
.github/               CI
```

## What ISB can fix

The stable design target is **software-visible driver behavior**, for example:

- known API/extension incompatibilities;
- bad driver-version/application combinations;
- incorrect feature selection;
- broken or unstable runtime paths with a validated alternative path;
- application-specific compatibility problems;
- configuration and deployment mistakes;
- reproducible user-mode/API failures that can be isolated and worked around.

A fix is accepted only when its trigger, action, provenance and verification are recorded.

## What ISB cannot magically fix

- Missing physical hardware blocks.
- RT Cores on V100.
- A dedicated Optical Flow Accelerator on V100.
- Proprietary functionality that the installed stack does not expose and cannot legally/technically be substituted for.
- Arbitrary kernel-driver faults from user space when no supported recovery path exists.

A future independent KMD/UMD remains a **research track**, not an MVP dependency.

## V100 scope

Primary target: **Tesla V100 SXM2 16 GB**.

V100 PCIe is tracked separately. 16 GB and 32 GB are separately qualified targets.

Relevant hardware facts:

- GV100 / Volta / SM70.
- First-generation Tensor Cores.
- No RT Cores.
- No dedicated Optical Flow Accelerator.
- ECC HBM2.
- No MIG.
- SXM2/PCIe differences must be detected rather than assumed.

## Experimental features

ISB may add higher-level features such as:

- neural super-resolution/reconstruction;
- denoising;
- workload-aware resource utilization;
- external upscaler orchestration;
- application compatibility profiles;
- frame-generation research;
- additional compute/graphics compatibility layers.

These are user-space features unless explicitly documented otherwise.

## Development principles

1. Fix first, replace later.
2. Evidence before claims.
3. Never confuse a base driver's capability with an ISB capability.
4. Every workaround needs a reproducible trigger and verification path.
5. Preserve working driver functionality instead of replacing it unnecessarily.
6. Hardware facts, driver capabilities and ISB-added features are separate layers.
7. Proprietary driver binaries are supplied by the user/vendor package; ISB does not redistribute them without appropriate rights.
8. External projects are integrated through explicit, auditable boundaries.
9. Experimental features never silently become stable dependencies.
10. V100 hardware limitations are not hidden by software naming.

## Status

**Driver Fixer reframe — architecture and foundation phase.**

The existing CAL v1 and Verification Tools remain valid foundations. The next implementation work is the Fix Engine, driver-base fingerprinting, issue database, first workaround, and regression harness.

## License

Project licensing and third-party provenance are maintained under [`legal/`](legal/).
