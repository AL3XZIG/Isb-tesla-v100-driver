# ISB Linux Release Builder and Driver Manager

## 1. Purpose

This document defines the lifecycle requirements for the **Linux x86-64-only** ISB product:

1. **Release Builder** — builds, tests, packages and publishes ISB.
2. **Driver Manager** — discovers relevant Linux NVIDIA driver branches and versions, verifies official artifacts and maintains a local cache.

The components remain separate from the low-level GPU/provider layer.

Architecture:

`Control Center / CLI -> Hub -> DriverRepository / Release services`

ISB does not replace the installed NVIDIA Linux base driver.

---

# 2. Linux Release Builder

The user should be able to build a release with:

```bash
python tools/release/release.py
```

Supported selectors:

```bash
python tools/release/release.py --target linux
python tools/release/release.py --dry-run
python tools/release/release.py --clean
python tools/release/release.py --verbose
```

The pipeline is:

```text
CHECK ENVIRONMENT
      |
DETERMINE VERSION
      |
READ GIT COMMIT/TAG
      |
CMAKE CONFIGURE
      |
BUILD
      |
TEST
      |
PACKAGE
      |
SHA256
      |
MANIFEST
      |
DIST
```

A failed required stage stops the release. A physical V100 is not required for the normal software build; physical V100 qualification is a separate gate.

## 2.1 Environment

Verify:

- Git;
- Python where required;
- CMake;
- C++17 toolchain;
- Linux packaging tools for the selected package format.

Errors must state what is missing and how to resolve it.

## 2.2 Artifacts

Initial release targets:

### Linux x86-64

- `.deb`;
- `.tar.gz`.

Naming:

```text
isb-<version>-linux-x64.<extension>
```

Examples:

```text
isb-0.1.0-linux-x64.deb
isb-0.1.0-linux-x64.tar.gz
```

Do not publish an artifact until its build/package job succeeds.

## 2.3 Manifest and checksums

Every release contains:

```text
SHA256SUMS
manifest.json
```

The manifest records verifiable information:

- schema version;
- ISB version;
- Git commit/tag;
- build timestamp;
- Linux architecture;
- compiler/toolchain where practical;
- artifact name/format/size;
- SHA-256;
- enabled build features where useful;
- test result.

---

# 3. GitHub Actions

Desired workflow:

```text
git tag vX.Y.Z
      |
      v
Linux build
      |
      +--> tests
      |
      +--> package
      |
      v
collect artifacts
      |
      v
SHA256 + manifest
      |
      v
GitHub Release
```

Release workflows trigger on semantic version tags such as `v0.1.0`.

The publish job depends on build, test, package, checksum and manifest jobs. Failed gates prevent publication.

Generated binaries are never committed into `main`.

---

# 4. Linux Driver Manager

Driver management is a separate product subsystem.

Responsibilities:

- discover relevant Linux NVIDIA releases;
- expose branches and versions;
- determine V100 compatibility from evidence;
- show official/source URLs;
- download artifacts;
- verify artifacts;
- maintain a local cache;
- expose driver information through Hub, CLI and Control Center;
- optionally install/configure a driver only through an explicit supported privileged backend.

The Driver Manager never silently replaces the active driver.

## 4.1 Provider model

Initial supported source:

- **NVIDIA Linux/Data Center**;
- local cache.

Google-provided or other vendor-specific Windows packages are not product targets.

A driver record retains:

- vendor;
- family/branch;
- version;
- Linux distribution/package type;
- architecture;
- V100 variant compatibility evidence;
- source URL;
- download URL;
- release date when known;
- SHA-256/signature when available;
- provenance.

Unknown fields remain `UNKNOWN`.

## 4.2 Compatibility

Compatibility is evaluated against:

- Linux x86-64;
- exact V100 variant where known;
- NVIDIA driver branch/version;
- installed runtime;
- required API path;
- project compatibility evidence.

States:

```text
COMPATIBLE
INCOMPATIBLE
UNKNOWN
```

Absence of evidence is not proof of incompatibility.

## 4.3 Download pipeline

```text
DISCOVER
  -> SELECT
  -> SHOW METADATA
  -> USER CONFIRMATION
  -> DOWNLOAD TEMPORARY FILE
  -> SIZE VALIDATION
  -> SHA256
  -> SIGNATURE VERIFICATION WHEN AVAILABLE
  -> ATOMIC CACHE MOVE
  -> PROVENANCE RECORD
  -> REPORT
```

Rules:

- use trusted official sources;
- use temporary files;
- never treat partial downloads as valid;
- verify checksums when available;
- preserve source URLs;
- never execute downloaded installers automatically.

Download and installation are separate operations.

## 4.4 Cache

The cache identity includes at least:

- vendor;
- branch;
- version;
- Linux distribution;
- architecture;
- package type;
- checksum.

A version number alone is not sufficient artifact identity.

## 4.5 Installation boundary

If a Linux driver installation backend is implemented, the workflow is:

```text
DETECT CURRENT DRIVER
  -> CHECK CANDIDATE
  -> SHOW RISKS
  -> BACKUP/RECOVERY INFORMATION
  -> USER APPROVAL
  -> INSTALL
  -> REBOOT IF REQUIRED
  -> RE-DETECT
  -> VERIFY
  -> REPORT / ROLLBACK
```

The normal ISB installer installs ISB and configures ISB. It does not silently replace the NVIDIA base driver.

---

# 5. Linux render-path configuration

Driver management and render routing are separate concerns.

The intended V100 topology is:

```text
iGPU / secondary dGPU
        |
     display
        |
Linux session
        |
   application
        |
   V100 render
        |
 Vulkan / OpenGL / CUDA
```

ISB should prefer application-scoped routing using mechanisms actually available on the host, such as:

- NVIDIA PRIME Render Offload;
- Vulkan device selection;
- OpenGL vendor selection;
- per-application environment/configuration.

Configuration is not considered successful until the selected render device is verified.

---

# 6. CLI

The CLI mirrors Hub semantics:

```bash
isb drivers list
isb drivers search <query>
isb drivers info <version>
isb drivers compatible
isb drivers download <artifact>
isb drivers verify <file>
isb drivers cache list
isb drivers cache remove <artifact>
```

Release commands:

```bash
isb release build
isb release build --target linux
isb release verify
```

JSON output should use the existing deterministic CLI conventions.

---

# 7. Security and provenance

Requirements:

- HTTPS for supported downloads;
- temporary download files;
- checksum verification;
- signature verification when available;
- no automatic execution of downloaded packages;
- no silent driver replacement;
- least-privilege installation;
- provenance retained in reports/cache;
- no proprietary NVIDIA binaries bundled into ISB releases without an explicit legal basis.

The local cache is a user-side artifact cache, not an ISB-owned driver mirror.

---

# 8. Acceptance criteria

The Linux lifecycle is complete when:

- Linux x86-64 is the only supported release target;
- Linux DEB/TAR.GZ artifacts build successfully;
- tests gate releases;
- SHA256SUMS and a release manifest are generated;
- GitHub Releases can be produced from version tags;
- Linux NVIDIA driver versions can be discovered/searched;
- source/provenance metadata is preserved;
- V100 compatibility is evidence-based;
- downloads are verified and safely cached;
- download and installation remain separate;
- ISB never silently replaces the active NVIDIA driver;
- Linux render routing can be dry-run, applied and verified where supported;
- proprietary third-party driver packages are not silently redistributed.
