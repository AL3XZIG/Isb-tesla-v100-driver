#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${ROOT}/build"
PREFIX="${HOME}/.local"
if [[ "${EUID}" -eq 0 ]]; then PREFIX="/usr/local"; fi
echo "[ISB] Checking build dependencies..."
if command -v apt-get >/dev/null 2>&1; then
  if [[ "${EUID}" -eq 0 ]]; then apt-get update; apt-get install -y build-essential cmake pkg-config
  else sudo apt-get update; sudo apt-get install -y build-essential cmake pkg-config; fi
elif ! command -v cmake >/dev/null 2>&1 || ! command -v c++ >/dev/null 2>&1; then
  echo "[ISB] Unsupported package manager. Install CMake and a C++17 compiler, then rerun."; exit 2
fi
cmake -S "${ROOT}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build "${BUILD_DIR}" --parallel
ctest --test-dir "${BUILD_DIR}" --output-on-failure
mkdir -p "${PREFIX}/bin"
install -m 0755 "${BUILD_DIR}/cli/isb" "${PREFIX}/bin/isb"
echo "[ISB] Installed: ${PREFIX}/bin/isb"
