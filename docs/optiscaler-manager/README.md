# ISB OptiScaler Manager

## Status

**Planned — documentation/specification only.** This directory does not mean the manager has been implemented or tested.

ISB OptiScaler Manager will manage an external OptiScaler installation for explicitly selected games on the currently supported platform, Linux x86-64. It will integrate through the canonical ISB Hub and follow the project's evidence, provenance, and reversible-mutation rules.

OptiScaler is an external project/component. ISB will provide its own management and integration layer; it must not imply that ISB owns OptiScaler or that OptiScaler binaries are bundled with ISB.

## Goals

- Detect existing OptiScaler files, version (when reliably identifiable), configuration, and provenance for a selected game.
- Explain whether the detected game/runtime/API combination is known-compatible, incompatible, or unknown based on recorded evidence.
- Discover upstream release metadata and artifacts from trusted official project sources.
- Verify artifact integrity before use. Prefer upstream-published checksums/signatures; if no trusted digest or signature exists, disclose that limitation and require explicit user approval rather than inventing verification.
- Preview every change with a dry-run plan.
- Back up affected files and configuration before install, update, or removal.
- Apply changes transactionally where practical, read back resulting state, and support rollback.
- Keep per-game configuration explicit, inspectable, and reversible.
- Produce a diagnostic report with artifact identity, hashes, source URL, license/provenance information, selected game/runtime/API, actions, and verification results.
- Keep CLI and future Control Center workflows behind the canonical Hub; UI must not access providers or mutate files directly.

## Planned functional areas

### 1. Discovery and inventory

- User explicitly selects or confirms the game directory.
- Detect likely OptiScaler components and configuration files without modifying them.
- Record file paths, version evidence, hashes, and detection confidence.
- Distinguish `PRESENT`, `ABSENT`, and `UNKNOWN`; missing permissions or ambiguous files must not be treated as absence.
- Do not scan or change arbitrary directories without user consent.

### 2. Compatibility assessment

- Collect evidence for game, executable, graphics API, Wine/Proton runtime, OptiScaler version, and configuration.
- Return `COMPATIBLE`, `INCOMPATIBLE`, or `UNKNOWN` with reasons and evidence sources.
- Do not infer universal support from a successful download or file detection.
- Keep anti-cheat/online-game status conservative. Never silently change anti-cheat, integrity, or online-play configuration.

### 3. Upstream artifact management

- Maintain an auditable source/provenance record for release metadata and downloads.
- Download only from configured/trusted upstream locations.
- Validate published checksum/signature where available and reject mismatches.
- Use safe temporary files and atomic replacement where practical.
- Do not execute downloaded artifacts as part of installation.
- Track applicable license/SPDX and redistribution constraints. Do not bundle external binaries in ISB releases without an explicit licensing review.

### 4. Transaction lifecycle

Expected lifecycle:

`inspect -> plan -> user approval -> backup -> apply -> read-back -> verify -> report`

- `dry-run` must not mutate files.
- Install/update/remove must show the exact target files and actions before approval.
- Preserve pre-existing files and configurations in a backup manifest.
- On failure, attempt rollback and report rollback success/failure separately.
- Never report mutation success without read-back verification.
- Avoid following symlinks outside the selected game root; validate paths and defend against path traversal and symlink/TOCTOU issues.

### 5. Per-game configuration

- Store configuration per game, not as a global implicit change.
- Show configuration diffs before applying.
- Keep a backup and restore path for every modified file.
- Do not silently inject DLL overrides or change Steam/Proton launch options; present the proposed change and require explicit approval.
- Preserve user-owned settings where possible.

### 6. Evidence and reports

Reports should identify:
- ISB version and manager schema version.
- Game identity/path and detection evidence.
- OS, Wine/Proton version, graphics API/runtime when available.
- OptiScaler version, artifact source, license/provenance, and hashes.
- Planned and executed operations.
- Backup/rollback locations and verification results.
- Which observations are real, inferred, unavailable, or synthetic.
- Test status: unit-tested, manually tested, or not tested on real hardware.

Do not include secrets, authentication tokens, or unnecessary personal paths in shareable reports.

## Initial Linux/Proton proof of concept

The first end-to-end proof should target **one explicitly selected game** and a documented, compatible runtime/API combination. It should:

1. Capture the pre-change game/runtime/configuration state.
2. Run a dry-run and review the proposed changes.
3. Back up affected files.
4. Apply the minimal required integration.
5. Launch and verify the game/render path where a suitable test environment exists.
6. Capture logs and frame-time/FPS baseline-versus-candidate data if measurable.
7. Restore the original state and verify rollback.

A mocked test or successful file installation is not proof that the game renders correctly or that performance improves. Mark real game/V100 qualification as `NOT_TESTED` until it has actually been run.

## Tesla V100-specific limitations

Tesla V100 (Volta) has no dedicated RT cores and does not provide a native DLSS/Optical Flow hardware path. OptiScaler integration does not add those hardware blocks. Any usable upscaling/reconstruction path must be verified for the specific game, graphics API, runtime, OptiScaler version, and backend; software reconstruction or alternative upscaling must be labelled accurately. Frame generation must not be advertised as supported unless an actual compatible path is demonstrated.

## Non-goals for the first version

- Replacing NVIDIA's kernel or userspace driver.
- Claiming support for all games, APIs, upscalers, or Proton versions.
- Silent system-wide graphics changes.
- Automatic modification of anti-cheat or online-game protections.
- Shipping OptiScaler binaries without license/provenance review.
- Claiming V100 performance gains before reproducible measurement.

## Acceptance criteria

- Deterministic unit tests cover discovery, status semantics, compatibility decisions, dry-run, path safety, integrity-check failures, backup, partial failure, rollback, and report generation.
- Tests use temporary fixtures and do not require a real game or GPU in CI.
- A dry-run makes no changes.
- All mutating operations require explicit approval and post-operation read-back verification.
- Errors and rollback failures are visible; no silent success.
- Documentation distinguishes automated tests from manual game and physical-V100 qualification.
- Roadmap and implementation-status documents are updated only to reflect code and evidence that actually exist.

## References

- OptiScaler upstream: https://github.com/optiscaler/OptiScaler
- ISB project roadmap: ../ROADMAP_STATUS.md
- GitHub planning issue: https://github.com/AL3XZIG/Isb-tesla-v100-driver/issues/81
