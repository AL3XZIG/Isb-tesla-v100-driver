# ISB V100 Architecture

## 1. Purpose

ISB is an independent driver/runtime stack for NVIDIA Tesla V100 / GV100 systems.

The architecture is hardware-aware and separates three concerns that must not be conflated:

1. **hardware support** — GV100/V100 and future GPU families;
2. **OS/platform support** — Windows, Linux, BSD and their native driver models;
3. **technology/runtime support** — CUDA, Vulkan, OpenGL, Direct3D/DXGI and management APIs.

The first production hardware target is **GV100 / Tesla V100 16 GB**, with 32 GB V100 support following as a separate validation target.

## 2. Independent OS and hardware matrices

OS and hardware support are independent dimensions. A supported OS does not imply support for every GPU, and a supported GPU does not imply support on every OS.

### Operating-system tiers

- **O1 / P0:** Windows 10/11 x64 — primary gaming, professional and compute target.
- **O1 / P0:** Linux x86-64 — primary development, AI, rendering and server target.
- **O2 / P2:** Windows 7 x64 — legacy compatibility target.
- **O2 / P2:** Windows 8.1 x64 — legacy compatibility target.
- **O3 / P3:** FreeBSD x86-64 — community/server target.
- **O4 / P4:** OpenBSD, NetBSD and other Unix-like systems — experimental target.
- **ARM64:** future architecture target, outside the initial production matrix.

### Hardware tiers

- **H1 / P0:** GV100 / Tesla V100 16 GB.
- **H2 / P1/P2:** GV100 / Tesla V100 32 GB.
- **Future:** GP104, P104, CMP and other GPUs only after independent HAL/provider validation.

V100 SXM2 and V100 PCIe are separate hardware variants even though both are GV100-based.

The complete matrix is maintained in `docs/SUPPORT_MATRIX.md`.

## 3. Layered architecture

The common ISB software is deliberately split into OS-neutral core logic, hardware abstraction, and OS-specific integration:

```text
                         Applications / Games
                                  |
                         ISB API / Compatibility
                                  |
                         ISB Core / Runtime
                         /         |          \
                       CAL     Providers    Policy
                                  |
                    +-------------+-------------+
                    |                           |
             Hardware Abstraction        OS Abstraction
                    |                           |
              Hardware Backend        +--------+--------+
                    |                  |        |        |
                 GV100               Linux   Windows    BSD
                    |                 DRM     WDDM      native API
                    |                         /   \
                    |                        KMD  UMD
                    +--------------------------+--------+
                                  |
                         Physical GPU / system
```

The diagram is conceptual rather than a statement that every path uses every layer.

### 3.1 ISB Core / Runtime

`isb-core` owns normalized device state and orchestration:

- GPU identity and exact variant;
- PCIe/device topology;
- HBM2 memory state;
- temperature, power, clocks and utilization;
- driver/runtime state;
- diagnostics and logging;
- workload state;
- execution-policy decisions;
- stable public runtime contracts.

Core must not contain direct WDDM, DRM, BSD, NVML or kernel-driver implementation details.

### 3.2 CAL

The Capability Abstraction Layer exposes machine-readable hardware/runtime capabilities rather than assumptions.

Example:

```yaml
gpu:
  architecture: Volta
  compute_capability: "7.0"
  variant: V100_SXM2
  tensor_cores:
    state: available
    generation: 1
  rt_cores:
    state: unavailable
```

Unavailable and unknown capabilities remain distinct states.

### 3.3 Technology providers

Providers translate technology-specific interfaces into ISB contracts. Examples include:

- CUDA;
- NVML-compatible management provider;
- Vulkan;
- OpenGL;
- Direct3D/DXGI;
- Linux-specific runtime integration;
- validated experimental Mesa/Nouveau integration where appropriate.

Providers are not kernel drivers and must not silently invent hardware capabilities.

### 3.4 Hardware Abstraction Layer

The HAL is a narrow hardware-facing contract. It is **not** a generic collection of OS utilities.

HAL v0.1 is intentionally limited to:

- device handles;
- PCI identity/metadata;
- BAR metadata;
- MMIO read/write primitives;
- backend lifecycle;
- controlled device enumeration/open operations.

The HAL does not define Vulkan, CUDA, WDDM, DRM, scheduling, GPU virtual memory, command submission or product-specific policy.

A future production HAL backend will ultimately reach the ISB kernel driver rather than requiring NVML.

### 3.5 OS Abstraction / platform backends

OS integration is a separate architectural dimension from hardware support.

#### Linux

The Linux backend targets the DRM/KMS ecosystem and Linux kernel interfaces. The kernel side will eventually provide memory management, GPUVM, synchronization, command submission, scheduling and reset/recovery facilities through native Linux mechanisms.

#### Windows

The Windows backend targets WDDM. WDDM is not a thin wrapper around the HAL: Windows graphics integration requires both a kernel-mode driver (KMD) and user-mode driver (UMD), integrated with the Windows graphics runtime and its version-specific interfaces.

The ISB Core and hardware contracts remain common, while KMD/UMD implementations are Windows-specific.

#### BSD

BSD support is a separate backend and is not assumed to inherit Linux DRM or Windows WDDM behavior.

## 4. Driver lifecycle and installation

The Driver Manager is responsible for:

1. detecting the current platform and hardware;
2. checking the exact OS × hardware compatibility matrix;
3. collecting a backup/configuration manifest;
4. staging the correct platform driver components;
5. installing/configuring them with explicit privilege escalation;
6. verifying device initialization and runtime health;
7. generating diagnostics;
8. rolling back on failure.

### 4.1 Windows installation model

The user-facing Windows product can be a **single self-contained `.exe` installer**.

However, the installed driver is not literally one executable. A Windows driver package normally contains components such as:

- `.sys` kernel-mode driver;
- `.dll` user-mode driver/runtime components;
- `.inf` installation metadata;
- `.cat` catalog/signature data;
- ISB runtime/control binaries;
- configuration and diagnostics resources.

The installer can embed these components into one bootstrapper/self-extracting `.exe`, select the correct OS/hardware package, install transactionally and perform rollback. Windows still loads the appropriate kernel/user-mode driver components according to WDDM and Plug-and-Play rules.

Normal production deployment requires appropriate Windows driver signing. The installer must never bypass platform security silently.

### 4.2 Linux installation model

Linux may also expose a single installer command or self-extracting package, but the installed stack consists of native kernel modules, shared libraries, firmware/resources, device rules and configuration as required by the target distribution/kernel.

A single `.exe` is therefore **not** a cross-platform driver binary. Each OS has its own native driver artifacts.

## 5. Production independence from NVIDIA's driver

During development, NVML/NVIDIA drivers may be used as a reference or diagnostic provider.

The final ISB driver stack is intended to operate without requiring NVIDIA's proprietary driver for the supported V100 path:

```text
Application
   -> ISB Runtime / API
   -> ISB Providers
   -> ISB Core
   -> ISB HAL / OS backend
   -> ISB kernel driver
   -> GV100
```

CUDA Toolkit remains conceptually separate from the kernel driver. A toolkit can be installed independently while the ISB driver provides the device-side driver interface required by supported CUDA workloads.

## 6. Graphics stack

ISB does not reimplement mature graphics translation layers when an established project can be integrated through an explicit boundary.

Planned integration points include:

- Vulkan;
- OpenGL;
- Direct3D/DXGI;
- DXVK;
- VKD3D-Proton;
- OptiScaler or equivalent neutral interception mechanisms;
- future application compatibility/interception components.

The compatibility layer is user-space functionality and must not be mistaken for the kernel driver, HAL, CAL or hardware backend.

### 6.1 External graphics compatibility boundary

Responsibilities may include:

- identifying compatible applications;
- selecting an external graphics/upscaling component;
- validating component version and architecture;
- passing configuration;
- collecting compatibility diagnostics.

It must never claim that V100 has DLSS hardware, RT Cores or a dedicated optical-flow accelerator.

External projects remain independently licensed and versioned. GPL components such as OptiScaler are not silently copied into the independent ISB core.

### 6.2 Upscaler backend abstraction

```text
Compatibility Layer
        |
        v
Upscaler Backend Interface
        |
        +-- External OptiScaler
        +-- future FSR/XeSS backend
        +-- future ISB neural backend
```

CAL describes what hardware/runtime capabilities exist; the upscaler interface describes how a selected reconstruction implementation is invoked.

## 7. Neural stack

The Neural Runtime may provide common interfaces for:

- super-resolution;
- image reconstruction;
- denoising;
- temporal reconstruction;
- research frame generation.

For external V100 inference:

**Render GPU → frame/data acquisition → transfer → V100 inference → transfer → composition/display**

Transfer, synchronization, inference and frame-pacing costs must be measured. Tensor Cores alone do not prove that an external neural path improves end-to-end performance.

## 8. Compute Runtime

The Compute Runtime provides normalized execution interfaces over CUDA and future validated providers.

It covers:

- kernel execution;
- streams/events;
- device and host memory;
- synchronization;
- Tensor Core workloads;
- benchmark instrumentation.

The production runtime must not depend on NVML once the independent kernel path is validated.

## 9. Workload-aware resource utilization

ISB may investigate workload-aware scheduling and orchestration over resources actually available on GV100:

- dense matrix/AI stages → Tensor Cores where beneficial;
- general compute → CUDA Cores;
- neural reconstruction/denoising → Tensor Cores where suitable;
- software ray tracing research → CUDA Cores, with Tensor Cores for neural stages where appropriate.

Tensor Cores do not become RT Cores, and CUDA Cores do not become dedicated optical-flow hardware.

The research question is whether orchestration can beat strong static strategies after decision, synchronization and transfer overhead are included.

## 10. Diagnostics / IDR

The Intelligent Diagnostic Report contains normalized information about:

- exact hardware identity and variant;
- firmware/driver/runtime versions;
- CUDA/Vulkan/OpenGL/Direct3D state;
- capabilities;
- clocks, temperature, power and utilization;
- HBM2 state;
- PCIe topology/state;
- NVLink topology where applicable;
- errors and warnings;
- configuration provenance.

Reports must remain machine-readable, diffable and provenance-aware.

## 11. Profiles and manifests

Profiles describe known hardware/workload configurations.

Manifests record reproducibility-critical information including:

- OS and kernel;
- driver/runtime versions;
- compiler/toolchain;
- exact GPU variant;
- clocks/power/ECC state;
- model versions and hashes;
- benchmark parameters;
- random seeds;
- environment variables.

No performance claim is accepted without sufficient provenance.

## 12. Stability model

ISB uses four maturity states:

### Stable

Validated engineering functionality with reproducible tests.

### Experimental

Implemented functionality under active compatibility/performance evaluation.

### Research

Exploratory mechanisms whose feasibility or usefulness is not yet established.

### Verify Before Freeze

A claim, API, driver capability, benchmark assumption or compatibility statement that must be verified before becoming normative project policy.

### 12.1 OS × hardware qualification

Stability is qualified per concrete combination, not globally.

For example, `Windows 11 x64 + V100 SXM2 16 GB` must pass its own qualification gate. Passing that gate does not automatically qualify `Linux + V100 32 GB` or any future GP104/P104 device.

A production qualification should cover at least:

1. deterministic PCI/device discovery;
2. attach/detach and error paths;
3. MMIO safety and bounds checking;
4. memory isolation and GPUVM correctness;
5. command submission and synchronization;
6. GPU reset/recovery;
7. suspend/resume where applicable;
8. multi-process and multi-GPU operation where supported;
9. long-duration stress workloads;
10. installer upgrade/uninstall/rollback;
11. crash diagnostics and reproducibility;
12. performance regression testing.

Therefore the architecture is designed for stability, but **stability is a qualification result, not an assumption of the architecture**.

## 13. Hardware invariants

The architecture preserves these facts:

- V100 SXM2 and V100 PCIe are different configurations and must be identified separately.
- V100 has Tensor Cores but no RT Cores.
- V100 has no dedicated Optical Flow Accelerator.
- V100 has no display outputs; another GPU/iGPU may own display output.
- V100 does not support MIG.
- NVLink availability/topology is configuration-dependent and must be detected.
- Peak specifications are not equivalent to achieved application performance.

## 14. Security and legal boundaries

ISB must not silently download or redistribute proprietary driver binaries without appropriate rights.

Installation actions must be explicit, auditable, reversible and privilege-aware.

Remote control APIs must authenticate before exposing privileged operations.

Third-party components are isolated and versioned. GPL components and multi-license upstream projects require explicit provenance and license tracking.

## 15. Future extensions

Potential future components include:

- multi-GPU orchestration;
- NVLink-aware scheduling;
- headless compute/server mode;
- richer remote control;
- additional graphics providers;
- advanced research rendering;
- frame-generation research;
- application-level graphics compatibility;
- external upscaler orchestration;
- additional GPU families after independent validation.

These remain outside the stable surface until independently validated.

## 16. Architectural invariants

1. Hardware support and OS support are independent matrices.
2. Hardware facts and software capabilities are separate concepts.
3. Hardware variants are never silently mixed.
4. Core logic remains independent of a specific OS provider.
5. WDDM/DRM/BSD integrations remain platform-specific.
6. HAL remains a narrow hardware-facing contract.
7. Experimental features cannot silently become stable dependencies.
8. Performance claims require reproducible measurements.
9. Decision and transfer overhead are part of end-to-end cost.
10. Existing external projects are integrated through explicit boundaries.
11. Proprietary components are externally supplied unless redistribution rights are established.
12. Third-party source reuse requires explicit provenance and license tracking.
13. A driver is considered stable only after OS × hardware qualification.
14. When evidence contradicts an architectural assumption, the assumption is revised rather than the measurement being ignored.
