# Providers — Technical Task

## Goal
Expose OS/API/vendor-specific facts through narrow provider interfaces.

## Must implement
- Linux and Windows provider boundaries.
- NVIDIA provider for nvidia-smi/NVML-compatible observations where legally and technically available.
- Vulkan/OpenGL/Direct3D/DirectCompute/CUDA probes as independent providers.
- Provider availability must be explicit; missing tools produce UNKNOWN, not false.
- Return raw observations plus provenance; do not decide final GpuCapabilities.

## Reference
GPU-T demonstrates a useful modular probe/factory pattern on Linux; use it as architectural reference, not as an automatic code dependency. citeturn0search0

## Acceptance
- Providers are testable with fake command/API results.
- No GUI dependency.
