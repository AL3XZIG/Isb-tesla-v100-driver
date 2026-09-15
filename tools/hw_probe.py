#!/usr/bin/env python3
"""NVML-only hardware snapshot probe for the ISB Verification Pack.

Observes the current NVIDIA GPU/driver state through NVML only. It never invokes
CUDA runtime, Vulkan, Windows APIs, PCI sysfs/ioctl, or changes device state.
Unknown is preferred to an inferred value.
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
except ImportError as exc:
    print("hw_probe.py requires NVIDIA NVML Python bindings (pynvml / nvidia-ml-py).", file=sys.stderr)
    raise SystemExit(2) from exc

SCHEMA_VERSION = "1.0"
UNKNOWN, AVAILABLE, UNAVAILABLE = "unknown", "available", "unavailable"

def _obs(value: Any = None, state: str | None = None, **extra: Any) -> dict[str, Any]:
    result = {"state": state or (AVAILABLE if value is not None else UNKNOWN), "value": value}
    result.update(extra)
    return result

def _call(fn: Callable[..., Any] | None, *args: Any) -> tuple[bool, Any]:
    if fn is None:
        return False, None
    try:
        return True, fn(*args)
    except Exception:
        return False, None

def _string(fn: Callable[..., Any] | None, *args: Any) -> dict[str, Any]:
    ok, value = _call(fn, *args)
    if not ok or value is None:
        return _obs(None, UNKNOWN)
    if isinstance(value, bytes):
        value = value.decode("utf-8", errors="replace")
    return _obs(str(value))

def _enum_name(value: Any, prefix: str) -> str | None:
    for name in dir(pynvml):
        if name.startswith(prefix) and getattr(pynvml, name) == value:
            return name[len(prefix):].lower()
    return None

def _pci(device: Any) -> dict[str, Any]:
    ok, info = _call(getattr(pynvml, "nvmlDeviceGetPciInfo", None), device)
    keys = ("bus_id", "domain", "bus", "device", "function", "device_id", "subsystem_id")
    if not ok or info is None:
        return {key: _obs(None, UNKNOWN) for key in keys}
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
    return {
        "bus_id": _obs(str(bus_id)) if bus_id else _obs(None, UNKNOWN),
        "domain": field("domain"), "bus": field("bus"), "device": field("device"),
        "function": field("function"),
        # Report NVML's raw IDs. Do not map IDs to a hardcoded GPU model table.
        "device_id": field("pciDeviceId", "deviceId"),
        "subsystem_id": field("pciSubSystemId", "subsystemId"),
    }

def _memory(device: Any) -> dict[str, Any]:
    ok, info = _call(getattr(pynvml, "nvmlDeviceGetMemoryInfo", None), device)
    if not ok or info is None:
        return {key: _obs(None, UNKNOWN, unit="bytes") for key in ("total", "used", "free")}
    return {key: _obs(int(getattr(info, key)), unit="bytes") for key in ("total", "used", "free") if hasattr(info, key)} | {
        key: _obs(None, UNKNOWN, unit="bytes") for key in ("total", "used", "free") if not hasattr(info, key)
    }

def _clock(device: Any, constant: str) -> dict[str, Any]:
    clock_type = getattr(pynvml, constant, None)
    ok, value = _call(getattr(pynvml, "nvmlDeviceGetClockInfo", None), device, clock_type) if clock_type is not None else (False, None)
    return _obs(int(value), unit="MHz") if ok and value is not None else _obs(None, UNKNOWN, unit="MHz")

def _temperature(device: Any) -> dict[str, Any]:
    sensor = getattr(pynvml, "NVML_TEMPERATURE_GPU", None)
    ok, value = _call(getattr(pynvml, "nvmlDeviceGetTemperature", None), device, sensor) if sensor is not None else (False, None)
    return _obs(int(value), unit="C") if ok and value is not None else _obs(None, UNKNOWN, unit="C")

def _power(device: Any) -> dict[str, Any]:
    result = {}
    for key, fn_name in (("usage", "nvmlDeviceGetPowerUsage"), ("limit", "nvmlDeviceGetPowerManagementLimit")):
        ok, value = _call(getattr(pynvml, fn_name, None), device)
        result[key] = _obs(int(value), unit="mW") if ok and value is not None else _obs(None, UNKNOWN, unit="mW")
    return result

def _utilization(device: Any) -> dict[str, Any]:
    ok, value = _call(getattr(pynvml, "nvmlDeviceGetUtilizationRates", None), device)
    if not ok or value is None:
        return {key: _obs(None, UNKNOWN, unit="percent") for key in ("gpu", "memory")}
    return {"gpu": _obs(int(value.gpu), unit="percent"), "memory": _obs(int(value.memory), unit="percent")}

def _mode(device: Any, fn_name: str, prefix: str) -> dict[str, Any]:
    ok, value = _call(getattr(pynvml, fn_name, None), device)
    if not ok:
        return _obs(None, UNKNOWN)
    name = _enum_name(value, prefix)
    return _obs(name if name is not None else str(value))

def _ecc(device: Any) -> dict[str, Any]:
    result = {"mode": _obs(None, UNKNOWN), "errors": {}}
    ok, value = _call(getattr(pynvml, "nvmlDeviceGetEccMode", None), device)
    if ok and isinstance(value, tuple) and value:
        current = value[0]
        name = _enum_name(current, "NVML_FEATURE_") or ("enabled" if current else "disabled")
        result["mode"] = _obs(name)
    total = getattr(pynvml, "nvmlDeviceGetTotalEccErrors", None)
    if total is None:
        return result
    for error_name in ("NVML_ECC_ERROR_TYPE_CORRECTED", "NVML_ECC_ERROR_TYPE_UNCORRECTED"):
        error_type = getattr(pynvml, error_name, None)
        if error_type is None:
            continue
        for counter_name in ("NVML_VOLATILE_ECC", "NVML_AGGREGATE_ECC"):
            counter_type = getattr(pynvml, counter_name, None)
            if counter_type is None:
                continue
            ok, value = _call(total, device, error_type, counter_type)
            suffix = error_name.replace("NVML_ECC_ERROR_TYPE_", "").lower()
            suffix += "_" + counter_name.replace("NVML_", "").lower()
            result["errors"][suffix] = _obs(int(value), unit="errors") if ok and value is not None else _obs(None, UNKNOWN, unit="errors")
    return result

def _nvlink(device: Any) -> dict[str, Any]:
    fn = getattr(pynvml, "nvmlDeviceGetNvLinkState", None)
    max_links = getattr(pynvml, "NVML_NVLINK_MAX_LINKS", None)
    if fn is None or max_links is None:
        return {"state": UNKNOWN, "links": []}
    links, observed, enabled = [], True, 0
    for index in range(int(max_links)):
        ok, value = _call(fn, device, index)
        if not ok:
            observed = False
            links.append({"index": index, "state": UNKNOWN})
        else:
            state = AVAILABLE if bool(value) else UNAVAILABLE
            enabled += int(bool(value))
            links.append({"index": index, "state": state})
    overall = AVAILABLE if enabled else (UNAVAILABLE if observed else UNKNOWN)
    return {"state": overall, "links": links}

def _device_snapshot(device: Any, index: int) -> dict[str, Any]:
    identity = {
        "index": index,
        "name": _string(getattr(pynvml, "nvmlDeviceGetName", None), device),
        "uuid": _string(getattr(pynvml, "nvmlDeviceGetUUID", None), device),
        "serial": _string(getattr(pynvml, "nvmlDeviceGetSerial", None), device),
        "brand": _obs(None, UNKNOWN),
        "architecture": _obs(None, UNKNOWN),
    }
    ok, value = _call(getattr(pynvml, "nvmlDeviceGetBrand", None), device)
    if ok:
        identity["brand"] = _obs(_enum_name(value, "NVML_BRAND_") or str(value))
    return {
        "identity": identity,
        "pci": _pci(device), "memory": _memory(device), "ecc": _ecc(device),
        "clocks": {"graphics": _clock(device, "NVML_CLOCK_GRAPHICS"), "sm": _clock(device, "NVML_CLOCK_SM"), "memory": _clock(device, "NVML_CLOCK_MEM")},
        "power": _power(device), "temperature": _temperature(device), "utilization": _utilization(device),
        "persistence_mode": _mode(device, "nvmlDeviceGetPersistenceMode", "NVML_FEATURE_"),
        "compute_mode": _mode(device, "nvmlDeviceGetComputeMode", "NVML_COMPUTEMODE_"),
        "nvlink": _nvlink(device),
    }

def build_snapshot() -> dict[str, Any]:
    snapshot = {
        "schema_version": SCHEMA_VERSION,
        "timestamp": datetime.now(timezone.utc).isoformat().replace("+00:00", "Z"),
        "probe_status": {"state": AVAILABLE, "source": "NVML-only"},
        "host": {"hostname": _obs(socket.gethostname()), "platform": _obs(platform.system()), "kernel": _obs(platform.release()), "machine": _obs(platform.machine())},
        "os": {"name": _obs(platform.system()), "release": _obs(platform.release())},
        "driver": {"version": _obs(None, UNKNOWN)},
        "cuda": {"driver_version": _obs(None, UNKNOWN), "runtime_version": _obs(None, UNKNOWN)},
        "gpus": [],
    }
    try:
        pynvml.nvmlInit()
    except Exception as exc:
        snapshot["probe_status"] = {"state": UNKNOWN, "source": "NVML-only", "error": type(exc).__name__}
        return snapshot
    try:
        snapshot["driver"]["version"] = _string(getattr(pynvml, "nvmlSystemGetDriverVersion", None))
        fn = getattr(pynvml, "nvmlSystemGetCudaDriverVersion_v2", None) or getattr(pynvml, "nvmlSystemGetCudaDriverVersion", None)
        ok, value = _call(fn)
        if ok and value is not None:
            number = int(value)
            snapshot["cuda"]["driver_version"] = _obs(f"{number // 1000}.{(number % 1000) // 10}")
        try:
            count = int(pynvml.nvmlDeviceGetCount())
        except Exception:
            count = 0
        for index in range(count):
            try:
                device = pynvml.nvmlDeviceGetHandleByIndex(index)
                snapshot["gpus"].append(_device_snapshot(device, index))
            except Exception as exc:
                snapshot["gpus"].append({"identity": {"index": index}, "probe_error": type(exc).__name__})
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
