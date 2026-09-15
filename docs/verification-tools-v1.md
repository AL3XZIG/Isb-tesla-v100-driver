# Verification Tools v1

This document describes the small, read-only tools added around the ISB Hardware Snapshot / IDR format.

## Components

### IDR validator

`tools/validate_idr.py` validates an IDR JSON document against the checked-in `diagnostics/idr/idr_schema.json` using the Draft 2020-12 `jsonschema` implementation.

Examples:

```bash
python -m pip install -r tools/requirements.txt
python tools/validate_idr.py snapshot.json
python tools/validate_idr.py - < snapshot.json
```

Exit codes:

- `0`: valid document
- `1`: invalid JSON or schema validation failure
- `2`: validator dependency is unavailable

The validator does not repair data or infer missing hardware information.

### Hardware fingerprint

`tools/hardware_fingerprint.py` derives a SHA-256 fingerprint from a deliberately small set of observed hardware identity fields:

- GPU UUID and serial when available;
- GPU name and brand when available;
- PCI location and raw device/subsystem IDs when available;
- total GPU memory when available;
- observed GPU count.

Only observations whose IDR state is `available` participate. Unknown or unavailable observations are represented as `null`, not guessed.

The tool is **not** a V100 classifier and contains no NVIDIA model lookup table. It identifies an observed hardware configuration so verification artifacts can be associated with the configuration that produced them.

Examples:

```bash
python tools/hardware_fingerprint.py snapshot.json
python tools/hardware_fingerprint.py snapshot.json --json
```

Changing an included observed field changes the fingerprint. Therefore this fingerprint should not be treated as a permanent identity for a physical GPU across hardware replacement or configuration changes.

### Verification manifest schema

`diagnostics/verification/manifest.schema.json` defines metadata for a verification or benchmark run. It records:

- run identity, timestamp, purpose and notes;
- GPU count and optional per-GPU identity/variant observations;
- hardware fingerprint;
- OS, kernel, driver, CUDA, compiler and library versions;
- benchmark name/version and parameters;
- timing method, unit and formula;
- optional GPU-state-change declarations;
- optional result status and artifact references.

The manifest is provenance metadata. It does not turn an experimental result into a guaranteed hardware specification.

## Intended pipeline

```text
NVML hw_probe
      |
      v
    IDR JSON
      |
      +----> validate_idr.py ----> schema PASS/FAIL
      |
      +----> hardware_fingerprint.py
      |
      +----> verification manifest ----> reproducible benchmark record
```

The tools do not require a physical V100 for unit-level parsing and schema tests. Actual hardware claims still require real hardware observations and, where applicable, real benchmark execution.

## Scope exclusions

These tools intentionally do not provide:

- GPU tuning or overclocking;
- power, clock, ECC or persistence controls;
- CUDA execution management;
- scheduler or workload policy;
- driver installation;
- game compatibility or graphics emulation;
- V100 model inference from incomplete data.
