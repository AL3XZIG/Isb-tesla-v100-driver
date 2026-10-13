# Implementation task: OptiScaler Manager

**Type:** implementation plan for an AI coding agent  
**Status:** not implemented; read this before changing code.

Repository: https://github.com/AL3XZIG/Isb-tesla-v100-driver  
Specification: [README.md](README.md)  
Tracking issue: https://github.com/AL3XZIG/Isb-tesla-v100-driver/issues/81

## Instructions

First inspect the current repository, `CURRENT_IMPLEMENTATION.md`, `docs/ROADMAP_STATUS.md`, the canonical Hub interfaces, provider patterns, report schemas, build system, and tests. Do not assume paths or APIs from this task exist. Follow existing conventions and propose a short implementation plan before editing.

Implement in small, testable slices. Keep all functionality behind the canonical Hub. Do not refactor unrelated modules or mark planned features complete without source/tests.

## Slice 1 — Read-only inventory and contracts

- Define backend-neutral domain types for inventory, compatibility evidence, artifact provenance, operation plan, transaction result, and report.
- Use the project's existing status/error conventions where available; otherwise document explicit tri-state semantics.
- Add read-only inspection of a user-selected game directory.
- Detect likely OptiScaler files/configuration and report version only when supported by concrete evidence.
- Include hashes and provenance fields without claiming that a hash alone proves authenticity.
- Reject unsafe paths and avoid following symlinks outside the selected root.
- Add fixture-based unit tests.

**Gate:** build/tests pass and inspection is demonstrably read-only.

## Slice 2 — Plan and transaction safety

- Add a dry-run plan that lists every intended file/configuration change.
- Require explicit caller approval before mutation.
- Implement backup manifest, install/update/remove plan, post-write read-back verification, and rollback.
- Handle partial failure, permission errors, existing user files, path traversal, symlink attacks, and interrupted operations.
- Do not report success unless the final state is verified.
- Add failure-injection and rollback tests.

**Gate:** dry-run has no side effects; failed operations produce explicit results and restore state where possible.

## Slice 3 — Trusted upstream artifacts

- Implement source metadata discovery only from reviewed upstream endpoints.
- Store URL, release identity, retrieval time, hash, and license/provenance data.
- Verify upstream-published checksum/signature when available.
- If upstream provides no verifiable signature/digest, mark trust as limited and require explicit user approval; never fabricate a checksum or authenticity claim.
- Keep downloading separate from installation; never execute downloaded content.
- Do not bundle OptiScaler artifacts into ISB packages.

**Gate:** tests cover bad hashes, unavailable metadata, untrusted URLs, network errors, and incomplete downloads.

## Slice 4 — Linux/Proton integration proof

- Document a manual test recipe for one selected game and exact runtime/API combination.
- Detect and display the relevant runtime/configuration state where feasible.
- Make DLL override, launch-option, and game-file changes explicit, previewable, and reversible.
- Capture before/after state and frame-time/FPS data when the environment allows.
- Do not claim a successful V100 render or performance improvement from unit tests/mocks.

**Gate:** a real game test is marked PASS only with reproducible evidence; otherwise it remains NOT_TESTED.

## Cross-cutting requirements

- Linux x86-64 is the current supported platform.
- CLI/GUI must call through Hub; no direct provider or filesystem mutation from UI.
- Keep game/OptiScaler compatibility evidence-specific; unknown means UNKNOWN, not compatible.
- Never silently alter anti-cheat or online-play protections.
- Track licensing/SPDX and artifact provenance; external distribution requires explicit review.
- Update `docs/ROADMAP_STATUS.md` and `CURRENT_IMPLEMENTATION.md` only after corresponding implementation and tests exist.
- Keep experimental integration isolated from stable Hub dependencies until validated.
- Do not implement native DLSS/RT/Optical Flow claims for V100; report only demonstrated software/backend paths.

## Final response expected from the coding agent

Report:
1. Files changed and architecture decisions.
2. Build/test commands and actual results.
3. Safety limitations and unsupported cases.
4. What remains unimplemented.
5. Whether any real game or physical V100 was tested (never imply that if it was not).
