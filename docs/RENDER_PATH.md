# Automatic Multi-GPU Render Path Configuration

## Scope

The render-path subsystem distinguishes the roles of GPUs instead of assuming that the fastest GPU is also the display adapter.

The intended V100 topology is:

- Tesla V100: compute/render target when the installed driver/API path permits it.
- iGPU or secondary dGPU: display/presentation adapter.

A Tesla V100 having no physical display connectors is expected and is not itself an error.

## Detection pipeline

Real mode uses three observation layers:

1. NVML: enumerates NVIDIA devices, reads PCI identity, active-display state where exposed, and WDDM/TCC where exposed.
2. Vulkan provider: enumerates physical devices and correlates vendor/device IDs with NVML devices.
3. OS platform adapter: Windows uses DXGI; Linux uses DRM connector state and the active Wayland/X11 session.

The classifier uses observed facts only and never invents a display output for the V100.

## Role classification

- ComputeOnly: compute-capable GPU without an observed active display.
- DisplayOnly: display adapter without the compute capability represented by the render-path provider.
- ComputeAndDisplay: both compute and display are observed.
- Unknown: insufficient evidence.

The render target is selected from a GPU with an observed graphics API capability. On Windows, TCC is treated as a graphics-path blocker; ISB does not silently switch TCC to WDDM.

## Linux

For Linux multi-GPU configuration, ISB can plan application-scoped environment variables for offload:

- __NV_PRIME_RENDER_OFFLOAD=1
- __GLX_VENDOR_LIBRARY_NAME=nvidia for OpenGL
- __VK_LAYER_NV_optimus=NVIDIA_only when Vulkan offload is observed/planned

These are plan data in the current stage. Persistent system or per-application mutation and rollback remain a separate deployment boundary.

## Windows

DXGI enumerates hardware adapters. NVML supplies NVIDIA PCI identity and, where available, the driver model. A V100 reported as TCC is not presented as a graphics render target. Automatic WDDM/TCC changes are intentionally excluded.

## CLI

- isb render detect
- isb render --json detect
- isb render plan
- isb render configure --apply
- isb render verify

Mock mode remains available for deterministic tests and is marked synthetic.

## Current limitations

Detection is real when NVML/Vulkan/platform providers are available. Host mutation is intentionally not claimed as complete yet. Persistent Linux configuration, Windows per-application preference persistence, end-to-end application presentation verification, and stronger Vulkan PCI-bus correlation remain deployment work.

The real configure operation refuses host mutation rather than pretending that configuration succeeded.
