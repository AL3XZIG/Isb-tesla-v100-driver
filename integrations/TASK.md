# Integrations — Technical Task

## Goal
Integrate external projects through auditable, replaceable boundaries.

## Initial targets
- OptiScaler.
- DXVK/VKD3D where compatibility work requires them.
- Other user-space compatibility tools only after technical and license review.

## Must implement
- Versioned integration descriptors.
- Separate process/library boundary where appropriate.
- Configuration import/export.
- Health/version detection.
- License and provenance metadata.
- No copied third-party source without explicit approval and audit.

## Acceptance
- An integration can be disabled without breaking core ISB.
- Unsupported or missing external components become a clear unavailable state.
