# Graphics — Technical Task

## Goal
Diagnose and improve user-space graphics compatibility on top of the installed driver.

## Must implement
- Vulkan loader/ICD/device/extension probes.
- OpenGL/GLX/EGL probes.
- Windows DXGI/D3D11/D3D12/DirectCompute probes.
- Headless vs display-owner detection.
- Application-specific graphics compatibility profiles.
- Optional interception/shim integration behind explicit compatibility rules.

## Rules
- Do not implement a fake API merely to report success.
- API availability and functional execution are separate observations.
- Every workaround needs a reproducible test.

## Acceptance
- Linux and Windows expose a common normalized graphics capability model.
