# Repository Inventory

Status: baseline audit

This document records the verified repository state before architecture consolidation work.

## Purpose

Provide a source-of-truth inventory before PR #29-#37 refactoring.

## Rules

- Current state only. Do not describe planned architecture as implemented.
- Verify source code and build targets before architecture changes.
- New modules must declare ownership and status.
- Documentation is not proof of implementation.

## Initial inventory

| Component | Status | Review |
| --- | --- | --- |
| common | Existing | Foundation ownership audit required |
| core | Requires audit | Confirm production ownership |
| CAL | Existing | Capability ownership candidate |
| providers/nvml | Existing | Hardware observation layer |
| experiments | Existing | Consolidation review required |
| hub | Existing | Control plane candidate |
| control-center | Existing | Hub integration review required |
| diagnostics | Existing | Reporting and verification review required |

## Refactoring gate

PR #29-#37 must update this inventory only after decisions are verified by code, CMake targets, and tests.
