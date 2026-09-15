#!/usr/bin/env python3
"""NVML-only hardware snapshot probe for the ISB Verification Pack.

This tool observes the current NVIDIA GPU/driver state through NVML only. It does
not invoke CUDA runtime, Vulkan, Windows APIs, PCI sysfs/ioctl, or change device
state. Unknown is preferred to an inferred value.

Dependency: NVIDIA NVML Python bindings (``pynvml`` / ``nvidia-ml-py``).
"""

from __future__ import annotations

import json
import platform
import socket
import sys
from datetime import datetime, timezone
from typing import Any, Callable

try:
    import pynvml
except ImportError as exc:  # pragma: no cover - environment dependent
    print(
        "hw_probe.py requires the NVIDIA NVML Python bindings (pynvml / nvidia-ml-py).",
        file=sys.stderr,
    )
    raise SystemExit(2) from exc

SCHEMA_VERSION = "1.0"
UNKNOWN = "unknown"
AVAILABLE = "available"
UNAVAILABLE = "unavailable"


def _iso_now() -> str:
    return datetime.now(timezone.utc).isoformat().replace("+00:00", "Z")


def _obs(value: Any = None, state: str | None = None, **extra: Any) -> dict[str, Any]:
    if state is None:
        state = AVAILABLE if value is not None else UNKNOWN
    result: dict[str, Any] = {"state": state, "value": value}
    result.update(extra)
    return result


def _call(fn: Callable[..., Any], *args: Any) -> tuple[str, Any]:
    """Call an NVML function and convert absent/unsupported values to UNKNOWN."""
    try:
        return AVAILABLE, fn(*args)
    except Exception:
        return UNKNOWN, None


def _string(fn: Callable[..., Any], *args: Any) -> dict[str, Any]:
    state, value = _call(fn, *args)
    if state != AVAILABLE or value is None:
        return _obs(None, UNKNOWN)
    if isinstance(value, bytes):
        value = value.decode("utf-8", errors="replace")
    return _obs(str(value))


def _enum_name(value: Any, prefix: str) -> str | None:
    for name in dir(pynvml):
        if name.startswith(prefix) and getattr(pynvml, name) == value:
            return name[len(prefix) :].lower()
    return None


def _version_tuple(value: Any) -> dict[str, Any]:
    """Represent an NVML version string without pretending it is a CUDA runtime version."""
    if value is None:
        return _obs(None, UNKNOWN)
    if isinstance(value, bytes):
        value = value.decode("utf-8", errors="replace")
    return _obs(str(value))


def _pci(device: Any) -> dict[str, Any]:
    state, info = _call(pynvml.nvmlDeviceGetPciInfo, device)
    if state != AVAILABLE or info is None:
        return {
            "bus_id": _obs(None, UNKNOWN),
            "domain": _obs(None, UNKNOWN),
            "bus": _obs(None, UNKNOWN),
            "device": _obs(None, UNKNOWN),
            "function": _obs(None, UNKNOWN),
            "device_id": _obs(None, UNKNOWN),
            "subsystem_id": _obs(None, UNKNOWN),
        }

    def field(*names: str) -> dict[str, Any]:
        for name in names:
            if hasattr(info, name):
                value = getattr(info, name)
                if value is not None:
                    return _obs(int(value) if isinstance(value, int) else str(value))
        return _obs(None, UNKNOWN)

    bus_id = getattr(info, "busId", None)
    if isinstance(bus_id, bytes):
        bus_id = bus_id.decode("ascii", errors="replace")
    elif bus_id is not None:
        bus_id = str(bus_id)

    # NVML's pciDeviceId is reported as-is. We deliberately do not decode it into
    # a GPU model or hardcode a V100 device-ID table.
    return {
        "bus_id": _obs(bus_id) if bus_id else _obs(None, UNKNOWN),
        "domain": field("domain"),
        "bus": field("bus"),
        "device": field("device"),
        "function": field("function"),
        "device_id": field("pciDeviceId", "deviceId"),
        "subsystem_id": field("pciSubSystemId", "subsystemId"),
    }


def _memory(device: Any) -> dict[str, Any]:
    state, info = _call(pynvml.nvmlDeviceGetMemoryInfo, device)
    if state != AVAILABLE or info is None:
        return {key: _obs(None, UNKNOWN, unit="bytes") for key in ("total", "used", "free")}
    return {
        "total": _obs(int(info.total), unit="bytes"),
        "used": _obs(int(info.used), unit="bytes"),
        "free": _obs(int(info.free), unit="bytes"),
    }


def _clock(device: Any, clock_type_name: str) -> dict[str, Any]:
    clock_type = getattr(pynvml, clock_type_name, None)
    if clock_type is None:
        return _obs(None, UNKNOWN, unit="MHz")
    state, value = _call(pynvml.nvmlDeviceGetClockInfo, device, clock_type)
    return _obs(int(value), unit="MHz") if state == AVAILABLE and value is not None else _obs(None, UNKNOWN, unit="MHz")


def _temperature(device: Any) -> dict[str, Any]:
    sensor = getattr(pynvml, "NVML_TEMPERATURE_GPU", None)
    if sensor is None:
        return _obs(None, UNKNOWN, unit="C")
    state, value = _call(pynvml.nvmlDeviceGetTemperature, device, sensor)
    return _obs(int(value), unit="C") if state == AVAILABLE and value is not None else _obs(None, UNKNOWN, unit="C")


def _power(device: Any) -> dict[str, Any]:
    result: dict[str, Any] = {}
    for key, fn_name in (("usage", "nvmlDeviceGetPowerUsage"), ("limit", "nvmlDeviceGetPowerManagementLimit")):
        fn = getattr(pynvml, fn_name, None)
        if fn is None:
            result[key] = _obs(None, UNKNOWN, unit="mW")
            continue
        state, value = _call(fn, device)
        result[key] = _obs(int(value), unit="mW") if state == AVAILABLE and value is not None else _obs(None, UNKNOWN, unit="mW")
    return result


def _utilization(device: Any) -> dict[str, Any]:
    fn = getattr(pynvml, "nvmlDeviceGetUtilizationRates", None)
    if fn is None:
        return {"gpu": _obs(None, UNKNOWN, unit="percent"), "memory": _obs(None, UNKNOWN, unit="percent")}
    state, value = _call(fn, device)
    if state != AVAILABLE or value is None:
        return {"gpu": _obs(None, UNKNOWN, unit="percent"), "memory": _obs(None, UNKNOWN, unit="percent")}
    return {
        "gpu": _obs(int(value.gpu), unit="percent"),
        "memory": _obs(int(value.memory), unit="percent"),
    }


def _mode(device: Any, fn_name: str, enum_prefix: str) -> dict[str, Any]:
    fn = getattr(pynvml, fn_name, None)
    if fn is None:
        return _obs(None, UNKNOWN)
    state, value = _call(fn, device)
    if state != AVAILABLE:
        return _obs(None, UNKNOWN)
    name = _enum_name(value, enum_prefix)
    return _obs(name if name is not None else str(value))


def _ecc(device: Any) -> dict[str, Any]:
    result: dict[str, Any] = {"mode": _obs(None, UNKNOWN), "errors": {}}
    fn = getattr(pynvml, "nvmlDeviceGetEccMode", None)
    if fn is not None:
        state, value = _call(fn, device)
        if state == AVAILABLE and isinstance(value, tuple) and value:
            current = value[0]
            name = _enum_name(current, "NVML_FEATURE_")
            if name is None:
                name = "enabled" if current else "disabled"
            result["mode"] = _obs(name)

    # Total ECC counters are optional across NVML generations/bindings. Probe only
    # enums that are actually exposed by the installed binding.
    total_fn = getattr(pynvml, "nvmlDeviceGetTotalEccErrors", None)
    if total_fn is None:
        return result
    for error_name in ("NVML_ECC_ERROR_TYPE_CORRECTED", "NVML_ECC_ERROR_TYPE_UNCORRECTED"):
        error_type = getattr(pynvml, error_name, None)
        if error_type is None:
            continue
        for counter_name in ("NVML_VOLATILE_ECC", "NVML_AGGREGATE_ECC"):
            counter_type = getattr(pynvml, counter_name, None)
            if counter_type is None:
                continue
            state, value = _call(total_fn, device, error_type, counter_type)
            key = f"{error_name.removeprefix('NVML_ECC_ERROR_TYPE_').lower()}_{counter_name.removeprefix('NVML_').lower()}"
            result["errors"][key] = _obs(int(value), unit="errors") if state == AVAILABLE and value is not None else _obs(None, UNKNOWN, unit="errors")
    return result


def _nvlink(device: Any) -> dict[str, Any]:
    state_fn = getattr(pynvml, "nvmlDeviceGetNvLinkState", None)
    max_links = getattr(pynvml, "NVML_NVLINK_MAX_LINKS", None)
    if state_fn is None or max_links is None:
        return {"state": UNKNOWN, "links": []}

    links = []
    observed = False
    enabled = 0
    for index in range(int(max_links)):
        state, value = _call(state_fn, device, index)
        if state == AVAILABLE:
            observed = True
            link_state = bool(value)
            if link_state:
                enabled += 1
            links.append({"index": index, "state": AVAILABLE if link_state else UNAVAILABLE})
        else:
            links.append({"index": index, "state": UNKNOWN})

    overall = AVAILABLE if enabled else (UNAVAILABLE if observed and all(link["state"] != UNKNOWN for link in links) else UNKNOWN)
    return {"state": overall, "links": links}


def _device_snapshot(device: Any, index: int) -> dict[str, Any]:
    identity = {
        "index": index,
        "name": _string(pynvml.nvmlDeviceGetName, device),
        "uuid": _string(pynvml.nvmlDeviceGetUUID, device),
        "serial": _string(pynvml.nvmlDeviceGetSerial, device),
        "brand": _obs(None, UNKNOWN),
        "architecture": _obs(None, UNKNOWN),
    }

    brand_fn = getattr(pynvml, "nvmlDeviceGetBrand", None)
    if brand_fn is not None:
        state, value = _call(brand_fn, device)
        if state == AVAILABLE:
            name = _enum_name(value, "NVML_BRAND_")
            identity["brand"] = _obs(name if name is not None else str(value))

    return {
        "identity": identity,
        "pci": _pci(device),
        "memory": _memory(device),
        "ecc": _ecc(device),
        "clocks": {
            "graphics": _clock(device, "NVML_CLOCK_GRAPHICS"),
            "sm": _clock(device, "NVML_CLOCK_SM"),
            "memory": _clock(device, "NVML_CLOCK_MEM"),
        },
        "power": _power(device),
        "temperature": _temperature(device),
        "utilization": _utilization(device),
        "persistence_mode": _mode(device, "nvmlDeviceGetPersistenceMode", "NVML_FEATURE_"),
        "compute_mode": _mode(device, "nvmlDeviceGetComputeMode", "NVML_COMPUTEMODE_"),
        "nvlink": _nvlink(device),
    }


def build_snapshot() -> dict[str, Any]:
    snapshot: dict[str, Any] = {
        "schema_version": SCHEMA_VERSION,
        "timestamp": _iso_now(),
        "probe_status": {"state": AVAILABLE, "source": "NVML-only"},
        "host": {
            "hostname": _obs(socket.gethostname()),
            "platform": _obs(platform.system()),
            "kernel": _obs(platform.release()),
            "machine": _obs(platform.machine()),
        },
        "os": {
            "name": _obs(platform.system()),
            "release": _obs(platform.release()),
        },
        "driver": {
            "version": _obs(None, UNKNOWN),
        },
        "cuda": {
            "driver_version": _obs(None, UNKNOWN),
            # NVML exposes the CUDA driver API version, not the installed CUDA
            # runtime/toolkit version used by an arbitrary workload.
            "runtime_version": _obs(None, UNKNOWN),
        },
        "gpus": [],
    }

    driver_fn = getattr(pynvml, "nvmlSystemGetDriverVersion", None)
    if driver_fn is not None:
        snapshot["driver"]["version"] = _string(driver_fn)

    cuda_driver_fn = getattr(pynvml, "nvmlSystemGetCudaDriverVersion_v2", None)
    if cuda_driver_fn is None:
        cuda_driver_fn = getattr(pynvml, "nvmlSystemGetCudaDriverVersion", None)
    if cuda_driver_fn is not None:
        state, value = _call(cuda_driver_fn)
        if state == AVAILABLE and value is not None:
            # NVML returns an integer CUDA driver API version (e.g. 12040).
            major = int(value) // 1000
            minor = (int(value) % 1000) // 10
            snapshot["cuda"]["driver_version"] = _obs(f"{major}.{minor}")

    try:
        pynvml.nvmlInit()
    except Exception as exc:
        snapshot["probe_status"] = {
            "state": UNKNOWN,
            "source": "NVML-only",
            "error": type(exc).__name__,
        }
        return snapshot

    try:
        try:
            count = int(pynvml.nvmlDeviceGetCount())
        except Exception:
            count = 0
        for index in range(count):
            try:
                device = pynvml.nvmlDeviceGetHandleByIndex(index)
                snapshot["gpus"].append(_device_snapshot(device, index))
            except Exception as exc:
                snapshot["gpus"].append({
                    "identity": {"index": index},
                    "probe_error": type(exc).__name__,
                })
    finally:
        try:
            pynvml.nvmlShutdown()
        except Exception:
            pass

    return snapshot


def main() -> int:
    json.dump(build_snapshot(), sys.stdout, indent=2, sort_keys=True)
    sys.stdout.write("\n")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
