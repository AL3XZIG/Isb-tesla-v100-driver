# Superseded mainline specification

> This document is historical/reference material. The mainline product architecture was subsequently established as the V100-focused user-space ISB Hub/control plane. It must not be used as evidence for an independent production NVIDIA driver or a separate ControlPlane API. See `docs/ARCHITECTURE.md` and `docs/TASKS.md` for the current verified boundary.

# ISB Driver Fixer MVP

## Objective

Prove that ISB can improve a real NVIDIA V100 software stack without replacing its kernel driver.

## Definition of done

The MVP is complete when one real compatibility problem has all of the following:

1. exact GPU variant recorded;
2. exact OS/build recorded;
3. exact base-driver package/version recorded;
4. affected API/runtime recorded;
5. deterministic reproduction exists;
6. ISB diagnosis identifies the failure condition;
7. a reversible workaround is selected by a rule;
8. the workaround is applied through a supported user-space/configuration boundary;
9. the application/test passes after the change;
10. rollback restores the baseline;
11. an IDR/evidence manifest captures the before/after state;
12. the case becomes a regression test.

## First implementation order

### P0 — Fix Engine core

- environment model;
- base-driver fingerprint;
- rule representation;
- deterministic rule matching;
- workaround plan representation;
- dry-run mode.

### P0 — Driver/API probes

Start with read-only probes:

- GPU identity and exact variant;
- OS/build;
- NVIDIA driver version/package identity;
- CUDA availability/version;
- Vulkan availability/extensions;
- OpenGL state;
- Direct3D/DXGI state on Windows;
- DirectCompute availability on Windows where exposed by the base stack.

All probe results need provenance.

### P0 — First real bug

Do not invent a bug. Select one that can be reproduced on the target V100 system and capture the baseline with IDR.

Candidate classes:

- API path fails on one driver branch;
- application breaks on a known extension combination;
- a feature is exposed but unusable for a specific application;
- a driver/runtime combination needs a stable configuration workaround.

### P1 — Runtime shim

After the first configuration-only fix works, implement one narrow user-space interception boundary using a documented API/layer mechanism.

### P1 — Regression matrix

Run the same reproduction against several base-driver versions and record:

`GPU × OS × driver × API × application × workaround`

### P1 — Control Center

Only after the CLI workflow works:

`scan -> diagnose -> propose -> apply -> verify -> rollback -> report`

## Google/Compute driver strategy

A Google Compute Engine / vGPU-class NVIDIA package may be used as an experimentally supported **base stack** if it is actually compatible with the target V100 setup and exposes useful APIs such as DirectCompute.

ISB should not bundle that proprietary package. Instead:

1. user installs/supplies the base package;
2. ISB fingerprints it;
3. ISB verifies the APIs it exposes;
4. ISB selects compatible profiles/workarounds;
5. ISB adds user-space features above those APIs;
6. ISB records the exact combination for regression testing.

This allows ISB to exploit useful driver variants without pretending to be the underlying NVIDIA driver.

## What we are deliberately not doing yet

- custom Windows KMD;
- custom Linux KMD;
- GPUVM implementation;
- command submission implementation;
- replacement CUDA driver stack;
- replacement WDDM/DRM graphics stack;
- binary patching of proprietary driver files;
- universal game compatibility.

Those can remain research tracks while the Fixer becomes useful much sooner.
