# Profiles — Technical Task

## Goal
Represent reusable combinations of GPU, driver, OS, API, application and ISB behavior.

## Must implement
- Base-driver profiles.
- Hardware profiles for V100 SXM2/PCIe and 16/32 GB variants.
- Application/game profiles.
- Compatibility and enhancement presets.
- Version constraints and prerequisites.
- Provenance and validation state.

## Rules
- Profiles are declarative; execution belongs to fixer/runtime components.
- No profile may silently enable an unverified workaround.

## Acceptance
- Profiles can be listed, inspected, matched, applied and exported through CLI.
