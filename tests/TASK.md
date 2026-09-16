# Tests — Technical Task

## Goal
Prevent the project from turning into a collection of unverified compatibility hacks.

## Test layers
- Unit tests for pure logic.
- Provider tests with mocked/fake observations.
- Compatibility tests with synthetic fingerprints.
- Regression tests for every accepted workaround.
- Hardware tests for physical V100 validation.
- Cross-driver matrix tests.

## Must implement
- Deterministic fixtures.
- Expected provenance and state assertions.
- Negative tests for UNKNOWN/UNAVAILABLE.
- Regression artifacts that record exact driver revision and environment.

## Acceptance
- CI covers unit/provider/compatibility tests.
- Hardware-only tests are clearly marked and never treated as passing on CI without hardware.
