$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent (Split-Path -Parent $MyInvocation.MyCommand.Path)
$Build = Join-Path $Root "build"
Write-Host "[ISB] Windows bootstrap"
Write-Host "[ISB] CMake and a C++17 toolchain must already be installed."
cmake -S $Root -B $Build -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build $Build --config Release --parallel
ctest --test-dir $Build -C Release --output-on-failure
Write-Host "[ISB] Build complete: $Build"
