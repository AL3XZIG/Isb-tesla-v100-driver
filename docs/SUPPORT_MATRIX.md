# ISB Support Matrix

## 1. Product support policy

ISB is a **Linux x86-64-only V100 product**. The project no longer treats Windows or BSD as supported runtime targets.

Support is expressed as:

- **OS/platform support** — maturity of the Linux integration;
- **hardware support** — maturity of the V100 backend;
- **feature evidence** — whether a capability is actually observed and verified on the installed Linux driver/runtime.

A feature is production-ready only when the relevant Linux platform, V100 hardware variant and provider path have been validated.

## 2. Operating-system scope

| Tier | Platform | Priority | Intended role | Status |
|---|---|---:|---|---|
| O1 | Linux x86-64 | P0 | Gaming, rendering, CUDA/AI, diagnostics, server/workstation | Primary target |

Not in the supported product matrix:

- Windows 10/11 and legacy Windows;
- FreeBSD/OpenBSD/NetBSD;
- other Unix-like systems;
- ARM64.

Old platform-specific code and documentation may remain only as historical or research material and must not be presented as supported runtime functionality.

## 3. Hardware scope

| Tier | GPU | Priority | Notes |
|---|---|---:|---|
| H1 | Tesla V100 SXM2 16 GB / GV100 | P0 | First physical qualification target |
| H2 | Tesla V100 SXM2 32 GB / GV100 | P1 | Same architecture; detect capacity/topology |
| H2 | Tesla V100 PCIe 16/32 GB / GV100 | P1/P2 | Separate board/thermal/topology qualification |

The product remains V100-focused. Other GPU families are not product targets.

## 4. Intended Linux topology

The primary multi-GPU topology is:

```text
             Linux display server
              / Wayland/X11
                    |
          +---------+---------+
          |                   |
      Display GPU          Tesla V100
      iGPU / dGPU        Compute + Render
          |                   |
       outputs          Vulkan/OpenGL/CUDA
```

A V100 without physical display connectors is expected.

ISB must distinguish:

- **Display GPU** — owns physical display/presentation;
- **Render GPU** — performs graphics rendering;
- **Compute GPU** — performs CUDA/compute work.

The common target is:

**Display GPU = iGPU/secondary dGPU; Render/Compute GPU = V100.**

## 5. Linux graphics stack

The supported graphics paths are:

- Vulkan;
- OpenGL;
- Linux DRM device discovery;
- NVIDIA PRIME Render Offload where applicable;
- Vulkan device selection;
- X11/Xwayland/Wayland integration where actually observed.

ISB must verify the selected Vulkan/OpenGL device rather than assuming that the V100 is the renderer.

## 6. Linux compute stack

The supported compute paths are:

- CUDA;
- NVML;
- CUDA-aware application/runtime integration;
- V100 Tensor Core workloads where supported by the installed software stack.

DirectCompute/DXGI/WDDM are not part of the Linux product.

## 7. Driver support model

The base driver remains an external NVIDIA component.

ISB records:

- vendor;
- branch;
- version;
- OS;
- architecture;
- package type;
- V100 compatibility evidence;
- CUDA/runtime/API observations;
- provenance.

Driver installation is a separate explicit operation. ISB must not silently replace the active NVIDIA driver.

## 8. Qualification policy

Linux/V100 production qualification requires, at minimum:

1. deterministic V100 discovery;
2. exact SXM2/PCIe and memory-size identification where observable;
3. NVML observation;
4. CUDA observation;
5. Vulkan enumeration;
6. OpenGL observation where available;
7. multi-GPU role detection;
8. application-scoped V100 render routing;
9. telemetry and diagnostics;
10. install/upgrade/uninstall/rollback validation;
11. long-duration workloads;
12. reproducible evidence reports.

A capability remains **UNKNOWN** until the relevant Linux provider or verification test establishes it.

## 9. Compatibility rule

Compatibility is specific to:

**Linux x86-64 + V100 variant + installed NVIDIA driver + runtime/API path + application.**

No single driver version is assumed to be universally compatible.

The repository must prefer observed/provider-backed evidence over hard-coded claims.
