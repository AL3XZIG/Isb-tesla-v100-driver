# Drivers — Technical Task

## Goal
Identify, stage, configure, verify and roll back supported base-driver packages without becoming a driver distributor.

## Must implement
- Driver fingerprint: provider, branch, version, package/type, kernel/user components where observable.
- Driver profile registry for validated revisions such as Google GRID/vGPU and NVIDIA stacks.
- Installation/staging abstraction; actual package acquisition remains external/user-supplied unless legally distributable.
- Preflight checks and post-install verification.
- Rollback to a known-good profile.

## Rules
- Never assume newest is best.
- Record exact package/version/source and verification status.
- Never silently replace a working driver.

## Acceptance
- Fake driver lifecycle tests cover detect, stage, verify and rollback.
