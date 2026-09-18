# PR #29 Foundation Ownership Audit Plan

## Goal

Audit the repository foundation before architectural consolidation.

## Scope

- Identify current owners of Result, Status, ErrorCode, CapabilityState and Observation types.
- Verify core vs common responsibilities.
- Verify Hub and legacy ControlPlane relationships.
- Review CMake targets and dependency boundaries.

## Non-goals

- No migrations.
- No renames.
- No new APIs.
- No GUI/CLI implementation.
- No provider feature expansion.

## Acceptance Criteria

- Repository inventory is factual.
- Ownership conflicts are documented.
- Production code and documentation are distinguished.
- Future refactoring decisions are based on verified findings.
