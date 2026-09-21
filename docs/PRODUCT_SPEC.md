
# ISB V100 Hub — Product & Technical Specification

This document consolidates the current ISB V100 product requirements, architecture decisions, graphics/render-path work, optimization workflow, GUI requirements, provider boundaries, safety rules, and experimental feature policy.

**Important:** this is a specification. Implementation status must always be verified against current source, tests and CI.

## 1. Product identity

**ISB — Intelligent Systems Bureau / V100 Hub**

ISB is a lightweight, V100-focused user-space control, compatibility, optimization and diagnostics layer operating above an installed NVIDIA/Google-compatible driver stack.

ISB is not the NVIDIA kernel-mode driver, user-mode driver, firmware, CUDA runtime, NVML implementation, or replacement display driver.

The goal is to make Tesla V100 easier to install, understand, tune, diagnose, optimize and use for graphics/compute workloads without pretending to replace the NVIDIA base stack.

The independent KMD/UMD/GPUMM/alternative-driver research track remains isolated under research/alternative-driver and must never become a dependency of the stable Hub.

## 2. Product scope

Primary qualification target:

- Tesla V100 SXM2 16 GB / GV100 / SM70.

Variants must be detected rather than assumed:

- V100 SXM2 16 GB;
- V100 SXM2 32 GB;
- V100 PCIe 16 GB;
- V100 PCIe 32 GB.

Initial and supported OS scope:

- **Linux x86-64 only**.

Windows and BSD are outside the supported product scope.

The project is V100-focused and should not become a generic GPU suite unless a feature is directly required by the V100 product.

## 3. Architecture

The required user-facing path is:

**Panel → Hub → Providers**

GUI and CLI consume the same Hub contracts. Business logic is never duplicated between GUI and CLI.

The conceptual stack is:

User
→ Control Center / CLI
→ ISB Hub
→ Capability / Telemetry / Operations
→ Provider and Adapter Layer
→ NVML / CUDA / Vulkan / OpenGL / Linux DRM / PRIME
→ Base Driver Stack
→ V100 / GV100

The Hub is orchestration. Providers own provider-specific access. CAL owns normalized capability definitions. FixEngine owns diagnostic rules and plans. Verification owns post-operation verification. GUI owns presentation.

## 4. Capability model

ISB separates three layers.

### Hardware capability

Physical GPU features.

V100/GV100 provides:

- CUDA cores;
- first-generation Tensor Cores;
- FP16 acceleration;
- INT8 inference acceleration;
- HBM2;
- ECC.

V100 does not provide:

- RT Cores;
- dedicated Optical Flow Accelerator;
- MIG;
- physical display outputs.

The absence of display outputs is normal for a Tesla V100.

### Base-driver capability

Features exposed by the installed NVIDIA/Google-compatible stack on the current OS/API:

- CUDA;
- CUDA runtime;
- NVML;
- Vulkan;
- OpenGL;
- Direct3D/DXGI;
- DirectCompute;
- driver-specific render/offload mechanisms;
- management controls.

A feature exposed by the base driver is recorded as a base-driver capability, not as an ISB implementation.

### ISB capability

Functionality added above the base stack:

- capability aggregation;
- telemetry normalization;
- optimization planning;
- performance profiles;
- compatibility profiles;
- FixEngine workarounds;
- game discovery;
- OptiScaler orchestration;
- graphics enhancement;
- multi-GPU render-path planning;
- verification;
- reports;
- benchmarks;
- GUI;
- installer/release tooling.

## 5. Capability states

All capability-aware subsystems use explicit states:

- AVAILABLE;
- UNAVAILABLE;
- UNKNOWN;
- UNSUPPORTED;
- PERMISSION_DENIED;
- ERROR.

Unknown is not Available and is not Unsupported.

Unknown must never cause speculative mutation.

Never display fabricated zero values for unavailable telemetry. Use Unknown/Unavailable with a reason.

Synthetic data must be explicitly marked synthetic.

## 6. Providers

### NVML

NVML currently acts as a read-only observation source unless a real, verified mutation provider is introduced.

Relevant observations include:

- GPU identity;
- UUID/VBIOS;
- PCIe generation/width;
- memory;
- temperature;
- power draw/limit;
- graphics/memory/SM clocks;
- GPU/memory utilization;
- persistence;
- compute mode;
- performance state;
- ECC mode/errors;
- NVLink state.

A read-only observation must not be presented as a writable GUI control.

### CUDA

CUDA provider supplies runtime/capability observation and later compute functionality where explicitly implemented.

### Vulkan

Vulkan provider supplies graphics capability detection and provenance.

### Windows

Windows platform probing may use DXGI/D3D and driver-model information.

The V100 must not be treated as a display adapter simply because it is a GPU.

### Linux

Linux probing must distinguish DRM devices, display ownership, render devices, Vulkan devices and PRIME/offload environment.

## 7. Telemetry

Normalized Hub telemetry should cover:

- GPU utilization;
- memory/HBM utilization;
- temperature;
- power draw;
- power limit;
- SM clock;
- memory clock;
- VRAM used/free/total;
- ECC corrected/uncorrected;
- PCIe generation/width;
- PCIe traffic/errors where available;
- NVLink state;
- active process count;
- driver/runtime state;
- throttling/performance state.

Optional sampled history:

- temperature;
- utilization;
- power;
- clocks;
- throttling indicators.

The first version must not require a permanent telemetry daemon.

## 8. Tuning

Potential controls:

- Persistence Mode;
- Application Clocks;
- Power Limit;
- Compute Mode;
- Auto Boost;
- other V100-supported controls discovered by the provider.

Controls are capability-driven. The UI must not assume support.

Every control should expose:

- capability state;
- writable state;
- current value;
- requested value;
- resulting value;
- provider;
- driver dependence;
- persistence;
- reversibility;
- restart/reset requirement;
- verification state.

Canonical transaction:

READ CURRENT
→ VALIDATE REQUEST
→ CREATE PLAN
→ USER APPROVAL
→ APPLY
→ READ BACK
→ VERIFY

Canonical result:

**Requested → Applied → Verified**

No silent hardware changes.

## 9. Performance profiles

Profiles:

- Balanced;
- Gaming;
- Compute;
- AI/Tensor;
- Maximum Performance;
- Low Power;
- Custom.

A profile is a policy template, not a guarantee of optimal performance.

Home should show the current profile and a compact Change Profile action. Detailed controls belong in TUNING.

Profile application always follows plan → review → approval → apply → verify.

## 10. Optimize V100

Primary workflow:

**SCAN → DETECT → ANALYZE → PLAN → USER REVIEW → APPLY → VERIFY → RESULT**

Scan collects exact GPU, OS, driver, CUDA/NVML/API state, telemetry, applications/games, FixEngine evidence, OptiScaler state and render/display topology.

Detect determines available, unsupported, unknown and permission-limited capabilities.

Analyze correlates evidence with known issues, FixEngine rules, performance profiles, graphics compatibility and render-path requirements.

Plan shows:

- proposed operation;
- subsystem;
- availability;
- risk;
- required privilege;
- restart/reset requirement;
- rollback path;
- verification steps.

The user must see Detected, Recommended Changes, Skipped and Risks before applying.

No mutation occurs without explicit approval.

## 11. Driver Doctor / FixEngine

The existing FixEngine is the canonical rule engine.

Pipeline:

Fingerprint
→ Environment/Error Evidence
→ FixEngine
→ Candidate Rules
→ Dry Run
→ User Approval
→ Apply
→ Verify
→ Rollback when possible

False, Unknown and True must remain distinct.

Unknown predicates never become planned actions.

Each diagnosis records evidence, rule, state/confidence, action, privilege, rollback and verification method.

No second GUI/CLI diagnostic engine is permitted.

## 12. Multi-GPU render/display architecture

This is a fundamental V100 decision.

A Tesla V100 has no physical HDMI/DP/DVI outputs.

ISB therefore distinguishes:

- Compute GPU;
- Render GPU;
- Display GPU.

Typical configuration:

V100 = Compute/Render GPU
iGPU or secondary dGPU = Display GPU

The lack of display outputs is not a hardware fault.

### Render-path detection

Detect:

- all GPUs;
- GPU roles;
- display ownership;
- render capability;
- CUDA;
- Vulkan;
- OpenGL;
- DirectX;
- Windows driver model;
- Linux DRM/PRIME;
- X11/Wayland/Xwayland where relevant.

### Render-path planning

Possible Linux mechanisms:

- PRIME Render Offload;
- Vulkan device selection;
- OpenGL vendor selection;
- per-application environment/configuration.

Linux examples may use NVIDIA PRIME/Vulkan environment variables, but they are examples only. Applicability must be detected.

### Safety

ISB must not automatically:

- switch TCC ↔ WDDM;
- disable a display adapter;
- replace the base driver;
- alter driver signing;
- blindly change global GPU ownership.

Prefer per-application render configuration.

### Verification

Render-path verification has three levels:

1. Configuration Verified;
2. Render Device Verified;
3. Runtime Verified.

A configuration file existing is not proof that the application rendered on the V100.

## 13. Graphics Enhancement Layer

ISB Graphics is a compatibility/enhancement layer, not a proprietary NVIDIA feature clone.

Conceptual path:

Game/Application
→ ISB Graphics Layer
→ resolution scaling / upscaling / sharpening / DRS / frame pacing / HDR / VSync-FPS / integrations
→ Vulkan / DX12 / DX11 / OpenGL
→ V100 render/compute GPU

### Spatial upscaling

Supported design targets:

- FSR1-style spatial scaling;
- NIS-style scaling where appropriate;
- Lanczos;
- CAS-style sharpening.

These do not require DLSS hardware.

### AI upscaling

V100 Tensor Cores may be used for user-space neural reconstruction.

Possible approaches:

- CUDA;
- TensorRT;
- ONNX Runtime with CUDA;
- small Real-ESRGAN/FSRCNN-like models;
- custom CNNs.

This must be called ISB software AI upscaling, not native DLSS.

### Frame interpolation

Experimental research may use:

- RIFE;
- FILM;
- custom CUDA/Tensor pipelines.

Risks include latency, artifacts and synchronization.

This is not DLSS 3 Frame Generation.

### Software rendering / ray research

Possible experimental features:

- SSR;
- SSAO;
- SSGI;
- voxel lighting;
- CUDA ray marching;
- software ray tracing.

These are software techniques and must not be presented as native RT hardware.

## 14. DLSS policy

Never claim native DLSS support on V100.

V100 lacks RT Cores and a dedicated Optical Flow Accelerator.

Compatibility/injection paths may be researched, but they must be explicitly labeled compatibility, proxy, external integration or experimental.

## 15. OptiScaler

OptiScaler remains an external component.

ISB responsibilities:

- installed version detection;
- compatibility evaluation;
- install/update/remove;
- backup;
- hash verification;
- rollback;
- per-game configuration;
- logs;
- provenance/license metadata.

Safe deployment:

SCAN
→ DETECT
→ COMPATIBILITY
→ BACKUP
→ PLAN
→ USER APPROVAL
→ DEPLOY
→ VERIFY
→ RESULT / ROLLBACK

Never silently modify online/anti-cheat games.

Never silently download arbitrary binaries.

Do not copy the OptiScaler implementation into ISB.

## 16. Game Manager

Game Manager is not a launcher.

Discovery targets:

- Steam;
- Epic;
- GOG;
- standalone.

Where possible detect executable, graphics API, upscaler/runtime signals, OptiScaler state and compatibility.

Compatibility states:

- AVAILABLE;
- UNAVAILABLE;
- UNKNOWN.

Anti-cheat states:

- PRESENT;
- ABSENT;
- UNKNOWN.

Unknown compatibility or anti-cheat must default to dry-run/manual review.

## 17. Graphics API integration policy

Prefer:

1. documented application configuration;
2. documented loader/layer/plugin mechanisms;
3. external compatibility components;
4. isolated experimental interception;
5. binary patching only as an explicitly experimental last resort.

No proprietary NVIDIA binary modification is part of the stable product.

## 18. Control Center GUI

Preferred technology when no existing framework overrides it:

**Qt 6 Widgets**

Do not use Electron, Chromium, a web frontend or a browser shell.

GUI should be optional in CMake and must consume Hub contracts.

### Navigation

Persistent narrow sidebar:

1. GPU
2. TUNING
3. OPTIMIZATION
4. GRAPHICS

Tools such as Driver Doctor, Benchmark and Reports are actions/sections inside these control surfaces rather than unnecessary top-level navigation.

### Global header

Compact status:

- GPU;
- temperature;
- utilization;
- power;
- clocks;
- driver state;
- active performance profile.

### GPU page

Display exact identity, variant, architecture, compute capability, VRAM/HBM, ECC, power/TDP, PCIe, NVLink, Tensor Cores, RT Cores state, Optical Flow state, CUDA, NVML, Vulkan and DirectX/OpenGL state where exposed.

### TUNING page

Only provider-supported controls.

Show current, requested, capability state, writable/reversible state, driver dependence, restart/reset requirement and verification.

### OPTIMIZATION page

Separate Games and Applications.

Show executable, API, profile, upscaler, OptiScaler state, compatibility, proposed changes, risks and backup/rollback state.

### GRAPHICS page

Expose only capabilities actually available to the selected application/backend.

Native hardware support and software compatibility paths must be visually distinct.

## 19. Appearance / Linux Rice

Appearance is separate from hardware tuning.

Default style:

- dark;
- technical;
- professional;
- compact;
- V100-oriented;
- low visual noise;
- no acid RGB.

Appearance presets:

- ISB Default;
- Industrial;
- Minimal;
- Terminal;
- Laboratory;
- Linux Rice;
- High Contrast.

Customizable:

- wallpaper;
- wallpaper mode;
- accent;
- density;
- font size;
- sidebar compact/expanded;
- animation level;
- graph style;
- background effects;
- transparency/blur where inexpensive.

Performance profiles and appearance profiles are independent.

Appearance changes must never change hardware state.

## 20. Installer

Installer responsibilities:

- prerequisite detection;
- OS detection;
- base-driver detection;
- ISB installation;
- configuration;
- verification;
- rollback/uninstall;
- logs.

The ISB installer must not automatically replace the NVIDIA base driver.

Driver installation is a separate privileged boundary.

## 21. Driver lifecycle

Potential driver providers:

- NVIDIA Data Center/Tesla;
- NVIDIA vGPU/vWS/GRID where applicable;
- Google-provided NVIDIA packages where applicable;
- local cache.

Driver provenance must preserve vendor, branch, version, OS, architecture, package type, source and checksum/signature when available.

Workflow:

DISCOVER
→ SELECT
→ USER CONFIRMATION
→ DOWNLOAD
→ CHECKSUM/SIGNATURE VERIFY
→ CACHE
→ REPORT

Downloaded drivers must never execute automatically.

## 22. Release engineering

Windows artifacts:

- .zip;
- .exe.

Linux artifacts:

- .deb;
- .tar.gz.

Naming:

isb-<version>-<platform>-<arch>.<extension>

Release metadata:

- source commit/tag;
- platform;
- architecture;
- package type;
- file size;
- SHA256;
- provenance.

Target pipeline:

tag
→ clean checkout
→ configure
→ build matrix
→ tests
→ package matrix
→ checksums
→ manifest
→ release notes
→ GitHub Release
→ artifacts

Failed required build/test/package/manifest jobs must block publication.

Third-party driver binaries remain outside the repository unless redistribution is explicitly legal.

## 23. Reports and evidence

Canonical report bundle:

manifest.json
gpu.json
driver.json
capabilities.json
games.json
optiscaler.json
performance.json
errors.json
logs/

Include:

- schema version;
- ISB version/commit;
- generation time;
- environment hash;
- exact GPU;
- driver/runtime provenance;
- operation history;
- synthetic/real markers;
- verification state.

Support baseline → candidate → verified result comparison.

## 24. Testing

Test:

- Unknown handling;
- capability gating;
- profile preview;
- optimization plan presentation;
- failed operation presentation;
- rollback presentation;
- render-path classification;
- render-path planning;
- upscaler capability state;
- game compatibility;
- OptiScaler deployment plan;
- report determinism.

Normal build:

rm -rf build
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel
ctest --test-dir build --output-on-failure

NVML-disabled build:

rm -rf build-nvml-disabled
cmake -S . -B build-nvml-disabled -DBUILD_TESTING=ON -DISB_ENABLE_NVML=OFF
cmake --build build-nvml-disabled --parallel
ctest --test-dir build-nvml-disabled --output-on-failure

Also run git diff --check and configured warnings/static analysis.

## 25. Mock mode

Mock mode is required while real providers are incomplete.

Mock mode must:

- use deterministic fixtures;
- mark synthetic data;
- exercise the same Hub/UI contracts;
- never perform real hardware mutation;
- visibly identify DEMO / MOCK DATA.

## 26. Error handling

Optional backend failure must not crash the GUI.

All backend states are rendered explicitly:

- Available;
- Unknown;
- Unavailable;
- Unsupported;
- Permission denied;
- Error.

No fabricated zero values.

No fabricated successful operations.

No silent conversion of real provider failure into mock success.

## 27. Ownership and dependency rules

Ownership:

- common: common foundation primitives;
- CAL: normalized capability schema;
- providers: provider-specific observations;
- Hub: user-facing orchestration;
- FixEngine: diagnostic rules/plans;
- verification: verification;
- graphics: graphics contracts/adapters;
- game manager: game discovery/compatibility;
- OptiScaler manager: external integration boundary;
- GUI: presentation;
- CLI: command parsing/output.

Forbidden:

- GUI directly calling NVML/CUDA;
- GUI implementing FixEngine rules;
- CLI duplicating Hub logic;
- Hub duplicating provider internals;
- duplicate render-path models;
- circular dependencies such as hub → graphics → hub;
- fake include-path fixes that hide incorrect target ownership;
- duplicate control planes.

Public headers must be self-contained and intentional about dependencies.

## 28. Current implementation boundary

The repository already contains the C++17/CMake foundation, common/CAL foundations, Hub control-plane contracts, read-only provider work, FixEngine/diagnostic foundations, graphics/upscaler contracts, render-path contracts/platform probing, optional GUI foundation and mock/unavailable paths.

The following must remain clearly marked as incomplete unless verified by current source/CI/hardware:

- provider-backed hardware tuning/apply;
- complete render-path deployment;
- complete render-path verification/rollback;
- complete game optimization;
- complete OptiScaler deployment;
- native DLSS on V100;
- native frame generation on V100;
- independent NVIDIA driver functionality;
- physical V100 qualification without hardware evidence.

A UI or interface existing does not make the underlying operation implemented.

## 29. Experimental boundary

Experimental features may include:

- neural super-resolution;
- AI denoising;
- frame interpolation;
- software ray tracing;
- CUDA ray marching;
- advanced Vulkan/DX12 interception;
- render-path experiments.

Experimental code must be isolated, explicitly labeled, tested separately and prevented from silently activating in stable paths.

## 30. Development order

Recommended sequence:

0. Build/CMake integrity
1. Common contracts
2. CAL/capability ownership
3. Read-only providers
4. Telemetry
5. Hub status
6. Tuning backend
7. FixEngine integration
8. Optimize V100
9. Game compatibility
10. OptiScaler
11. Graphics backend
12. Multi-GPU render-path deployment
13. Reports/evidence
14. CLI completion
15. Qt Control Center
16. Appearance layer
17. Installer
18. Driver lifecycle
19. Release automation
20. Pre-test code freeze
21. Test gate
22. Real V100 qualification
23. Experimental graphics/AI features
24. Production release

## 31. Definition of done

A feature is not stable because it compiles.

It requires:

1. capability detection;
2. positive path;
3. negative/unsupported path;
4. unknown/error path;
5. provenance;
6. deterministic behavior;
7. rollback or explicit non-reversible declaration;
8. post-operation verification;
9. report/log coverage;
10. no false V100 hardware claims;
11. clean CMake integration;
12. tests in normal and optional-dependency configurations.

## 32. Rules for AI coding agents

Before coding, read:

- README.md;
- docs/ARCHITECTURE.md;
- docs/TASKS.md;
- docs/COMPONENT_TASKS.md;
- docs/IMPLEMENTATION_PLAN.md;
- docs/PRODUCT_SPEC.md;
- relevant CMake files;
- public headers;
- relevant tests.

Agents must inspect the real repository first, never invent APIs, reuse existing contracts, create adapters only where a backend boundary is actually missing, preserve C++17/CMake conventions, verify dependency direction, run clean builds/tests and report exactly what is real, mock, unavailable or experimental.

## 33. Source-of-truth hierarchy

When documentation conflicts:

1. current source and tests;
2. current CMake target graph;
3. current CI;
4. docs/ARCHITECTURE.md;
5. docs/TASKS.md;
6. docs/IMPLEMENTATION_PLAN.md;
7. docs/PRODUCT_SPEC.md;
8. historical prompts and roadmaps.

Historical ideas are not automatically current requirements.
