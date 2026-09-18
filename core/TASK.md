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


## Foundation ownership audit result — #29

The current repository audit establishes the following:

- `core/` exists only as a specification area at this point; there is no `core/CMakeLists.txt` or production `core` target on the audited `main`.
- `core/TASK.md` defines provider-neutral orchestration and normalized runtime state. It does not establish ownership of `Status`, `Result<T>`, `ErrorCode` or other foundation primitives.
- `common/` contains the implemented foundation primitives currently used by the repository.
- Therefore `common` remains the foundation owner for these primitives.
- No `common → core` migration is justified by the current evidence.
- The existence of `core/TASK.md` is not evidence that a separate low-level driver plane is implemented.
- Any future driver-plane architecture must be established from production code and dependency ownership before being encoded as a repository invariant.

The `Result<T>` contract is hardened so release/NDEBUG builds do not rely on assertions for the basic success/failure invariant.
