# CAL — Technical Task

CAL remains the normalized capability/domain layer inherited from the earlier driver architecture.

## Must preserve
- Hardware identity and capability model.
- Unknown/available/unavailable semantics.
- V100 hardware facts only where verified or explicitly documented.
- Provider/runtime distinctions: CAL describes normalized facts; providers supply observations.

## New role
Consume verified detector/provider observations without assuming that hardware capability equals exposed driver capability.

## Acceptance
Existing CAL tests remain green; new Driver Fixer integration tests do not weaken CAL semantics.
