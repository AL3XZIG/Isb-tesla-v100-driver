# Repository Inventory

Status: baseline audit

This document records the verified repository state before architecture consolidation work.

## Rules

- Current state only. Do not describe planned architecture as implemented.
- Verify code and build targets before changing architecture.
- New modules must declare ownership and status.

## Initial inventory

| Component | Status | Review |
| --- | --- | --- |
| common | Existing | Foundation ownership audit required |
| core | Requires audit | Confirm production ownership |
| CAL | Existing | Capability ownership candidate |
| providers/nvml | Existing | Hardware observation layer |
| experiments | Existing | Consolidation review required |
| hub | Existing | Control plane candidate |
| control-center | Existing | Hub migration review required |
| diagnostics | Existing | Reporting and verification review required |

## Refactoring gate

PR #29-#37 should update this inventory as decisions become verified.
