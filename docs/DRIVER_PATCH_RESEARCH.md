# Driver Patch Research Layer

## Purpose

This directory contains **research-only** tooling for studying software restrictions in the installed NVIDIA Linux userspace stack.

It does not replace the NVIDIA kernel/user-mode driver and it is not part of the normal ISB runtime path.

The initial reference is the open-source [Forceware-382.69](https://github.com/SupraGSX/Forceware-382.69) project. ISB uses it as an engineering reference for reproducible patch descriptions, input validation, byte-level preconditions, source/binary verification and fail-closed behavior.

ISB must independently implement these ideas. NVIDIA binaries, proprietary source, firmware and vendor-derived code are not copied into this repository.

## Responsibilities

The research layer may eventually provide:

- driver fingerprinting;
- version/architecture/GPU matching;
- patch-profile discovery;
- signature and expected-byte validation;
- dry-run patch analysis;
- pre/post hash verification;
- source-to-binary verification for independently authored research code;
- backup and rollback metadata;
- deterministic evidence reports.

It must **not** silently modify the installed driver.

## Patch lifecycle

A future experimental patch follows:

```
SCAN
  -> FINGERPRINT
  -> MATCH PROFILE
  -> CHECK PRECONDITIONS
  -> DRY RUN
  -> EXPLICIT APPROVAL
  -> BACKUP
  -> APPLY
  -> READ-BACK
  -> VERIFY
  -> REPORT
  -> ROLLBACK on failure
```

Stable ISB paths must never treat a research patch as available merely because a profile exists.

## Patch categories

Every research result is classified as one of:

- **UNLOCK** — an existing hardware/driver capability is exposed by software changes.
- **EMULATION** — a missing hardware feature is approximated by CUDA/Vulkan/other software.
- **IMPOSSIBLE** — the required capability depends on hardware that GV100 does not contain or cannot practically replace.
- **UNKNOWN** — evidence is insufficient.

The system must never turn UNKNOWN into AVAILABLE.

## Layer model

Research must identify where a restriction occurs:

```
Hardware
  -> Device identification
  -> Driver acceptance
  -> Capability detection
  -> Capability advertisement
  -> API exposure
  -> User-mode implementation
  -> Kernel implementation
  -> Hardware execution
```

Changing a device ID or capability flag is not considered proof that the underlying feature works.

## Proposed profile format

A future profile can use a declarative manifest such as:

```yaml
id: example-v100-research-patch
target:
  vendor: NVIDIA
  gpu: GV100
  architecture: sm70
  driver_branch: "example"

preconditions:
  file_sha256: "<exact hash>"
  signatures: []
  expected_bytes: []

edits: []

verification:
  post_hash: "<optional exact hash>"
  runtime_checks: []

rollback:
  required: true
```

The exact schema is intentionally not frozen until an implementation owner and tests exist.

## Safety rules

1. Unknown driver versions are rejected by default.
2. Unknown file hashes are rejected by default.
3. Expected bytes/signatures must match before an edit.
4. Patches are never selected solely by file offset.
5. Backups precede reversible mutations.
6. Failed verification triggers rollback where technically possible.
7. No online/anti-cheat game process is modified by this research layer.
8. Stable Hub functionality must not depend on experimental patches.
9. Every experiment records provenance and evidence.
10. A patch cannot claim a hardware capability that is absent from GV100.

## Current status

**Design/research only.** No automatic NVIDIA driver binary patching is implemented by this document.
