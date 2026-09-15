#!/usr/bin/env python3
from __future__ import annotations

import json
import sys
import unittest
from pathlib import Path

TOOLS = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(TOOLS))

from hardware_fingerprint import fingerprint  # noqa: E402
from validate_idr import validate  # noqa: E402

SCHEMA = json.loads((TOOLS.parent / "diagnostics" / "idr" / "idr_schema.json").read_text(encoding="utf-8"))


def obs(value=None, state="unknown"):
    return {"state": state, "value": value}


def valid_idr():
    return {
        "schema_version": "1.0",
        "timestamp": "2026-09-15T12:00:00Z",
        "host": {"hostname": obs("test"), "platform": obs("Linux"), "kernel": obs("test"), "machine": obs("x86_64")},
        "os": {"name": obs("Linux"), "release": obs("test")},
        "driver": {"version": obs("test")},
        "cuda": {"driver_version": obs(None), "runtime_version": obs(None)},
        "gpus": [],
    }


class VerificationToolsTests(unittest.TestCase):
    def test_valid_idr(self):
        self.assertEqual(validate(valid_idr(), SCHEMA), [])

    def test_unknown_observation_cannot_have_value(self):
        document = valid_idr()
        document["driver"]["version"] = obs("must-not-be-present")
        errors = validate(document, SCHEMA)
        self.assertTrue(errors)

    def test_fingerprint_is_deterministic(self):
        snapshot = {"gpus": [{"identity": {"uuid": obs("GPU-a")}, "pci": {}, "memory": {}}]}
        first, material = fingerprint(snapshot)
        second, _ = fingerprint(snapshot)
        self.assertEqual(first, second)
        self.assertEqual(material["gpu_count"], 1)

    def test_unknown_identity_does_not_enter_fingerprint_as_value(self):
        snapshot = {"gpus": [{"identity": {"uuid": obs(None), "name": obs("GPU")}, "pci": {}, "memory": {}}]}
        digest_a, _ = fingerprint(snapshot)
        snapshot["gpus"][0]["identity"]["uuid"] = obs("GPU-a")
        digest_b, _ = fingerprint(snapshot)
        self.assertNotEqual(digest_a, digest_b)


if __name__ == "__main__":
    unittest.main()
