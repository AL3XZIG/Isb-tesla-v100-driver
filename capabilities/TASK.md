# Capabilities — Technical Task

## Goal
Define the separation between hardware capability, base-driver capability, observed runtime capability and ISB-added capability.

## Must implement
- Stable capability identifiers.
- State model: available / unavailable / unknown.
- Provenance for every observed capability.
- Driver capability and hardware capability must never be conflated.
- Conversion into CAL-compatible normalized data.
- Capability comparison/diff for driver matrix testing.

## Acceptance
- Negative/unknown cases are represented without guessed values.
- A capability can be traced to the exact probe, driver fingerprint and timestamp.
