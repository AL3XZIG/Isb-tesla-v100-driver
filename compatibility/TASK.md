# Compatibility — Technical Task

## Goal
Implement provider/API/application-specific compatibility paths above the base driver.

## Must implement
- API capability checks.
- Compatibility profiles by application/version/API/driver/GPU.
- Configuration and shim selection without hidden global mutations.
- Per-application overrides with clear precedence.
- Safe enable/disable and verification.

## Rules
- Compatibility logic cannot claim a missing hardware feature exists.
- Prefer configuration/workaround paths over invasive interception.
- Every shim/interception must document scope, lifetime and rollback.

## Acceptance
- At least one fake application profile can be diagnosed and applied end-to-end.
