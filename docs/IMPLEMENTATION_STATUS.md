# ISB implementation status

This document separates implemented code from specifications, planned work, and hardware validation. A documented interface is not evidence that a driver backend exists.

## Current state

| Component | Status | Evidence / boundary |
|---|---|---|
| CAL v1 | Implemented | C++ user-space capability model and tests |
| CAL validation | Implemented on this branch | Cross-field invariants and V100 profile checks |
| Verification Tools v1 | Implemented | IDR schema, probe and benchmark tooling |
| HAL v0.1 | Implemented / verification required | Provider-neutral C++ HAL + fake backend |
| OSAL v0.1 | Specification / implementation in progress | Contract approved; real OS backends do not exist yet |
| NVML provider | Planned / partial | Reference integration only until provider code is validated |
| Linux KMD | Not implemented | No production kernel driver yet |
| Windows KMD/UMD | Not implemented | WDDM/MCDM boundaries are specified, no backend yet |
| BSD backend | Not implemented | Architecture tier only |
| GPUVM | Not implemented | Future kernel-driver layer |
| Command submission | Not implemented | Future kernel-driver layer |
| Interrupt/event path | Not implemented | OSAL/KMD work required |
| Fence/synchronization | Not implemented | OSAL v0.2+ / KMD work required |
| Reset/recovery | Not implemented | Capability query is specified; reset is future work |
| Vulkan/OpenGL backend | Not implemented | Graphics compatibility research only |
| D3D/WDDM graphics path | Not implemented | Future Windows backend |
| CUDA execution backend | Not implemented | CUDA Toolkit remains an external toolchain |
| Installer/control center | Not implemented | Future control-plane work |
| Real Tesla V100 validation | Blocked | Requires the target hardware and a reproducible test environment |

## Validation levels

Use these terms in issues, commits, pull requests, and release notes:

1. **Unit test** — deterministic test of a local component.
2. **Contract test** — verifies an interface invariant between layers.
3. **Integration test** — verifies multiple implemented components together.
4. **Backend test** — verifies an OS/runtime backend without implying physical GPU correctness.
5. **Hardware validation** — executed against the specified physical GPU, OS, driver state, firmware, and topology.
6. **Performance validation** — hardware measurements with recorded provenance and methodology.

A green unit-test job must never be described as proof that the V100 driver works on physical hardware.

## Hardware evidence policy

For V100 claims, record at minimum:

- V100 variant: SXM2 or PCIe;
- framebuffer size;
- PCI identity when applicable;
- firmware/VBIOS source and revision when available;
- host CPU and OS/kernel;
- active NVIDIA or ISB driver stack;
- CUDA/toolkit version when relevant;
- topology/NVLink configuration when relevant;
- exact benchmark revision;
- raw result and measurement method.

Unknown values remain unknown until measured or sourced. Do not replace unknown with a plausible value.

## Promotion rule

The maturity of the repository is determined by the lowest layer required for the claim. For example, an implemented user-space API cannot be called a working V100 driver while the KMD, GPUVM, and command-submission path are absent.
