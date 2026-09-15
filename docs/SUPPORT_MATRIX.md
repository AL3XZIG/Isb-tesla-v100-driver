# ISB Support Matrix

## 1. Support model

ISB separates **OS support** from **hardware support**. A hardware backend is not tied to a single operating system, and an OS integration does not imply support for every GPU family.

Support is expressed as an independent matrix:

- **OS tier** describes the maturity of the platform integration.
- **Hardware tier** describes the maturity of the GPU backend.
- A feature is production-ready only when both dimensions are validated.

## 2. Operating-system tiers

| Tier | Platform | Priority | Intended role | Status target |
|---|---|---:|---|---|
| O1 | Windows 10/11 x64 | P0 | Gaming, professional graphics, compute | Production target |
| O1 | Linux x86-64 | P0 | Development, AI, rendering, server | Production target |
| O2 | Windows 7 x64 | P2 | Legacy compatibility | Legacy/experimental |
| O2 | Windows 8.1 x64 | P2 | Legacy compatibility | Legacy/experimental |
| O3 | FreeBSD x86-64 | P3 | Community/server | Experimental |
| O4 | OpenBSD / NetBSD / other Unix-like | P4 | Research/community | Experimental |

ARM64 is a future architecture target and is not part of the initial production matrix.

## 3. Hardware tiers

| Tier | GPU family | Initial target | Notes |
|---|---|---|---|
| H1 | GV100 / Tesla V100 16 GB | P0 | First physical validation target |
| H2 | GV100 / Tesla V100 32 GB | P1/P2 | Same architectural family; capacity and board topology must be detected |
| Future | GP104 / P104 / CMP and other validated devices | TBD | Added only after a separate HAL/provider validation |

V100 SXM2 and V100 PCIe remain separate variants even when they share the same GV100 architecture.

## 4. Platform architecture

The common ISB stack is OS- and hardware-neutral above the platform boundaries:

```text
Applications / Control Plane
            |
         ISB API
            |
       ISB Core/Runtime
            |
     +------+------+
     |             |
 GPU HAL      OS Abstraction
     |             |
     |      +------+------+------+
     |      |             |      |
     |    Linux        Windows   BSD
     |     DRM          WDDM     BSD GPU API
     |                  /   \
     |                 KMD   UMD
     |                  |
     +------------------+
            |
      Hardware backend
            |
       GV100 / future GPU
```

The diagram is conceptual. WDDM is not a thin wrapper around HAL: Windows graphics support requires a kernel-mode driver (KMD) and user-mode driver (UMD) integrated with the Windows graphics stack. Linux graphics integration uses DRM/KMS and kernel/user-space interfaces. BSD support is a separate backend.

## 5. Windows packaging

The Windows product is distributed as an installer executable, but the installed driver is a package rather than a single executable binary.

A release may contain:

- installer/bootstrapper `.exe`;
- kernel driver `.sys`;
- user-mode driver/runtime `.dll` files;
- INF installation metadata;
- catalog/signature `.cat` files;
- ISB Runtime and control utilities;
- configuration and diagnostics resources.

The installer may embed these files into one self-contained `.exe` and extract/install them transactionally. This does **not** mean Windows loads the whole driver from one `.exe`.

Kernel-mode Windows components require appropriate code signing for normal deployment. Installation must be explicit, privilege-aware, auditable, reversible, and able to roll back on failure.

## 6. Linux packaging

Linux uses a native package/installer model. The final distribution may provide a single installer command or self-extracting package, but kernel modules, shared libraries, firmware/resources, udev rules, and configuration remain separate installed components where required by the target distribution.

## 7. Stability policy

A platform is not marked production-ready merely because the code compiles or the installer succeeds.

Production promotion requires, at minimum:

1. deterministic device discovery;
2. clean attach/detach and failure paths;
3. memory isolation and bounds checking;
4. command submission and synchronization validation;
5. GPU reset/recovery testing;
6. suspend/resume testing where applicable;
7. multi-process and multi-GPU testing where supported;
8. stress testing and long-duration workloads;
9. installer rollback/uninstall validation;
10. reproducible diagnostics and crash/failure reporting.

Windows and Linux therefore share the same ISB Core contracts but have independent kernel/OS stability qualification.

## 8. Compatibility rule

A combination such as `Windows 11 x64 + V100 SXM2 16 GB` is a concrete support target. `Windows 11 x64 + future GP104` is not supported merely because both dimensions exist in the matrix.

Every OS × hardware combination must pass its own validation gate before being advertised as supported.
