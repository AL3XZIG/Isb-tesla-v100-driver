#!/usr/bin/env python3
"""Static dependency checks for the user-space architecture layers.

The checker is intentionally conservative: it rejects direct platform/API
includes in layers that are required to remain provider- and OS-neutral.
"""

from __future__ import annotations

import pathlib
import sys

ROOT = pathlib.Path(__file__).resolve().parents[1]

RULES = {
    "cal": {
        "forbidden": (
            "nvml.h",
            "cuda.h",
            "cuda_runtime",
            "vulkan/",
            "windows.h",
            "d3d",
            "wddm",
            "drm/",
            "linux/",
        )
    },
    "core": {
        "forbidden": (
            "nvml.h",
            "cuda.h",
            "cuda_runtime",
            "vulkan/",
            "windows.h",
            "d3d",
            "wddm",
            "drm/",
            "linux/",
        )
    },
    "hal/include": {
        "forbidden": (
            "nvml.h",
            "cuda.h",
            "cuda_runtime",
            "vulkan/",
            "windows.h",
            "d3d",
            "wddm",
            "drm/",
            "linux/",
        )
    },
    "os/include": {
        "forbidden": (
            "nvml.h",
            "cuda.h",
            "cuda_runtime",
            "vulkan/",
            "windows.h",
            "d3d",
            "wddm",
            "drm/",
        )
    },
}

SOURCE_SUFFIXES = {".h", ".hpp", ".c", ".cc", ".cpp", ".cxx"}


def main() -> int:
    failures: list[str] = []

    for relative_root, rule in RULES.items():
        root = ROOT / relative_root
        if not root.exists():
            continue

        for path in root.rglob("*"):
            if not path.is_file() or path.suffix not in SOURCE_SUFFIXES:
                continue

            try:
                text = path.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                failures.append(f"{path}: not valid UTF-8 source text")
                continue

            for line_number, line in enumerate(text.splitlines(), start=1):
                if not line.lstrip().startswith("#include"):
                    continue
                lowered = line.lower()
                for forbidden in rule["forbidden"]:
                    if forbidden.lower() in lowered:
                        failures.append(
                            f"{path}:{line_number}: forbidden include pattern {forbidden!r}"
                        )

    if failures:
        print("Architecture dependency check FAILED:")
        for failure in failures:
            print(f"  - {failure}")
        return 1

    print("Architecture dependency check PASSED")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
