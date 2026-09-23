# Linux Installer — Technical Task

## Goal

Provide a production Linux installer/configuration workflow for ISB V100 Hub.

The supported product target is **Linux x86-64 + Tesla V100**. The installer configures ISB and Linux application/render profiles; it does not replace the NVIDIA base driver.

## Must implement

- Preflight environment checks.
- Detect Linux distribution/package manager.
- Detect architecture and required build/runtime dependencies.
- Detect NVIDIA driver, V100 and relevant runtime/API providers.
- Install/uninstall ISB components.
- Support user-local and system installation modes.
- Initialize ISB configuration.
- Detect display GPU vs Tesla V100.
- Configure safe application-scoped V100 render routing where supported.
- Backup configuration before mutation.
- Verify every applied configuration.
- Roll back failed configuration.
- Produce deterministic installation logs.
- Expose the same operations through CLI and GUI/Control Center.

## Linux render configuration

The installer/configuration layer must support the intended topology:

- iGPU/secondary dGPU = display GPU;
- Tesla V100 = render/compute GPU.

Prefer application-scoped configuration over global GPU switching.

Possible mechanisms include Linux PRIME Render Offload, Vulkan device selection and OpenGL vendor selection, but applicability must be detected rather than assumed.

## Driver boundary

The normal ISB installer:

- may detect the installed NVIDIA driver;
- may stage verified driver metadata/artifacts for an explicit user workflow;
- must never silently replace the active NVIDIA driver;
- must never bundle proprietary NVIDIA driver binaries into ISB releases.

Driver installation remains a separate explicit privileged workflow.

## Acceptance

- Fresh Linux install is reproducible.
- Upgrade preserves compatible configuration.
- Uninstall removes ISB without silently removing the NVIDIA driver.
- Failed configuration restores the previous configuration.
- V100 render routing can be dry-run, applied and verified where the host supports it.
