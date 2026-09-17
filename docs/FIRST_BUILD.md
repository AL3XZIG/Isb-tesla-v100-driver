# First backend build (no V100 required)

ISB's first build runs entirely without a GPU, CUDA Toolkit, NVML, Qt, or Windows SDK. The deterministic mock is evidence labelled `"synthetic":true`; it is a demonstration fixture, **not** proof of real V100 behavior.

## Prerequisites

Install CMake 3.16+, a C++17 compiler, and a build tool (Ninja or Make). On Linux: `cmake -S . -B build -DBUILD_TESTING=ON`, `cmake --build build --parallel`, then `ctest --test-dir build --output-on-failure`. On Windows, run the same commands from a Developer PowerShell, adding `-G Ninja` if Ninja is installed.

## Mock walkthrough

Launch the same headless CLI the future GUI will call:

```sh
./build/cli/isb --mock status
./build/cli/isb --mock telemetry
./build/cli/isb --mock capabilities
./build/cli/isb --mock profile list
./build/cli/isb --mock profile plan Balanced --dry-run
./build/cli/isb --mock optimize --dry-run
./build/cli/isb --mock diagnose
./build/cli/isb --mock benchmark
./build/cli/isb --mock report isb-report
```

The report is written to `isb-report/` with `manifest.json`, `gpu.json`, `driver.json`, `capabilities.json`, `games.json`, `optiscaler.json`, `performance.json`, `errors.json`, and `logs/`. JSON-producing commands accept `--json` (JSON is currently the canonical output format).

## Limits without hardware

Mock identity and telemetry are deterministic fixtures only. Controls intentionally remain `unknown`, plans have no executable mutations, and verification never claims an apply occurred. Real telemetry, NVML controls, CUDA health/benchmarks, Vulkan enumeration, PCIe/NVLink state, running processes, and actual driver detection require future optional provider implementations and applicable installed runtimes; the core safely reports `unknown` instead of inventing evidence.
