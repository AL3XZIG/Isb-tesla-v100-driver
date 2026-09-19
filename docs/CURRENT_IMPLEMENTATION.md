# ISB V100 Hub — Current Implementation Boundary

This snapshot is deliberately conservative. Source code and CI are authoritative if they disagree with this document.

## Current direction

The repository has moved from the original Driver Fixer concept toward a V100-focused user-space Hub/control plane above the installed NVIDIA/base driver stack.

The independent KMD/UMD/alternative-driver effort remains research-only.

## Established ownership

Documented stabilization work established:

- common/ as owner of implemented common primitives such as Status, Result<T> and ErrorCode;
- CAL as owner of normalized capability schema;
- providers as owners of raw runtime observations;
- Hub as owner of user-facing orchestration and capability projection;
- CLI and GUI as Hub clients;
- driver lifecycle and release tooling as separate product subsystems above the installed base driver.

Do not introduce competing ownership without an explicit architecture change.

## Hub

Hub is the canonical user-facing control plane.

Core concepts include:

- inspect;
- status;
- capabilities;
- telemetry;
- profiles/plans;
- optimize;
- apply;
- rollback;
- verify;
- benchmark;
- report;
- driver discovery/download/verification.

The mutation path is intentionally conservative until real providers can apply and read back settings.

## Provider model

Intended providers include:

- NVML;
- CUDA;
- Vulkan;
- Windows DXGI/D3D;
- Linux platform/runtime detection;
- NVIDIA driver source provider;
- Google driver source provider;
- local driver cache.

Optional providers must remain optional.

Unavailable/unknown/error states must be explicit.

## Driver lifecycle boundary

The project now documents a dedicated driver lifecycle subsystem.

Its responsibilities are:

- discover NVIDIA and Google driver releases;
- parse driver URLs and release pages;
- normalize version/vendor/OS/package/GPU metadata;
- download selected artifacts;
- verify checksums/signatures where available;
- maintain a local cache;
- expose driver information through Hub/CLI/Control Center;
- optionally support explicit installation workflows when a real installation backend exists.

This subsystem does not replace the installed NVIDIA/Google driver.

Download and installation are separate operations.

The detailed specification is:

docs/DRIVER_AND_RELEASE_PIPELINE.md

Important implementation-state rule: the specification is not proof that every provider, parser, downloader or installer is already production-complete. Source code and CI must be checked before marking an individual feature implemented.

## Control Center

Recent project work established a Qt6 Widgets Control Center direction.

The intended GUI is:

- lightweight native C++;
- Hub-backed;
- mock/development capable;
- explicit about Unknown/Unavailable;
- capability-aware;
- plan/review/apply/verify based.

Where a backend feature is not real, the UI must not pretend it is.

The planned GUI also contains a Drivers section for installed-driver information, release discovery, download/verification and cache management.

## Release engineering

ISB has a documented release-builder direction.

Target release artifacts:

Windows:
- .zip;
- .exe.

Linux:
- .deb;
- .tar.gz.

The release pipeline is intended to:

- build from a tagged commit;
- run tests;
- package each platform;
- generate SHA256SUMS;
- generate a release manifest;
- publish GitHub Releases;
- upload artifacts;
- block publication when required stages fail.

The detailed specification is in docs/DRIVER_AND_RELEASE_PIPELINE.md.

As with the driver lifecycle subsystem, release specification is not itself evidence that the full packaging/release automation is already implemented.

## Graphics/render path

The project contains a graphics/render-path direction for multi-GPU/headless V100 use.

Important user requirement:

The V100 may perform compute/render work while another GPU or iGPU drives the display.

This is a product requirement, not proof that every OS/driver/application supports such routing.

## OptiScaler

Recent project work advanced an OptiScaler integration boundary.

The intended architecture is:

ISB -> OptiScaler integration boundary -> external OptiScaler component

Do not move OptiScaler implementation into the core driver/hub.

Before claiming real installation/update/rollback/version selection, verify the current source and CI.

A mock or boundary is not equivalent to working deployment.

## Mock mode

Mock mode is a first-class development strategy because physical V100 qualification is not always available.

Mock values must be deterministic and visibly synthetic.

Mock success is not hardware success.

## Qualification

Qualification is evidence-gated:

- mock evidence is not physical V100 evidence;
- unavailable provider is not hardware success;
- unverified behavior remains unverified.

First real V100 qualification is a separate milestone.

## Build

The repository is C++17/CMake based.

Root CMake currently treats several providers/features as optional components.

When changing CMake or public headers, always perform a clean configure/build/test.

## Known gaps

Do not call these stable merely because a UI/interface/mock exists:

- real V100 tuning;
- universal Windows render routing;
- universal Vulkan/D3D compatibility;
- full game patching;
- complete OptiScaler deployment automation;
- complete driver discovery/download automation;
- complete release packaging/publication automation;
- native DLSS;
- native frame generation;
- RT hardware;
- Optical Flow hardware;
- independent NVIDIA-driver replacement.

## Recent implementation history

The repository documentation records stabilization work around foundation ownership, Hub canonicalization, capability ownership, qualification evidence gating, deterministic status, installer/qualification reporting and later GUI/graphics/OptiScaler work.

The driver lifecycle and release engineering specifications have now been added as dedicated documentation. Exact implementation status of each provider, parser, downloader, package target and release workflow must still be established from source and CI.

## Next-step rule

For every task:

1. inspect current code and tests;
2. compare against IMPLEMENTATION_PLAN.md and TASKS.md;
3. fix build/architecture regressions first;
4. finish real read-only evidence;
5. finish capability/verification boundaries;
6. add mutation only when the provider can read back state;
7. keep GUI thin;
8. treat driver discovery/download as a separate provider-backed lifecycle;
9. treat release packaging/publication as CI infrastructure;
10. update this document when implementation status changes.
