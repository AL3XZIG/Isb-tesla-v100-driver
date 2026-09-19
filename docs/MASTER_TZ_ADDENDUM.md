# ISB V100 — Master TZ Addendum / Verified Requirements

Revision: 2026-09-19

This document consolidates requirements that were accumulated after the original product specification and verifies them against the current repository documentation and external technical references.

It is an addendum to:
- MASTER_SPEC.md
- PRODUCT_SPEC.md
- IMPLEMENTATION_PLAN.md
- DRIVER_AND_RELEASE_PIPELINE.md
- RELEASE_BUILDER.md
- RENDER_PATH.md
- PRODUCT_REQUIREMENTS.md

It does not override source code or CI when describing implementation state.

## 1. Product boundary

ISB is a user-space V100 Control Center and compatibility/optimization platform.

Canonical path:

User -> CLI/Qt Control Center -> Hub -> Providers/Integrations -> installed NVIDIA/Google base driver -> V100

ISB is not the base NVIDIA driver.

The normal ISB installer must install ISB itself and must not silently replace the installed NVIDIA/Google driver.

Independent KMD/UMD/alternative-driver work remains a separate research track.

## 2. Capability truth

Every capability must be classified as:

- hardware;
- base-driver;
- runtime/API;
- observed;
- verified;
- ISB software/compatibility;
- experimental;
- mock.

States:

- AVAILABLE
- UNAVAILABLE
- UNKNOWN
- UNSUPPORTED
- PERMISSION_DENIED
- ERROR
- EXPERIMENTAL
- MOCK

UNKNOWN must never be converted into UNSUPPORTED or AVAILABLE.

For V100, never claim native:

- RT Cores;
- DLSS hardware;
- dedicated Optical Flow Accelerator;
- MIG.

Software graphics enhancement must be presented as software.

## 3. Graphics Enhancement Layer

The project includes an experimental ISB Graphics Enhancement Layer.

Its purpose is not to reproduce proprietary NVIDIA DLSS, but to provide V100-compatible software graphics functionality.

### Stable/low-risk candidates

- FSR1-style spatial upscaling;
- Lanczos;
- CAS-style sharpening;
- dynamic resolution;
- frame pacing;
- image comparison.

### Experimental neural path

Potential pipeline:

CUDA -> Tensor Cores -> ONNX Runtime/TensorRT -> neural model -> reconstructed image

Potential research models:

- small Real-ESRGAN;
- FSRCNN;
- custom CNN.

This is an ISB/software inference path, not native DLSS.

### Experimental frame interpolation

Potential research:

- RIFE;
- FILM;
- other interpolation models.

Must expose latency/quality limitations and never be labeled DLSS Frame Generation.

### Experimental software ray/lighting

Potential:

- SSR;
- SSAO;
- SSGI;
- voxel lighting;
- CUDA software ray tracing;
- ray marching.

These are software techniques, not RT Core acceleration.

### Graphics module contract

Prefer backend-independent interfaces with Vulkan-first experimentation and optional CUDA acceleration.

Candidate organization:

graphics/capability
graphics/backend
graphics/upscaler
graphics/tests

Actual repository structure must be audited before creating duplicate directories.

Graphics experiments require:

- deterministic tests;
- benchmark;
- image comparison;
- JSON report;
- provenance;
- explicit experimental state.

## 4. OptiScaler

OptiScaler is an external integration.

ISB may provide:

- discovery;
- version detection;
- compatibility check;
- per-game configuration;
- backup;
- install;
- update;
- verify;
- rollback;
- hashes;
- provenance.

Boundary:

ISB -> OptiScaler adapter -> external OptiScaler

Do not copy OptiScaler implementation into ISB core.

Do not silently download or execute arbitrary binaries.

## 5. Game compatibility

Game manager is a compatibility/profile manager, not a launcher.

Potential discovery:

- Steam;
- Epic;
- GOG;
- standalone.

Game record:

- executable;
- API;
- compatibility;
- upscaler;
- OptiScaler;
- profile;
- backup;
- rollback;
- anti-cheat state.

Anti-cheat state:

- PRESENT;
- ABSENT;
- UNKNOWN.

UNKNOWN must default to manual/dry-run.

No anti-cheat bypass.

## 6. Multi-GPU/headless V100 render path

The V100 may be used as a compute/render adapter while another GPU/iGPU drives display.

Roles:

- DisplayOnly;
- ComputeOnly;
- ComputeAndDisplay;
- Unknown.

Detection should correlate:

- NVML;
- Vulkan;
- DXGI on Windows;
- DRM/session state on Linux.

Linux may plan application-scoped offload using documented mechanisms such as:

- __NV_PRIME_RENDER_OFFLOAD=1;
- __GLX_VENDOR_LIBRARY_NAME=nvidia;
- __VK_LAYER_NV_optimus=NVIDIA_only.

Windows must use real DXGI/D3D/WDDM evidence.

ISB must not silently change TCC/WDDM.

Current render-path implementation must refuse unsupported host mutation rather than pretending configuration succeeded.

## 7. Driver Manager

Driver Manager is a first-class lifecycle subsystem.

Providers:

- NVIDIA;
- Google;
- local cache.

Responsibilities:

- detect installed driver;
- discover releases;
- list versions;
- filter by provider/branch/OS/architecture/GPU;
- parse release URLs;
- parse direct artifact URLs;
- normalize metadata;
- download;
- verify;
- cache;
- expose provenance;
- optionally install through a separate explicit backend.

NVIDIA and Google versions must remain separate namespaces.

## 8. Driver URL parser

The parser must accept:

- direct artifact URLs;
- release-page URLs;
- redirects;
- query parameters;
- vendor-specific release pages.

It must extract where possible:

- provider;
- vendor;
- branch/family;
- version;
- release date;
- OS;
- architecture;
- package type;
- GPU family;
- source URL;
- final download URL;
- checksum;
- signature;
- provenance.

Missing/ambiguous metadata becomes UNKNOWN.

Parser failure must be explicit.

No metadata may be fabricated from an ambiguous filename.

## 9. Driver catalog and compatibility

A catalog may contain:

- provider;
- branch;
- version;
- OS;
- architecture;
- GPU family;
- source URL;
- download URL;
- release date;
- checksum;
- signature;
- compatibility state.

Compatibility states:

- COMPATIBLE;
- INCOMPATIBLE;
- UNKNOWN;
- LEGACY;
- EXPERIMENTAL;
- INSTALLED.

Do not rank releases as best/worst without an explicit evidence-based project policy.

### Verification correction

NVIDIA's current Data Center documentation exposes release information suitable for automation, including a machine-readable release data source, and its current CUDA/driver/architecture matrix lists Volta's last driver support branch as R580. Therefore the project must not hard-code an older conversation-derived version such as 582.16 as universally valid. The parser/catalog must resolve compatibility from current official provider data and exact OS/GPU context. citeturn0search0turn0search1

NVIDIA's current driver installation documentation also distinguishes operating-system families and x86_64/aarch64 or amd64/arm64 targets, so OS and architecture are mandatory catalog dimensions. citeturn0search2

NVIDIA's vGPU documentation currently lists V100 variants under supported vGPU software history, including V100 SXM2 and PCIe variants, which further supports keeping exact V100 variant and provider/branch metadata rather than treating every V100 as identical. citeturn0search4turn0search5

## 10. Safe driver download

Pipeline:

DISCOVER -> SELECT -> SHOW METADATA -> USER CONFIRMATION -> TEMP DOWNLOAD -> SIZE CHECK -> SHA256 -> SIGNATURE WHEN AVAILABLE -> ATOMIC CACHE MOVE -> PROVENANCE -> RESULT

Rules:

- trusted source;
- HTTPS where supported;
- temporary files;
- no partial artifact accepted;
- checksum mismatch rejects artifact;
- no automatic execution;
- download is not installation.

Cache identity must include provider, version, OS, architecture, package type and checksum.

## 11. Driver installation

Separate operation:

DETECT -> COMPATIBILITY -> PLAN/RISKS -> BACKUP/RECOVERY -> USER APPROVAL -> INSTALL -> RE-DETECT -> VERIFY -> REPORT

The normal ISB installer does not replace the base driver.

Third-party proprietary NVIDIA/Google binaries must not be redistributed in ISB releases without a legal basis.

## 12. Telemetry

Unified snapshot:

- utilization;
- HBM/VRAM;
- temperature;
- power;
- power limit;
- SM clock;
- memory clock;
- ECC;
- PCIe;
- NVLink;
- processes where permitted;
- throttling/performance state;
- driver/runtime;
- timestamp.

Bounded history:

- temperature;
- utilization;
- power;
- clocks.

## 13. Tuning

Potential provider-backed controls:

- Persistence Mode;
- Power Limit;
- Application Clocks;
- Memory Clocks where exposed;
- Compute Mode;
- other V100-supported management controls.

Transaction:

READ -> VALIDATE -> PLAN -> USER REVIEW -> APPLY -> READ BACK -> VERIFY -> RESULT

Rollback must preserve the previous state where technically possible.

No automatic overclocking.

## 14. FixEngine / Driver Doctor

FixEngine remains the single compatibility engine.

Pipeline:

environment/error evidence -> rule evaluation -> fix plan -> user review -> apply -> verify -> rollback/report

Rule evaluation must distinguish true/false/unknown.

No second diagnostic engine.

## 15. Optimize V100

Pipeline:

SCAN -> DETECT -> ANALYZE -> PLAN -> REVIEW -> APPLY -> VERIFY -> RESULT

The planner must expose skipped changes and reasons:

- unsupported;
- unknown;
- permission denied;
- driver dependency;
- restart required;
- unsafe;
- experimental.

## 16. Reports

Deterministic report bundle may contain:

manifest.json
gpu.json
driver.json
capabilities.json
performance.json
games.json
optiscaler.json
errors.json
logs/

Evidence must distinguish REAL, MOCK, SYNTHETIC, EXPERIMENTAL and UNKNOWN.

## 17. Control Center

Qt 6 Widgets is the preferred native frontend.

Pages should cover:

- GPU;
- Performance;
- Tuning;
- Optimization;
- Graphics;
- Drivers;
- Games/Tools;
- Diagnostics;
- Settings.

The GUI calls Hub only.

It must not call NVML/CUDA/Vulkan/DXGI directly.

The Drivers page provides:

- installed driver;
- provider;
- branch;
- version;
- available releases;
- compatibility state;
- download;
- verification;
- cache;
- official/source link;
- installation only when an actual backend exists.

## 18. CLI

Suggested families:

- status;
- info;
- capabilities;
- telemetry;
- telemetry --watch;
- tuning list/plan/apply/verify/rollback;
- optimize;
- diagnose/doctor;
- benchmark;
- report;
- graphics;
- render;
- drivers list/search/info/compatible/download/verify/cache.

Exact names must follow current CLI source.

## 19. Installer

ISB installer:

- installs ISB;
- detects prerequisites;
- configures runtime;
- installs documentation;
- provides uninstall/rollback;
- records logs.

It must not silently replace the base NVIDIA/Google driver.

## 20. Release Builder

Release Builder builds ISB, not third-party drivers.

Pipeline:

TAG -> CLEAN CHECKOUT -> CONFIGURE -> BUILD -> TEST -> PACKAGE -> SHA256 -> MANIFEST -> RELEASE NOTES -> GITHUB RELEASE -> UPLOAD

Initial artifacts:

Windows x64:
- .zip;
- .exe.

Linux x86_64:
- .deb;
- .tar.gz.

Names:

isb-<version>-<platform>-<arch>.<extension>

Every release also contains:

- SHA256SUMS;
- manifest.json;
- release notes;
- source commit/tag.

GitHub Actions officially supports storing build/test artifacts, passing them between jobs and validating artifact digests, matching the proposed multi-job release pipeline. citeturn0search7

Publication must be blocked if a required build/test/package/checksum/manifest stage fails.

Generated binaries must not be committed into main.

## 21. Release manifest

Fields:

- schema version;
- ISB version;
- tag;
- commit;
- platform;
- architecture;
- package format;
- file size;
- SHA-256;
- build/test result;
- toolchain where practical.

No unverifiable claims.

## 22. Release security

- least-privilege CI permissions;
- no untrusted PR code in privileged release jobs;
- no secrets in logs;
- artifact integrity checks;
- provenance;
- no unauthorized proprietary binary redistribution.

## 23. Offline mode

Offline functionality should include:

- inspection;
- telemetry;
- diagnostics;
- local profiles;
- reports;
- cached driver artifacts;
- cached game/OptiScaler state where available.

Internet-dependent functions:

- driver discovery;
- driver download;
- OptiScaler download;
- metadata updates;
- ISB updates.

## 24. Testing

Software-only tests must cover:

- common contracts;
- capability reconciliation;
- parser;
- driver metadata;
- downloader with fixtures;
- cache;
- checksums;
- manifests;
- tuning planner;
- rollback planner;
- render-path detection;
- graphics algorithms;
- reports;
- CLI;
- release packaging.

Mock providers must be deterministic and clearly marked synthetic.

Hardware qualification comes after the clean software gate.

## 25. Clean gate

Recommended baseline:

cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure
git diff --check

Then perform real V100 qualification.

## 26. AI coding-agent rules

Every coding agent must:

1. inspect the actual repository;
2. read the relevant docs;
3. search for existing implementations;
4. reuse contracts;
5. preserve Hub ownership;
6. keep GUI thin;
7. keep OptiScaler external;
8. keep external drivers external;
9. never invent V100 hardware capabilities;
10. preserve UNKNOWN semantics;
11. add deterministic tests;
12. run clean build/test;
13. update documentation;
14. distinguish implemented/mock/experimental/unavailable;
15. avoid rewriting the architecture without an explicit requirement.

## 27. Roadmap

1. Build/CMake integrity.
2. Common contracts and ownership.
3. Real read-only providers.
4. Capability engine.
5. Telemetry.
6. Tuning.
7. FixEngine/Doctor.
8. Optimize V100.
9. Games.
10. OptiScaler Manager.
11. Render path.
12. Graphics Enhancement Layer.
13. Reports/evidence.
14. CLI.
15. Qt Control Center.
16. Installer.
17. Driver providers/parser/downloader/cache.
18. Release Builder.
19. GitHub Release automation.
20. Documentation synchronization.
21. Final software test gate.
22. Real V100 qualification.
23. First production release.

## 28. Final acceptance criteria

The complete product must be able to:

- identify a real V100;
- expose evidence-backed capabilities;
- provide telemetry;
- plan/apply/verify supported controls;
- diagnose compatibility issues;
- manage profiles;
- manage games/applications;
- integrate external OptiScaler;
- detect/configure supported render paths;
- provide software graphics enhancement;
- discover NVIDIA/Google driver releases;
- parse release/download URLs;
- download and verify selected driver artifacts;
- maintain a provenance-aware cache;
- keep driver download separate from installation;
- build Windows ZIP/EXE;
- build Linux DEB/TAR.GZ;
- generate SHA256SUMS;
- generate a release manifest;
- publish GitHub Releases from tags;
- block broken releases;
- preserve clear boundaries between real, mock and experimental functionality.

No feature is considered stable solely because it compiles.
