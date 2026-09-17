# Build and test

The repository currently exposes a small root CMake build that composes the
provider-neutral CAL target and its tests. GPU hardware is not required.

## Requirements

- CMake 3.16 or newer
- C++17 compiler
- CTest (provided by CMake)

## Configure and build

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON -DISB_CAL_ENABLE_WARNINGS=ON
cmake --build build --parallel
```

## Run the full CTest suite

```sh
ctest --test-dir build --output-on-failure
```

A successful test run exits with status `0`. Configure, build, or test
failures return a non-zero status.

## Debug build

```sh
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build-debug --parallel
ctest --test-dir build-debug --output-on-failure
```

## Dependency boundary

The root build currently depends only on the C++17 standard library and the
CAL source tree. It does not require CUDA, NVML, Vulkan, Linux kernel headers,
or a physical NVIDIA GPU.

The CI workflow intentionally runs the same configure/build/test sequence on
an ordinary Ubuntu runner. Hardware-dependent tests must be added as separate
integration/hardware jobs rather than making the baseline CI GPU-dependent.

## Optional Control Center

The native **ISB V100 Control Center** is a Qt 6 Widgets target. It is optional: a
normal C++17 build still configures and tests the hardware-independent control
plane when Qt is absent. Install Qt 6 Widgets and configure with
`-DISB_BUILD_CONTROL_CENTER=ON` to create the `isb-v100-control-center` target.

```sh
cmake -S . -B build -DBUILD_TESTING=ON -DISB_BUILD_CONTROL_CENTER=ON
cmake --build build --target isb-v100-control-center
./build/control-center/isb-v100-control-center
```

Without a provider-backed Hub implementation the application deliberately runs
in **MOCK / SIMULATION** mode. Its unavailable telemetry uses `—`, and controls
with unknown capability remain disabled; it does not issue NVIDIA API calls.
