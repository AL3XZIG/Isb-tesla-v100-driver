# ISB V100 Hub — Installation, Usage and Testing Guide

This guide is the practical entry point for building and testing ISB.

ISB is a V100-focused user-space control, compatibility, optimization and diagnostics hub. It works above an installed NVIDIA driver stack and does not replace the NVIDIA kernel/user-mode driver.

## 1. Current implementation status

The repository is being built in stages.

### Present on `main`

- C++17 project foundation and CMake build.
- CAL v1 capability abstraction.
- V100/GV100 capability model and capability definitions.
- Architecture and implementation roadmap under `docs/`.
- Existing diagnostics/verification foundations.
- The repository is designed so the GUI and CLI use one control-plane API rather than duplicating business logic.

### Headless Hub baseline

The current development baseline also contains a headless Hub/CLI implementation in the pending Hub backend work. It adds:

- shared Hub contracts for provider modes, provenance, environment, telemetry, controls, operations/plans, apply/verification results, diagnostics, benchmarks and reports;
- deterministic synthetic V100 provider data;
- an unavailable-provider path;
- capability-aware planning and safe no-op behavior when a provider does not expose a control;
- a headless `isb` CLI with `status`, `inspect`, `capabilities`, `telemetry`, `profile`, `optimize`, `diagnose`, `verify`, `benchmark` and `report` operations;
- automated regression tests for synthetic telemetry, capabilities, unknown controls, planning/apply/verification behavior and report generation.

If that Hub backend commit has not yet been merged into your checkout, the commands in sections 4–6 that use `build/cli/isb` will become available after the Hub baseline is merged.

## 2. What is deliberately not implemented yet

The first build is a safe headless/mock baseline. It does **not** magically provide real V100 hardware control.

Still pending for the production path:

- real NVML provider;
- real CUDA provider;
- Vulkan/DXGI/D3D probing;
- real Windows/Linux platform providers;
- real hardware tuning and mutation;
- game discovery;
- OptiScaler file lifecycle and integration;
- full FixEngine/IDR integration into the Hub transaction path;
- real graphics enhancement backends;
- real benchmark implementations;
- finished lightweight Qt Control Center.

The mock provider must never be interpreted as real hardware telemetry or successful hardware verification.

## 3. Requirements

### Linux

Recommended baseline:

- CMake 3.16 or newer;
- a C++17-capable compiler such as GCC or Clang;
- standard build tools (`make` or Ninja);
- Git.

No CUDA Toolkit, NVIDIA GPU or Qt installation is required for the headless mock build.

### Windows

Recommended baseline:

- CMake 3.16 or newer;
- Visual Studio 2022 with C++ desktop development tools, or another C++17-capable toolchain;
- Git.

The initial headless build does not require an installed V100 or Windows SDK-specific NVIDIA integration.

## 4. Build from source

Clone the repository and enter it:

```bash
git clone https://github.com/AL3XZIG/Isb-tesla-v100-driver.git
cd Isb-tesla-v100-driver
```

Configure:

```bash
cmake -S . -B build -DBUILD_TESTING=ON
```

Build:

```bash
cmake --build build --parallel
```

On a multi-configuration generator such as Visual Studio, use:

```powershell
cmake --build build --config Release --parallel
```

## 5. Run the automated tests

Run the complete test suite:

```bash
ctest --test-dir build --output-on-failure
```

For Visual Studio builds:

```powershell
ctest --test-dir build -C Release --output-on-failure
```

A successful run means the configured test targets passed. It does not mean that a real V100 was detected or that hardware mutation was tested.

Before submitting changes, also run:

```bash
git diff --check
```

This catches whitespace and patch-format problems.

## 6. Headless Hub / mock usage

After the Hub backend baseline is present, the CLI can be exercised without NVIDIA hardware by selecting the deterministic mock provider.

### Show GPU status

```bash
./build/cli/isb --mock status
```

### Show telemetry

```bash
./build/cli/isb --mock telemetry
```

### Show capabilities

```bash
./build/cli/isb --mock capabilities
```

The synthetic V100 fixture represents the hardware capability model, including Volta/SM70, HBM2, Tensor Cores and the absence of RT Cores, dedicated Optical Flow hardware, MIG and V100 display outputs.

Synthetic output is test data. It is not evidence from a physical GPU.

### List profiles

```bash
./build/cli/isb --mock profile list
```

### Generate a profile plan

```bash
./build/cli/isb --mock profile plan Balanced --dry-run
```

A plan describes what would be requested. A plan is not proof that a setting can be applied to a real V100.

### Generate an optimization plan

```bash
./build/cli/isb --mock optimize --dry-run
```

The safe path is:

```text
SCAN → DETECT → CAPABILITY CHECK → DIAGNOSE → BUILD PLAN → DRY-RUN → APPROVAL → APPLY → VERIFY
```

The mock baseline intentionally does not claim successful hardware mutation or verification.

### Run diagnostics

```bash
./build/cli/isb --mock diagnose
```

### Run verification

```bash
./build/cli/isb --mock verify
```

A non-successful verification result is expected when there is no real hardware mutation to verify.

### Run the benchmark fixture

```bash
./build/cli/isb --mock benchmark
```

The initial benchmark path is a fixture/testing path. Do not use it as a real performance measurement of your V100.

## 7. Generate a report

Create a deterministic report bundle:

```bash
rm -rf isb-report
./build/cli/isb --mock report isb-report
```

Inspect the generated files:

```bash
find isb-report -maxdepth 2 -type f -print | sort
```

The report model is intended to contain structured evidence such as:

```text
manifest.json
gpu.json
driver.json
capabilities.json
games.json
optiscaler.json
performance.json
errors.json
logs/
```

Not every file is populated with real hardware data in the mock build. Empty/unavailable sections must remain distinguishable from measured data.

## 8. Capability states

ISB intentionally distinguishes capability states.

- `AVAILABLE` — the capability was positively detected/exposed.
- `UNAVAILABLE` — the capability was checked and is not available through the current path.
- `UNKNOWN` — it could not be determined reliably.
- `PERMISSION_DENIED` — the operation exists but access is blocked by permissions.
- `ERROR` — probing or the underlying operation failed.

Do not convert `UNKNOWN` into `UNAVAILABLE` or `AVAILABLE`.

Also keep these layers separate:

1. physical V100 hardware capability;
2. base NVIDIA/OS driver capability;
3. ISB-added software capability.

For example, a software graphics technique must not be presented as native V100 RT, Optical Flow or DLSS hardware.

## 9. Development workflow

For a new feature, use this order:

1. Define or reuse the stable Hub contract.
2. Add capability detection/state handling.
3. Add a deterministic mock fixture.
4. Implement the provider-independent orchestration.
5. Add positive, negative and unknown-path tests.
6. Add provenance/evidence fields where applicable.
7. Add dry-run behavior before mutation.
8. Add apply only when a real provider exists.
9. Read back and verify every mutation.
10. Add rollback or explicitly document why rollback is impossible.
11. Add report/log coverage.
12. Only then expose the operation in the GUI.

The GUI and CLI must consume the same Hub contracts. Do not implement business logic twice.

## 10. Testing rules

Every stable feature should cover:

- normal/supported path;
- unsupported path;
- unknown path;
- permission-denied path where relevant;
- provider error path where relevant;
- deterministic serialization/output;
- no accidental mutation during dry-run;
- verification behavior;
- rollback behavior where applicable.

For hardware-dependent features, split tests into:

- unit tests — no GPU required;
- mock/provider tests — deterministic synthetic hardware;
- integration tests — real NVIDIA stack;
- hardware tests — physical V100 with explicitly recorded environment/provenance.

Never make CI depend on a physical V100 for the basic build.

## 11. Real V100 testing later

When a physical Tesla V100 is available, first collect read-only evidence.

Recommended sequence:

```text
GPU detection
→ exact variant detection
→ driver fingerprint
→ capability probing
→ telemetry
→ read-only benchmarks
→ tuning capability detection
→ dry-run plans
→ one controlled mutation
→ read-back
→ verification
→ rollback test
```

Do not begin by applying an aggressive overclock or power-limit profile.

Real hardware results should record at least:

- exact GPU and variant;
- VRAM size;
- driver/package version;
- OS and kernel/Windows version;
- CUDA/runtime version where relevant;
- provider/API used;
- profile/configuration;
- benchmark revision;
- temperature and power conditions;
- verification result.

## 12. GUI roadmap

The Control Center is intended to be a thin frontend over the Hub API.

The final compact interface is planned around four main sections:

1. **GPU** — identity, capabilities, telemetry and current state.
2. **TUNING** — clocks, power and supported controls with review-before-apply.
3. **OPTIMIZATION** — games/applications and safe optimization plans.
4. **GRAPHICS** — scaling, sharpening, compatibility layers and graphics options.

The GUI should remain lightweight and should not directly call NVML, CUDA, Vulkan, DXGI, WDDM or low-level hardware interfaces.

## 13. Troubleshooting

### CMake cannot find a compiler

Install a C++17-capable compiler/toolchain and run the CMake configure command again.

### Tests are missing

Make sure the project was configured with:

```bash
-DBUILD_TESTING=ON
```

Then rebuild before running CTest.

### `build/cli/isb` does not exist

You are probably on a revision that predates the headless Hub backend, or the CLI target was not configured. Check the current branch/commit and configure/build again.

### No NVIDIA GPU is present

That is fine for the mock build. Use `--mock` for deterministic development and testing.

### A mock result says V100

Check the provider/provenance field. Synthetic V100 output is a fixture and must not be treated as physical hardware evidence.

### A feature is `UNKNOWN`

Do not force it to `AVAILABLE` or `UNAVAILABLE`. Add or fix the relevant provider probe if the capability can be determined reliably.

## 14. Useful commands — quick reference

```bash
# Configure
cmake -S . -B build -DBUILD_TESTING=ON

# Build
cmake --build build --parallel

# Test
ctest --test-dir build --output-on-failure

# Patch hygiene
git diff --check

# Mock status
./build/cli/isb --mock status

# Mock telemetry
./build/cli/isb --mock telemetry

# Mock capabilities
./build/cli/isb --mock capabilities

# Mock profile plan
./build/cli/isb --mock profile plan Balanced --dry-run

# Mock optimization plan
./build/cli/isb --mock optimize --dry-run

# Mock report
./build/cli/isb --mock report isb-report
```

## 15. Important safety rule

ISB is a control and compatibility layer, not a replacement for the installed NVIDIA driver.

No feature should claim that it changed the GPU unless:

1. the underlying provider accepted the requested operation;
2. the resulting state was read back;
3. verification passed;
4. the result has appropriate provenance.

The same rule applies to graphics integrations, OptiScaler, game modifications and performance measurements.
