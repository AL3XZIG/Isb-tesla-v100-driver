#!/usr/bin/env python3
"""Validate an ISB IDR snapshot against the checked-in JSON Schema.

The validator is intentionally read-only: it never normalizes, repairs, or infers
hardware data. Input is read from a file or stdin and validation errors are
reported with their JSON path when the jsonschema package is available.
"""
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

SCHEMA_DEFAULT = Path(__file__).resolve().parent.parent / "diagnostics" / "idr" / "idr_schema.json"


def _load(path: str) -> Any:
    if path == "-":
        return json.load(sys.stdin)
    with open(path, "r", encoding="utf-8") as handle:
        return json.load(handle)


def _format_path(path: Any) -> str:
    parts = list(path)
    if not parts:
        return "$"
    result = "$"
    for part in parts:
        result += f"[{part}]" if isinstance(part, int) else f".{part}"
    return result


def validate(document: Any, schema: Any) -> list[str]:
    try:
        import jsonschema
    except ImportError:
        print("validate_idr.py requires the 'jsonschema' Python package.", file=sys.stderr)
        print("Install it with: python -m pip install jsonschema", file=sys.stderr)
        raise SystemExit(2)

    validator = jsonschema.Draft202012Validator(schema)
    errors = sorted(validator.iter_errors(document), key=lambda error: list(error.absolute_path))
    return [f"{_format_path(error.absolute_path)}: {error.message}" for error in errors]


def main() -> int:
    parser = argparse.ArgumentParser(description="Validate an ISB IDR JSON snapshot.")
    parser.add_argument("input", help="IDR JSON file, or '-' for stdin")
    parser.add_argument("--schema", default=str(SCHEMA_DEFAULT), help="JSON Schema path")
    args = parser.parse_args()

    try:
        document = _load(args.input)
        schema = _load(args.schema)
    except (OSError, json.JSONDecodeError) as exc:
        print(f"FAIL: {exc}", file=sys.stderr)
        return 1

    errors = validate(document, schema)
    if errors:
        print(f"FAIL: {len(errors)} validation error(s)")
        for error in errors:
            print(f"- {error}")
        return 1

    print("PASS: IDR snapshot conforms to schema")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
