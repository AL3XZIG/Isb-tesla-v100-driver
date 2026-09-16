# Core — Technical Task

## Goal
Provide provider-neutral orchestration and normalized runtime state for ISB.

## Must implement
- Environment/session model.
- Deterministic orchestration of detector, diagnosis, action and verification stages.
- Explicit state transitions and error propagation.
- No direct OS-specific probing in core.
- No NVIDIA-specific assumptions in public core interfaces.
- Support JSON/IDR-compatible normalized results without owning serialization policy.

## Acceptance
- Unit tests cover success, unknown capability, unavailable probe, action failure and verification failure.
- Same input fingerprint produces deterministic diagnosis input/output.
- Core can run without GUI.
