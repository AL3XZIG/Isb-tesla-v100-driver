# ISB multi-agent work plan

The project is split into small, independently reviewable tasks. Agents must not rewrite unrelated layers or modify another agent's branch. This follows the project's focused-PR policy.

## Rules for every agent

1. Inspect `main` and the current target branch before changing code.
2. Do not assume that an architecture document means an implementation exists.
3. Keep the change focused on the assigned row.
4. Add tests for every new contract or behavior.
5. Do not add NVIDIA proprietary binaries or proprietary headers.
6. Do not cross HAL/OSAL/kernel ABI boundaries with C++ STL, virtual interfaces, pointers, `bool`, or `size_t`.
7. Do not silently invent V100 PCI IDs, BAR layouts, VBIOS values, firmware behavior, or performance numbers.
8. Mark hardware-dependent claims as blocked until verified on physical hardware.
9. Produce a short PR summary containing: changed files, tests run, known limitations, and follow-up work.
10. If a task exposes a dependency on another task, stop and document the dependency instead of expanding scope.

## Work allocation

| ID | Agent | Task | Main files / area | Depends on | Deliverable | Priority |
|---|---|---|---|---|---|---|
| A1 | Codex | CAL validation + semantic cleanup | `cal/` | quality-foundation-v1 | Validation API, negative tests, no CUDA/platform headers | P0 |
| A2 | Claude Code | Architecture/CI enforcement | `.github/`, `tools/` | A1 | Required build/test jobs, include-boundary checks, reproducible checks | P0 |
| A3 | DeepSeek | OSAL v0.1 implementation | `os/` | HAL v0.1 | C++ OSAL contract, fake backend, tests, CMake | P0 |
| A4 | Claude Code | OSAL ABI contract | `os/include/isb/os/abi/`, `docs/` | A3 | C-compatible wire structs with version/size/reserved fields | P0 |
| A5 | DeepSeek | Linux KMD architecture | `drivers/linux/`, `docs/` | A3, A4 | PCI probe, device lifecycle and ioctl design; no unsafe MMIO shortcuts | P0 |
| A6 | Codex | Linux HAL→OSAL adapter | `hal/linux/` | A3, A5 | Real HAL backend boundary without direct userspace BAR access | P1 |
| A7 | Claude Code | GV100 hardware identity research | `docs/hardware/`, `diagnostics/` | none | Sourced PCI/device/BAR/firmware matrix, provenance only | P1 |
| A8 | DeepSeek | GPUVM contract | `drivers/common/`, `os/` | A5 | GPU virtual address and mapping lifecycle specification/tests | P1 |
| A9 | Codex | Command submission + sync contract | `drivers/common/`, `os/` | A8 | Queue/channel/fence abstractions; no scheduler implementation yet | P1 |
| A10 | Claude Code | Windows WDDM/MCDM architecture | `drivers/windows/`, `docs/` | A3, A4 | Separate WDDM KMD/UMD and MCDM boundaries; feasibility matrix | P1 |
| A11 | DeepSeek | Windows implementation prototype | `drivers/windows/` | A10 | Minimal compile-only backend skeleton, no unsupported V100 claims | P2 |
| A12 | Codex | CAL serialization extraction | `cal/`, `serialization/` | A1 | Move JSON formatting behind adapter without breaking v1 | P2 |
| A13 | Claude Code | Provider-neutral runtime | `providers/`, `core/` | A1, A3 | Provider interface, explicit errors, no NVML headers in core | P1 |
| A14 | DeepSeek | NVML provider | `providers/nvml/` | A13 | Optional reference provider, graceful no-NVML build | P1 |
| A15 | Codex | Real V100 validation harness | `diagnostics/`, `tests/hardware/` | A5, A9 | Hardware test protocol and machine-readable evidence capture | P1 |
| A16 | Claude Code | External code review | whole repo | after each P0/P1 PR | Independent review; no implementation changes unless assigned | P0 gate |

## Recommended execution order

### Phase 1 — quality foundation

`A1 → A2 → A16`

This closes the gap identified by the code review without pretending that a kernel driver already exists.

### Phase 2 — OS boundary

`A3 → A4 → A5 → A6 → A16`

This turns the approved OSAL design into a real path toward hardware access.

### Phase 3 — GPU execution foundation

`A8 → A9 → A13 → A14 → A16`

GPUVM and submission/synchronization must exist before higher-level runtime claims are made.

### Phase 4 — hardware validation

`A7 → A15 → A16`

Hardware-specific facts are collected separately from architectural assumptions.

### Phase 5 — Windows and graphics

`A10 → A11 → A16`

Windows work starts only after the OSAL ABI and Linux boundary are stable enough to avoid duplicating mistakes.

### Phase 6 — cleanup and compatibility

`A12 → graphics/compatibility agents → A16`

Only after the lower layers are real should the project spend significant effort on Vulkan, D3D, upscaling, frame generation, or game compatibility.

## Branch / PR policy

Prefer one branch and one PR per row. Dependent tasks may use stacked branches, but each PR must contain one coherent layer. GitHub explicitly recommends small, focused PRs and supports stacked PRs for dependent layers. citeturn0search0turn0search3

Suggested branch names:

- `cal-validation-v1`
- `ci-boundaries-v1`
- `osal-v0.1`
- `osal-abi-v0.1`
- `linux-kmd-foundation-v0.1`
- `linux-hal-backend-v0.1`
- `gv100-hardware-matrix-v0.1`
- `gpuvm-contract-v0.1`
- `command-sync-contract-v0.1`
- `windows-boundaries-v0.1`
- `provider-core-v0.1`
- `nvml-provider-v0.1`
- `hardware-validation-v0.1`

## Merge gate

A PR is not ready merely because it compiles. Before merge, require:

- focused diff;
- tests for changed behavior;
- architecture/dependency checks;
- explicit hardware-validation status;
- no unverified hardware claims;
- no proprietary NVIDIA source/binaries;
- documentation updated when an interface or maturity state changes.
