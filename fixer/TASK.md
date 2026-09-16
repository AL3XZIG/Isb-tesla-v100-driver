# Fixer — Technical Task

## Goal
Implement the Driver Fixer state machine: fingerprint -> diagnose -> candidate -> dry-run -> apply -> verify -> rollback.

## Must implement
- Rule matching against GPU/driver/OS/API/application fingerprints.
- Multiple candidates with prerequisites and risk metadata.
- Dry-run that performs no system mutation.
- Explicit action interface.
- Verification after every mutating action.
- Rollback contract for reversible actions.
- Audit trail containing rule, evidence, action and result.

## Non-goals
- Writing a kernel driver.
- Inventing unsupported API functionality.
- Applying unverified workarounds silently.

## Acceptance
- A fake issue can execute through the full lifecycle in tests.
- Failed verification produces a failed result and invokes rollback when supported.
