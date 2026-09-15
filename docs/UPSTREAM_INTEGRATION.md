# ISB Upstream Integration Map

This document records upstream projects that ISB may reuse, integrate, or treat as architectural references. It deliberately distinguishes **code reuse** from **architecture reuse** and from **external runtime integration**.

## Reference projects

- Mesa 3D — https://mesa3d.org/
- NVIDIA Open GPU Kernel Modules — https://github.com/NVIDIA/open-gpu-kernel-modules
- OptiScaler — https://github.com/optiscaler/OptiScaler
- DLSS-Enabler — https://github.com/artur-graniszewski/DLSS-Enabler

## What ISB can take now

### 1. Mesa: layered graphics architecture — ADOPT

Mesa demonstrates a useful separation between API implementations, hardware drivers, and layered drivers. Its current project includes hardware drivers and layered drivers such as Zink, which emits Vulkan rather than targeting a GPU directly, and NVK, a Vulkan driver for NVIDIA Maxwell and later hardware.

ISB adopts the **architectural principle**, not Mesa source code, for the following boundary:

```text
Application
    -> ISB graphics / compatibility layer
    -> graphics API backend
    -> ISB runtime / CAL
    -> provider / OS stack
```

This allows future integrations without making CAL depend on one graphics API.

### 2. NVIDIA Open GPU Kernel Modules: OS split — ADOPT AS REFERENCE

The NVIDIA repository explicitly separates OS-agnostic kernel-module code from the Linux kernel interface layer. ISB adopts this separation as a design rule for future platform-specific components:

```text
ISB common component
        |
        +-- Linux backend
        |
        +-- Windows backend
```

The upstream NVIDIA open kernel modules themselves are **not a V100 backend**. The current upstream documentation states that the open kernel modules target Turing and later GPUs. V100/GV100 is therefore outside that supported hardware scope.

ISB will not claim V100 support from the NVIDIA open-kernel-module repository.

### 3. OptiScaler: external compatibility/upscaling backend — INTEGRATE EXTERNALLY

OptiScaler is a GPL-3.0 project that bridges temporal upscaling and frame-generation technologies across GPU vendors and APIs. Its useful role in ISB is as an **external application-level graphics compatibility component**, not as code copied into the core ISB runtime.

Target boundary:

```text
Game
  -> compatibility/interception layer
  -> OptiScaler (external component)
  -> graphics/upscaling backend
  -> render/display GPU
```

ISB may provide detection, configuration, validation, launch integration, and telemetry around an installed OptiScaler version. OptiScaler source is not copied into ISB by this change.

### 4. DLSS-Enabler: interception/loader model — REFERENCE; POSSIBLE FUTURE CODE REUSE

DLSS-Enabler is MIT-licensed and demonstrates an application-level compatibility layer which can present DLSS/DLSS-G-style entry points while using alternative implementations. Its current workflow also automates acquisition of external components such as OptiScaler and XeSS.

ISB adopts the **integration pattern**:

```text
Game
  -> API interception / compatibility layer
  -> selected upscaler backend
```

The current change does not copy DLSS-Enabler source. Any future source reuse must retain the MIT copyright/license notice and must be reviewed file-by-file before import.

## What should NOT be copied into ISB core

### OptiScaler

OptiScaler is GPL-3.0. Its source should remain a separate component unless ISB deliberately adopts compatible licensing for the affected distribution boundary.

### Mesa

Mesa is a multi-license project. The core Mesa and Gallium code are MIT-licensed, but individual components can carry different licenses. Any source import must be audited by file/SPDX identifier rather than assuming that all Mesa code is MIT.

### NVIDIA Open GPU Kernel Modules

The repository contains MIT-licensed source components, while linked Linux kernel modules are subject to MIT/GPLv2 licensing terms. More importantly for this project, the current open-kernel-module target list starts at Turing, so it is not a V100 driver implementation.

## ISB integration layers

The upstream analysis suggests the following future repository boundaries:

```text
ISB API
   |
   +-- Compute Runtime
   |
   +-- Graphics Compatibility Layer
   |       |
   |       +-- external OptiScaler integration
   |       +-- future DLSS-Enabler-derived compatibility component
   |
   +-- CAL
           |
           +-- NVML provider
           +-- CUDA provider
           +-- future providers
```

The key rule is that **upstream projects are dependencies or explicitly isolated components, not hidden copies inside CAL**.

## Immediate implementation candidates

1. Add a provider-neutral `UpscalerBackend` interface to the graphics layer.
2. Add an OptiScaler external-component manifest containing version, source URL, license, hash, and compatibility state.
3. Add detection/validation for an installed OptiScaler component without bundling it.
4. Add a future Windows compatibility-layer boundary for DLL/API interception.
5. Keep V100 execution and inference in ISB Runtime/CAL; do not make OptiScaler responsible for hardware discovery.

These candidates are intentionally separated from the Verification Pack and can be implemented independently.

## Attribution rule

When source code is actually imported, the repository must record:

- upstream project;
- exact repository URL;
- exact commit/tag/version;
- original file path;
- license/SPDX identifier;
- copyright notice;
- ISB modifications.

A reference link alone is not a substitute for the applicable license notices when source code is redistributed.
