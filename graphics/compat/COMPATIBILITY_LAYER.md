# ISB Application Compatibility Layer

This directory defines the application-level boundary used by ISB for graphics interception and external compatibility components.

The boundary is intentionally separate from CAL/HAL and does not claim to be a kernel or display driver.

```text
Game / Application
        |
        v
ISB Compatibility Layer
        |
        +-- API interception (future DX11/DX12/Vulkan)
        +-- external component selection
        +-- capability validation
        |
        v
UpscalerBackend / GraphicsBackend
        |
        v
ISB Runtime / CAL / Provider
```

## Upstream design inputs

- DLSS-Enabler: application-level interception and external component orchestration.
- OptiScaler/fakenvapi: NVAPI compatibility and application-level integration patterns.
- Mesa: layered graphics/API architecture.

The ISB implementation is authored independently unless a source file is explicitly listed in `third_party/UPSTREAM_COMPONENTS.json` as bundled source.

## Current status

**Experimental architecture boundary.** No DLL proxy, DXGI/D3D interception, or proprietary NVIDIA binary is bundled by this layer yet.
