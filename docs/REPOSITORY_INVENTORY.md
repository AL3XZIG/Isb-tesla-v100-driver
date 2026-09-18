# Repository Inventory

Status: baseline audit

This document records the current repository state before architecture consolidation work.

## Purpose

Create a verified inventory of existing components before PR #29-#37 refactoring work.

## Rules

- This file describes the current repository, not the target architecture.
- Planned features must not be marked as implemented.
- New findings should update this inventory.

## Component status

| Component | Status | Notes |
| --- | --- | --- |
| common | Existing | Contains shared primitives currently used by parts of the project |
| core | Requires audit | Ownership and implementation status must be verified |
| CAL | Existing | Capability model area |
| providers/nvml | Existing | NVIDIA observation/provider layer |
| experiments | Existing | Experimental modules require consolidation review |
| hub | Existing | Candidate canonical control plane |
| control-center | Existing | Requires Hub integration review |
| diagnostics | Existing | Verification/reporting paths require consolidation |

## Verification policy

Architecture decisions must be based on source code and build targets, not only documentation.
