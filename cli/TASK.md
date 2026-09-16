# CLI — Technical Task

## Goal
Provide the primary automation and recovery interface; GUI must remain optional.

## Commands
- `isb scan`
- `isb diagnose`
- `isb fix --dry-run`
- `isb fix`
- `isb verify`
- `isb report`
- `isb profile list|show|apply`
- `isb driver list|inspect|rollback`

## Must implement
- Stable exit codes.
- Machine-readable JSON output.
- Human-readable output.
- Non-interactive mode for CI/regression runs.
- Confirmation before destructive/system-changing actions.

## Acceptance
- Complete scan/diagnose/dry-run/verify flow works without GUI.
