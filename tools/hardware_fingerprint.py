#!/usr/bin/env python3
"""Create a deterministic hardware fingerprint from an IDR snapshot.

The fingerprint is an identifier of observed fields, not a GPU model classifier.
No V100-specific lookup table or hardware inference is performed.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import sys
from pathlib import Path
from typing import Any


def _load(path: str) -> dict[str, Any]:
    if path == "-":
        value = json.load(sys.stdin)
    else:
        with open(path, "r", encoding="utf-8") as handle:
            value = json.load(handle)
    if not isinstance(value, dict):
        raise ValueError("IDR root must be a JSON object")
    return value


def _available(observation: Any) -> Any:
    if not isinstance(observation, dict) or observation.get("state") != "available":
        return None
    return observation.get("value")


def _fingerprint_material(snapshot: dict[str, Any]) -> dict[str, Any]:
    gpus = snapshot.get("gpus", [])
    material: list[dict[str, Any]] = []
    for gpu in gpus if isinstance(gpus, list) else []:
        if not isinstance(gpu, dict):
            continue
        identity = gpu.get("identity", {})
        pci = gpu.get("pci", {})
        memory = gpu.get("memory", {})
        material.append({
            "uuid": _available(identity.get("uuid")),
            "serial": _available(identity.get("serial")),
            "name": _available(identity.get("name")),
            "brand": _available(identity.get("brand")),
            "pci": {
                "domain": _available(pci.get("domain")),
                "bus": _available(pci.get("bus")),
                "device": _available(pci.get("device")),
                "function": _available(pci.get("function")),
                "device_id": _available(pci.get("device_id")),
                "subsystem_id": _available(pci.get("subsystem_id")),
            },
            "memory_total": _available(memory.get("total")),
        })
    return {"gpu_count": len(material), "gpus": material}


def fingerprint(snapshot: dict[str, Any]) -> tuple[str, dict[str, Any]]:
    material = _fingerprint_material(snapshot)
    canonical = json.dumps(material, sort_keys=True, separators=(",", ":"), ensure_ascii=True).encode("utf-8")
    digest = hashlib.sha256(canonical).hexdigest()
    return digest, material


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate a deterministic fingerprint from an ISB IDR snapshot.")
    parser.add_argument("input", help="IDR JSON file, or '-' for stdin")
    parser.add_argument("--json", action="store_true", help="Emit JSON instead of the default fingerprint line")
    args = parser.parse_args()

    try:
        snapshot = _load(args.input)
        digest, material = fingerprint(snapshot)
    except (OSError, json.JSONDecodeError, ValueError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    if args.json:
        json.dump({"algorithm": "SHA-256", "fingerprint": digest, "material": material}, sys.stdout, indent=2, sort_keys=True)
        sys.stdout.write("\n")
    else:
        print(digest)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
