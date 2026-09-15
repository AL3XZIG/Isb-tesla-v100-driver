# ISB V100 Architecture

## 1. Purpose

ISB is a software stack for detecting, characterizing, orchestrating, and extending NVIDIA Tesla V100 / GV100 systems.

The architecture is deliberately hardware-aware: missing RTX-era hardware features are not presented as if they existed.

## 2. Architectural layers

The planned dependency direction is:

**Application / Control Plane → ISB API → ISB Core → CAL → Providers → HAL → OS**

Technology runtimes and integrations consume the core abstractions rather than bypassing them with platform-specific logic.

### Core

`isb-core` owns orchestration and normalized GPU state:

- GPU identity and variant
- PCIe state
- HBM2 memory state
- temperature, power, clocks, utilization
- driver/runtime state
- diagnostics and logging
- workload state
- execution-policy decisions

Core must not contain OS-specific probing logic.

### Capability Abstraction Layer (CAL)

CAL exposes machine-readable capabilities rather than assumptions.

Example conceptual schema:

```yaml
gpu:
  architecture: Volta
  compute_capability: "7.0"
  variant: SXM2
  tensor_cores:
    available: true
    generation: 1
```

Capabilities that are unavailable on GV100 must be represented explicitly as unavailable.

### Providers

Providers translate technology-specific interfaces into ISB abstractions. Examples include:

- NVIDIA/NVML
- CUDA
- Vulkan
- OpenGL
- Linux-specific runtime providers
- experimental Nouveau/Mesa providers where validated

Providers must not silently invent capabilities that the underlying stack does not expose.

### HAL

The Hardware Abstraction Layer provides low-level OS primitives:

- files and directories
- processes
- memory information
- device enumeration
- PCI enumeration
- services
- kernel/module state
- permissions
- OS information

HAL is intentionally below technology providers.

## 3. Driver Manager

The Driver Manager is responsible for:

1. detecting the current driver/runtime stack;
2. checking compatibility;
3. collecting a backup/configuration manifest;
4. installing or configuring supported components supplied by the user/system;
5. verifying the resulting stack;
6. generating diagnostics;
7. rolling back on failure.

Linux is the primary development target.

A custom replacement NVIDIA kernel/display driver is **not** part of the MVP.

Windows support begins with detection and diagnostics; experimental graphics/WDDM workflows require a separate validated test path.

## 4. Graphics stack

ISB does not reimplement mature graphics translation layers when an established project can be integrated.

Planned integration points include:

- Vulkan
- OpenGL
- DXVK
- VKD3D-Proton
- OptiScaler or equivalent neutral interception mechanisms

The exact supported versions and extensions remain subject to validation.

## 5. Neural stack

The Neural Runtime provides a common interface for:

- super-resolution
- image reconstruction
- denoising
- temporal reconstruction
- research frame generation

Data acquisition may provide:

- color/frame data
- motion vectors
- depth
- history
- exposure
- jitter information
- reactive masks

When reliable temporal data is unavailable, spatial fallback may be used, but quality limitations must be recorded.

### External neural accelerator path

For a game-integrated external accelerator experiment:

**Game GPU → frame/data acquisition → transfer → V100 inference → transfer → composition/display**

The project must measure transfer, synchronization, inference, and frame-pacing costs. External neural processing is not assumed to be beneficial merely because V100 has Tensor Cores.

## 6. Compute Runtime

The Compute Runtime provides normalized execution interfaces over CUDA and suitable future providers.

It covers:

- kernel execution
- CUDA streams/events
- memory management
- synchronization
- Tensor Core workloads
- benchmark instrumentation

Single-kernel timing and concurrent execution require different measurement strategies; profiling tools must not be treated as zero-overhead instrumentation.

## 7. Workload-Aware Resource Utilization

ISB investigates whether workload-aware execution policies can improve end-to-end execution by mapping workload stages to resources that are actually available on GV100.

Examples:

- dense matrix/AI stages → Tensor Cores where the workload benefits;
- general compute → CUDA Cores;
- neural reconstruction/denoising → Tensor Cores where suitable;
- software ray tracing research → CUDA Cores for traversal/ray/shading work, with Tensor Cores potentially used for neural stages;
- heterogeneous pipelines → dynamically scheduled stages based on workload characteristics and runtime state.

**Tensor Cores do not become RT Cores, and CUDA Cores do not become dedicated optical-flow hardware.**

The research question is whether orchestration can outperform strong static strategies after including decision and synchronization overhead.

## 8. Diagnostics / IDR

The Intelligent Diagnostic Report (IDR) is a normalized report containing:

- hardware identity and exact variant;
- firmware/driver/runtime versions;
- CUDA/Vulkan/OpenGL state;
- capabilities;
- clocks, temperature, power and utilization;
- HBM2 state;
- PCIe topology/state;
- NVLink topology where applicable;
- errors and warnings;
- configuration provenance.

Reports should be machine-readable, diffable, and optionally anonymizable.

## 9. Profiles and manifests

Profiles describe known hardware/workload configurations.

Manifests record reproducibility-critical information:

- OS and kernel
- driver/runtime versions
- compiler/toolchain
- exact GPU variant
- clocks/power/ECC state
- model versions and hashes
- benchmark parameters
- random seeds
- environment variables

No performance claim should be accepted without sufficient provenance.

## 10. Model Registry

Every distributed or experimentally evaluated model should have metadata for:

- name/version
- source
- license
- hash
- supported runtime
- expected input/output format
- compatibility status
- benchmark provenance

Model weights are not automatically bundled with the repository.

## 11. Stability boundaries

### Stable

Validated engineering functionality with reproducible tests.

### Experimental

Functionality under active compatibility/performance evaluation.

### Research

Exploratory mechanisms where feasibility or usefulness is not yet established.

### Verify Before Freeze

A claim, API, driver capability, benchmark assumption, or compatibility statement that must be verified before becoming normative project policy.

## 12. Hardware invariants

The architecture must preserve these distinctions:

- V100 SXM2 and V100 PCIe are different configurations and measurements must identify the variant.
- V100 has Tensor Cores but no RT Cores.
- V100 has no dedicated Optical Flow Accelerator.
- V100 has no display outputs; display ownership may belong to another GPU/iGPU.
- V100 does not support MIG.
- NVLink availability/topology is configuration-dependent and must be detected rather than assumed.
- Peak hardware specifications are not equivalent to achieved application performance.

## 13. Security and legal boundaries

ISB must not silently download or redistribute proprietary driver binaries without appropriate rights.

Installation actions should be explicit, auditable, reversible, and privilege-aware.

Remote control APIs must authenticate before exposing privileged operations.

## 14. Future extensions

Potential future components include:

- multi-GPU orchestration;
- NVLink-aware scheduling;
- headless compute/server mode;
- richer remote control;
- additional graphics providers;
- advanced research rendering;
- frame-generation research.

These remain outside the initial stable surface until independently validated.

## 15. Architectural invariants

1. Hardware facts and software capabilities are separate concepts.
2. Hardware variants are never silently mixed.
3. Core logic remains independent of a specific OS provider.
4. Experimental features cannot silently become stable dependencies.
5. Performance claims require reproducible measurements.
6. Decision overhead is part of end-to-end execution cost.
7. Profiling overhead must be characterized rather than hidden.
8. Existing external projects are integrated through explicit boundaries.
9. Proprietary components are externally supplied unless redistribution rights are established.
10. When evidence contradicts an architectural assumption, the assumption is revised rather than the measurement being ignored.
