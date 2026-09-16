# Installer — Technical Task

## Goal
Provide optional installation/configuration workflows for ISB itself and validated driver profiles.

## Must implement
- Preflight environment checks.
- Install/uninstall ISB components.
- Driver profile staging without bundling proprietary NVIDIA packages.
- Configuration backup and restore.
- Verification after installation.
- Rollback on failed configuration.

## Rules
- Never silently download or redistribute proprietary drivers.
- CLI must expose the same operations as the GUI.

## Acceptance
- Fresh install and rollback are reproducible in a test environment.
