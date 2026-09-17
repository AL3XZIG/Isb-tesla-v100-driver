# Integrations — External Components

## Goal
Integrate external user-space compatibility projects through auditable, replaceable boundaries owned by the V100 Hub.

## Initial targets
- OptiScaler.
- DXVK/VKD3D where compatibility work requires them.
- Other user-space compatibility tools only after technical and license review.

## OptiScaler requirements
- Detect installed version and provenance.
- Maintain compatibility metadata by game/API/environment.
- Support explicit install, update, configure, remove and rollback operations.
- Back up target files and verify hashes.
- Keep per-game configuration rather than silent global injection.
- Provide deterministic operation logs.
- Track license/SPDX and source/provenance obligations.
- Never silently download arbitrary binaries.

## General requirements
- Versioned integration descriptors.
- Separate process/library boundary where appropriate.
- Configuration import/export.
- Health/version detection.
- License and provenance metadata.
- No copied third-party source without explicit approval and audit.

## Acceptance
- An integration can be disabled without breaking core ISB.
- Unsupported or missing external components become a clear unavailable/unknown state.
- Every mutating integration operation is explicit, auditable and reversible where technically possible.
