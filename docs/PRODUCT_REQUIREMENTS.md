# ISB V100 Hub — Product Requirements

This document records the product the project owner wants to build. It is intentionally user-facing and complements ARCHITECTURE.md and IMPLEMENTATION_PLAN.md.

## Product promise

The product should answer:

1. What exactly is my V100 and what does the current environment expose?
2. What is wrong or suboptimal?
3. What can ISB safely change?
4. Did the change actually work?

Complexity should be hidden from normal users without hiding truth.

## Home

Show, where available:

- Tesla V100 identity;
- SXM2/PCIe variant;
- VRAM;
- architecture;
- compute capability;
- driver;
- CUDA;
- graphics API state;
- temperature;
- GPU utilization;
- HBM/memory utilization;
- power;
- clocks;
- ECC/health;
- active profile;
- detected problems.

Missing data must show Unknown, Unavailable, Permission denied or Error, not fake zeroes.

## Performance

Telemetry:

- utilization;
- memory/HBM utilization;
- temperature;
- power draw;
- power limit;
- SM clock;
- memory clock;
- VRAM used/free/total;
- ECC;
- PCIe;
- NVLink where present;
- active processes where permissions allow.

History:

- temperature;
- utilization;
- power;
- clocks.

Tuning controls appear only when current provider evidence says they are supported.

Potential controls:

- Persistence Mode;
- Application Clocks;
- Power Limit;
- Compute Mode;
- Auto Boost semantics where exposed.

Each control must show support, writability, current value, requested value, resulting value, verification, reversibility and restart/reset requirements.

## Profiles

Profiles:

- Balanced;
- Gaming;
- Compute;
- AI/Tensor;
- Maximum Performance;
- Low Power;
- Custom.

Selecting a profile creates a plan. It must not immediately mutate hardware.

Use:

Requested -> Planned -> Applied -> Verified

and show skipped/unsupported/unknown controls.

## Optimize V100

Primary workflow:

SCAN -> DETECT -> ANALYZE -> PLAN -> USER REVIEW -> APPLY -> VERIFY -> RESULT

The review must expose:

- detected facts;
- recommended changes;
- skipped items and reasons;
- risks;
- required permissions;
- restart/reset requirements;
- verification steps.

No mutation without explicit user approval.

## Driver Doctor

Provide:

- environment fingerprint;
- diagnostics;
- known issue detection;
- FixEngine plans;
- dry-run;
- explicit apply;
- verification;
- rollback;
- reports.

FixEngine remains the single authoritative workaround/rule engine.

## Games

Discover Steam/Epic/GOG/standalone games where reliable.

Game records should expose:

- executable;
- graphics API;
- compatibility;
- upscaler;
- OptiScaler;
- profile;
- backups;
- rollback;
- anti-cheat state.

Unknown compatibility or anti-cheat state must stay visible and default to manual/dry-run.

## OptiScaler Manager

OptiScaler remains external.

User-facing actions:

- Scan Games;
- Install;
- Update;
- Rollback;
- Verify.

Show:

- installed version;
- available version;
- compatibility;
- installation state;
- per-game state;
- backup state;
- rollback availability.

Do not expose upstream implementation complexity to ordinary users.

## Graphics Enhancement

Possible paths:

- application-native scaling;
- FSR;
- XeSS;
- OptiScaler;
- ISB spatial scaling;
- experimental neural reconstruction.

Experimental:

- frame interpolation;
- software ray/lighting techniques;
- denoising/reconstruction.

The UI must distinguish native hardware, base-driver feature, external compatibility layer, ISB software and experimental functionality.

## Render path

V100 may be headless and used as a compute/render GPU while another GPU or iGPU provides display output.

The product should detect adapters and provide a safe render-path plan where the current OS and driver support it.

Windows and Linux are first-class targets.

Universal routing must never be assumed.

## Tools

Tools should include:

### Driver Doctor

Diagnostics and FixEngine.

### Benchmark

CUDA, Tensor Core, HBM, PCIe/interconnect and graphics benchmarks where supported.

### Support Report

Deterministic support bundle.

## Appearance

Desired style:

- dark;
- clean;
- technical;
- professional;
- readable.

Optional customization may include wallpaper/background, accent color, density and appearance presets.

Do not turn the product into an Electron-like desktop environment.

## Offline/local use

Basic inspection, telemetry, diagnostics, profiles and local reports should work without Internet when local dependencies are available.

Network access should be explicit for downloads and updates.

Where licenses permit, external dependencies may be packaged or cached with clear provenance.

## Non-goals

Do not build:

- a fake NVIDIA driver;
- a replacement NVIDIA KMD/UMD as part of the Hub MVP;
- Electron/Chromium frontend;
- huge game launcher/database;
- silent overclocking;
- silent global game patching;
- anti-cheat bypass;
- fake native DLSS/RT claims;
- proprietary NVIDIA binary redistribution without legal basis.

Independent driver research may continue under research/alternative-driver.

## Success condition

The desired end state is a lightweight, real V100 Control Center that:

- detects a real V100;
- understands the current driver/runtime environment;
- safely manages supported controls;
- diagnoses compatibility problems;
- manages game/application profiles;
- integrates OptiScaler safely;
- supports render-path configuration where technically possible;
- provides useful graphics enhancement paths;
- produces evidence-backed reports;
- clearly separates stable, experimental, mock and unavailable functionality.
