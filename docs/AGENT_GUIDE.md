# ISB V100 Hub — Agent Guide

This is the operational context for AI coding agents, reviewers and contributors.

## Project identity

ISB (Intelligent Systems Bureau) V100 Hub is a V100-focused user-space control, compatibility, optimization and diagnostics platform.

The product goal is to make a Tesla V100 understandable and useful on a modern workstation by combining:

- real hardware/runtime detection;
- V100 telemetry and supported controls;
- diagnostics and FixEngine workarounds;
- performance profiles;
- game/application compatibility;
- OptiScaler integration;
- graphics/render-path configuration where the OS and driver actually permit it;
- lightweight Control Center and CLI;
- verification and deterministic reports.

ISB is not allowed to pretend that functionality supplied by the installed NVIDIA/base driver is an ISB-owned driver feature.

## Read before coding

Recommended reading order:

1. README.md
2. docs/MASTER_SPEC.md
3. docs/AGENT_GUIDE.md
4. docs/PRODUCT_REQUIREMENTS.md
5. docs/CURRENT_IMPLEMENTATION.md
6. docs/ARCHITECTURE.md
7. docs/IMPLEMENTATION_PLAN.md
8. docs/TASKS.md
9. docs/COMPONENT_TASKS.md
10. target module source and tests

MASTER_SPEC.md is the consolidated product/technical context. It describes intended behavior; source code, CI and CURRENT_IMPLEMENTATION.md remain authoritative for what is actually implemented.

Never infer implementation from a directory name alone.

## V100-only scope

Primary target: Tesla V100 SXM2 16 GB.

Additional qualification targets: V100 SXM2 32 GB and V100 PCIe.

Do not silently expand hardware support to GeForce, RTX, CMP, P100 or unrelated NVIDIA architectures.

Generic interfaces are allowed, but concrete hardware semantics are V100/GV100-specific.

## Three capability layers

### Hardware capability

What physically exists on GV100: Volta/SM70, Tensor Cores, HBM2 and ECC. V100 has no RT Cores, dedicated Optical Flow Accelerator, MIG or display outputs on the V100 module itself.

### Base-driver capability

What the installed NVIDIA/Google-compatible driver/runtime exposes on the current OS and API: CUDA, NVML, Vulkan, OpenGL, Direct3D/DXGI, DirectCompute and driver-specific paths.

A base-driver feature is not automatically an ISB feature.

### ISB capability

What ISB adds above the base stack: planning, orchestration, diagnostics, profiles, FixEngine workarounds, compatibility management, OptiScaler orchestration, telemetry presentation, verification, reports and experimental software features.

## Canonical architecture

The user-facing path is:

Panel or CLI -> Hub -> Providers -> Base driver / OS

The GUI and CLI must use the same Hub contracts.

Do not create a second GUI control plane, second FixEngine, second capability model, or direct provider calls from UI code.

CAL owns normalized capability schema. Providers own raw observations. Hub owns user-facing orchestration and capability projection.

## Status semantics

Unknown is not Available.

Use explicit states such as:

- AVAILABLE
- UNAVAILABLE
- UNKNOWN
- UNSUPPORTED
- PERMISSION_DENIED
- ERROR
- EXPERIMENTAL
- MOCK / SYNTHETIC

Never replace unknown telemetry with zero.

Mock data must be visibly synthetic.

## Mutation safety

Hardware/configuration changes follow:

READ -> VALIDATE -> PLAN -> USER REVIEW -> APPLY -> READ BACK -> VERIFY -> RESULT

Where possible:

BACKUP -> MUTATE -> VERIFY -> ROLLBACK ON FAILURE

No silent hardware tuning. No silent global game modification. Unknown anti-cheat or compatibility state defaults to manual review/dry-run.

## Optimize V100

Primary product workflow:

SCAN -> DETECT -> ANALYZE -> PLAN -> USER REVIEW -> APPLY -> VERIFY -> RESULT

The plan must show detected facts, recommended changes, skipped items, risks, privileges, restart/reset requirements and verification steps.

Optimize V100 is not a hidden Gaming preset.

## Profiles

Target profiles:

- Balanced
- Gaming
- Compute
- AI/Tensor
- Maximum Performance
- Low Power
- Custom

Profiles are policy templates, not guarantees of optimal performance. Only provider-supported controls may be applied.

## Graphics and render path

The intended use case is often a headless V100 used as a compute/render GPU while another GPU or iGPU drives the display.

Agents must distinguish rendering GPU, display GPU, compute GPU, application-selected adapter and OS routing.

Automatic render-path configuration is desirable where the OS and driver expose a documented, safe mechanism.

On Windows investigate real DXGI/WDDM/Graphics Settings and documented driver mechanisms. On Linux investigate PRIME render offload, Vulkan device selection, X11/Wayland routing and documented environment variables.

Do not invent registry keys or claim universal GPU routing without evidence.

## Graphics enhancement

Possible features include spatial scaling, Lanczos, CAS-style sharpening, dynamic resolution, frame pacing, FSR, XeSS, OptiScaler, experimental neural reconstruction, frame interpolation and software lighting/ray techniques.

Always distinguish native hardware, base-driver capability, external compatibility layer, ISB software and experimental features.

Never call software reconstruction native DLSS. Never claim RT or Optical Flow hardware on V100.

## OptiScaler

OptiScaler is an external managed component.

ISB may provide detection, version/provenance, compatibility evaluation, per-game configuration, install/update/remove, backup, hash verification, rollback and logs.

Do not rewrite OptiScaler inside the driver core. If redistribution is ever enabled, preserve upstream license and provenance obligations. Do not silently download arbitrary binaries.

## GUI product intent

Preferred GUI technology when no repository decision overrides it: Qt 6 Widgets.

Do not use Electron or Chromium.

The Control Center should feel like a specialized ISB V100 Control Center: dark, clean, technical, professional and readable.

Primary areas:

- Home
- Performance
- Games
- Tools

Prominent action: Optimize V100.

## Games

Potential discovery sources:

- Steam
- Epic
- GOG
- standalone

Only implement an integration when a real boundary/provider exists or the task explicitly adds one.

Never silently modify online or anti-cheat applications.

## Testing

A feature is not complete because it compiles.

Where applicable require:

- positive path;
- negative path;
- unknown/error path;
- deterministic tests;
- provenance;
- rollback or explicit non-reversible declaration;
- post-operation verification;
- report/log coverage.

Compilation is not hardware qualification.

## Reverse engineering

For important discoveries record source/version, hardware variant, OS, observation, experiment, confidence and reproduction status.

Evidence priority:

1. direct hardware measurement;
2. public technical documentation;
3. open-source implementations;
4. controlled observation;
5. legally appropriate binary analysis;
6. inference.

Inference is not a hardware fact.

## Proprietary software

Do not copy or redistribute proprietary NVIDIA source, headers, SDKs or binaries without a documented legal basis.

External drivers may be detected, configured or used as baselines without becoming ISB-owned implementations.

## Coding style

Prefer C++17, RAII, const-correctness, dependency injection, narrow interfaces, deterministic serialization, meaningful errors and small testable components.

Avoid giant monolithic classes and duplicated business logic.

## When the repository disagrees with this guide

Source code and CI are authoritative for implementation state.

If this guide says a feature exists but source/CI does not demonstrate it:

1. mark it as missing or not verified;
2. inspect existing boundaries;
3. do not fabricate APIs;
4. implement the smallest correct boundary;
5. update CURRENT_IMPLEMENTATION.md.

## Definition of done

Final reports must include:

- files changed;
- interfaces reused;
- new interfaces/adapters;
- real vs mock behavior;
- build result;
- test result;
- warnings;
- hardware dependencies;
- OS/driver dependencies;
- remaining limitations.

Compilation alone is never sufficient.
