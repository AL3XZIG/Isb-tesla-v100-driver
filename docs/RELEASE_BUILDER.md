# ISB Release Builder and Driver Manager Specification

## 1. Purpose

This document defines the requirements for two related lifecycle components of ISB:

1. **Release Builder** — builds, tests, packages and publishes ISB releases.
2. **Driver Manager** — discovers supported NVIDIA/Google driver branches and versions, presents them in the Control Center/CLI, downloads official artifacts, verifies them and maintains a local cache.

These components are separate from the low-level GPU/provider layer.

The architecture remains:

`Control Center / CLI -> Hub -> DriverRepository / Release services`

ISB does not replace the installed NVIDIA/Google base driver.

---

# 2. Release Builder

## 2.1 Local command

The user should be able to build a release with one command:

```bash
python tools/release/release.py
```

Supported selectors should include:

```bash
python tools/release/release.py --target windows
python tools/release/release.py --target linux
python tools/release/release.py --all
python tools/release/release.py --dry-run
python tools/release/release.py --clean
python tools/release/release.py --verbose
```

The exact CLI may be adapted to the repository's existing conventions.

## 2.2 Build pipeline

The Release Builder performs:

```text
CHECK ENVIRONMENT
      |
      v
DETERMINE VERSION
      |
      v
READ GIT COMMIT/TAG
      |
      v
CLEAN/PREPARE BUILD
      |
      v
CMAKE CONFIGURE
      |
      v
BUILD
      |
      v
TEST
      |
      v
PACKAGE
      |
      v
SHA256
      |
      v
MANIFEST
      |
      v
DIST
```

A failed required stage must stop the release.

A physical Tesla V100 must not be required for the normal software build.

GPU-specific qualification remains a separate test gate.

## 2.3 Environment checks

Before building, verify required tools such as:

- Git;
- Python;
- CMake;
- compiler/toolchain;
- platform-specific packaging tools.

Errors must explain:

- what is missing;
- why it is required;
- how the user can resolve it.

## 2.4 Version source

There must be one canonical project version source.

Preferred release flow:

```text
git tag v0.1.0
      |
      v
Release Builder reads v0.1.0
      |
      v
all artifacts use 0.1.0
```

Do not maintain several independent version numbers.

For local development builds, the builder may derive a development version from the repository state.

## 2.5 Artifacts

Initial target artifacts:

### Windows x64

- `.zip`
- installer `.exe` when the installer backend exists.

### Linux x86_64

- `.deb` when packaging support exists;
- `.tar.gz`.

Naming:

```text
isb-<version>-<platform>-<arch>.<extension>
```

Examples:

```text
isb-0.1.0-windows-x64.zip
isb-0.1.0-windows-x64.exe
isb-0.1.0-linux-x64.deb
isb-0.1.0-linux-x64.tar.gz
```

Do not publish an artifact until its corresponding build/package job succeeds.

## 2.6 dist output

The local build should produce a deterministic `dist/` directory containing:

```text
dist/
├── artifacts
├── SHA256SUMS
└── manifest.json
```

The exact directory layout may follow the repository's existing packaging conventions.

## 2.7 Release manifest

The manifest should contain verifiable information including:

- schema version;
- ISB version;
- Git commit;
- Git tag where applicable;
- build timestamp;
- platform;
- architecture;
- compiler/toolchain where practical;
- artifact name;
- artifact format;
- artifact size;
- SHA-256;
- enabled build features where useful;
- test result.

Do not write unverifiable claims into the manifest.

## 2.8 Checksums

Generate:

```text
SHA256SUMS
```

Every published artifact must have a corresponding hash.

---

# 3. GitHub Actions release automation

The desired workflow is:

```text
git tag vX.Y.Z
      |
      v
GitHub Actions
      |
      +--> Windows build
      |
      +--> Linux build
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

Release workflows should trigger on semantic version tags such as:

```text
v0.1.0
v0.2.0
v1.0.0
```

The publish job must depend on all required build, test, package, checksum and manifest jobs.

A failed job must prevent publication of a partial release.

Generated binaries must not be committed into `main`.

---

# 4. Driver Manager

Driver management is a separate product subsystem.

It does not replace the NVIDIA/Google base driver.

Its responsibilities are:

- discover releases;
- expose driver branches;
- expose versions;
- determine compatibility;
- show official/source URLs;
- download artifacts;
- verify artifacts;
- maintain a local cache;
- expose driver information to Hub, CLI and Control Center;
- optionally install a driver only when a dedicated supported installation backend exists.

---

# 5. Driver provider selection

The application must allow the user to choose a driver source/provider.

Initial providers:

- **NVIDIA**
- **Google**
- local cache

Provider selection must remain explicit.

For example:

```text
Driver source:
[ NVIDIA       v ]

Branch:
[ Data Center  v ]

Version:
[ 582.xx       v ]
```

For Google:

```text
Driver source:
[ Google       v ]

Branch:
[ GPU driver   v ]

Version:
[ 582.16       v ]
```

Do not merge NVIDIA and Google versions into one ambiguous namespace.

A version such as `582.16` must retain its provider/source identity.

---

# 6. Driver version catalog

The Control Center must expose a machine-readable driver catalog.

Suggested structure:

```text
drivers/
├── catalog.json
└── providers/
    ├── nvidia.json
    ├── google.json
    └── local.json
```

The exact repository location may change after implementation audit.

The catalog should contain records such as:

```json
{
  "provider": "google",
  "branch": "gpu-driver",
  "version": "582.16",
  "platform": "windows",
  "architecture": "x86_64",
  "gpu_family": ["Tesla V100"],
  "source_url": "<official release page>",
  "download_url": "<official artifact URL>",
  "release_date": "<date when known>",
  "sha256": "<hash when published>",
  "status": "compatible"
}
```

Unknown fields must remain `UNKNOWN` rather than being guessed.

The catalog must not contain invented or unverified compatibility claims.

---

# 7. Driver version selection in the application

The Driver page should provide:

### Installed driver

Show:

- vendor;
- source/provider;
- branch;
- version;
- package;
- OS;
- architecture;
- runtime/CUDA information where available;
- provenance.

### Available drivers

Allow filtering by:

- provider;
- branch;
- version;
- operating system;
- architecture;
- GPU family;
- compatibility state.

Example conceptual UI:

```text
DRIVER MANAGER

Source
[ NVIDIA v ]

Branch
[ Data Center v ]

Platform
[ Windows x64 v ]

Available versions
------------------------------------------------
Version       Branch          Status
582.16        Data Center     Compatible
580.xx        Data Center     Compatible
535.xx        Legacy          Legacy
------------------------------------------------

[ Inspect ] [ Download ] [ Open Source ]
```

The UI must not rank drivers as "best" without an evidence-based project policy.

Use factual states such as:

- Compatible;
- Incompatible;
- Legacy;
- Experimental;
- Unknown;
- Installed.

---

# 8. Download actions

The user must have explicit choices:

- **Download**
- **Open official/source page**
- **Use local file**
- **Verify**
- **Install**, only when an installation backend actually exists.

Download must not automatically mean install.

Recommended flow:

```text
SELECT PROVIDER
      |
SELECT BRANCH
      |
SELECT VERSION
      |
SHOW METADATA
      |
USER CONFIRMATION
      |
DOWNLOAD TEMPORARY FILE
      |
VERIFY SIZE
      |
VERIFY SHA256
      |
VERIFY SIGNATURE WHEN AVAILABLE
      |
MOVE TO CACHE
      |
REPORT
```

Downloaded driver installers must never be executed automatically.

---

# 9. Automatic download

The Control Center may provide an **Auto Download** action.

It must:

1. resolve the selected catalog entry;
2. use the recorded download URL;
3. download to a temporary location;
4. validate the artifact;
5. verify SHA-256 when available;
6. optionally verify a signature;
7. move the validated artifact into the local cache;
8. record provenance;
9. report success/failure.

If verification fails:

```text
DOWNLOAD FAILED
Reason: SHA-256 mismatch

The artifact was not added to the trusted cache.
```

Corrupted or partial files must not be treated as valid driver packages.

---

# 10. Local cache

The cache must distinguish at least:

- provider;
- branch;
- version;
- OS;
- architecture;
- package type;
- checksum.

A version number alone is not sufficient identity.

Conceptual layout:

```text
cache/drivers/
├── nvidia/
│   └── <artifact identity>/
└── google/
    └── <artifact identity>/
```

The UI should expose:

- cached;
- verified;
- corrupted;
- missing;
- outdated metadata.

---

# 11. Driver compatibility

Compatibility must be evidence-based.

For a Tesla V100 request, consider:

- exact GPU family;
- V100 variant where known;
- OS;
- architecture;
- driver provider;
- driver branch;
- installed driver;
- required API/runtime;
- project compatibility records.

The application must distinguish:

```text
COMPATIBLE
INCOMPATIBLE
UNKNOWN
```

A lack of evidence is not proof of incompatibility.

Do not hard-code one driver version as universally compatible with every V100 environment.

---

# 12. Driver source and provenance

Every driver record should preserve:

- vendor;
- provider;
- branch;
- version;
- source URL;
- download URL;
- release date when known;
- OS;
- architecture;
- package type;
- GPU family;
- checksum;
- signature information;
- discovery timestamp;
- parser/provider version.

The application must be able to show the user where a driver came from.

Third-party driver packages remain third-party software.

ISB must not imply ownership of NVIDIA or Google driver binaries.

---

# 13. Separation between Driver Manager and Release Builder

These are related but independent systems.

### Driver Manager

```text
Find external driver
    -> select
    -> download
    -> verify
    -> cache
    -> optionally install
```

### Release Builder

```text
Build ISB source
    -> test
    -> package
    -> checksum
    -> manifest
    -> publish ISB release
```

The Release Builder must never accidentally package proprietary NVIDIA/Google driver binaries.

The Driver Manager must never modify ISB source/release artifacts.

---

# 14. CLI

The CLI should expose the same Hub semantics.

Suggested commands:

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

Release commands may include:

```bash
isb release build
isb release build --target windows
isb release build --target linux
isb release build --all
isb release verify
```

The exact command names must follow existing CLI conventions.

The CLI must not duplicate backend logic.

---

# 15. Security and legal boundaries

Driver downloads must use trusted sources and preserve provenance.

Requirements:

- HTTPS where supported;
- temporary download files;
- checksum verification;
- signature verification when available;
- no automatic execution;
- no silent driver replacement;
- least-privilege installation;
- no secrets in logs;
- no untrusted binaries bundled into ISB releases.

ISB may download third-party driver packages for the user, but must not redistribute proprietary NVIDIA/Google packages without a legal basis.

The local cache is a user-side cache, not an ISB-owned driver mirror.

---

# 16. Implementation order

1. Audit existing build/release code.
2. Audit existing driver code and models.
3. Consolidate driver metadata.
4. Implement provider/catalog contracts.
5. Implement NVIDIA provider.
6. Implement Google provider.
7. Implement local cache.
8. Implement safe downloader.
9. Implement checksum/signature verification.
10. Integrate DriverRepository with Hub.
11. Add CLI.
12. Add Control Center Driver page.
13. Add automatic download.
14. Add explicit installation backend only if technically and legally justified.
15. Implement Release Builder.
16. Implement package matrix.
17. Implement manifest/checksum generation.
18. Implement GitHub Actions release workflow.
19. Add release verification.
20. Synchronize documentation.

---

# 17. Acceptance criteria

The system is considered complete only when:

- the user can select a driver provider;
- branches remain distinguishable;
- versions can be listed/searched;
- each version has source/provenance metadata;
- official/source links can be opened;
- downloads can be initiated from the application;
- automatic download validates the artifact;
- SHA-256 is checked when available;
- artifacts are cached safely;
- V100 compatibility is evidence-based;
- download and installation are separate;
- the application never silently replaces the NVIDIA/Google base driver;
- Release Builder can build supported ISB targets;
- tests gate releases;
- release artifacts have deterministic names;
- SHA256SUMS is generated;
- manifest is generated;
- GitHub Releases can be generated from version tags;
- failed build/test/package stages prevent publication;
- proprietary third-party driver binaries are not silently redistributed.

---

# 18. User workflow

The intended everyday workflow for the owner of the project is:

### Build a local release

```bash
python tools/release/release.py
```

### Build a specific platform

```bash
python tools/release/release.py --target windows
```

### Build all supported platforms

```bash
python tools/release/release.py --all
```

### Create a published release

```bash
git tag v0.1.0
git push origin v0.1.0
```

GitHub Actions then performs the release pipeline.

### Select a driver

In Control Center:

```text
DRIVERS
 -> Provider
 -> Branch
 -> Version
 -> Inspect
 -> Download
 -> Verify
```

Installation is shown only if the corresponding installation backend exists.

---

# 19. Relationship to the architecture

Driver management and release engineering belong to the lifecycle layer.

They must not become part of:

- GPU HAL;
- provider implementations;
- telemetry logic;
- tuning logic;
- GUI-only code.

The intended boundary is:

```text
Qt Control Center / CLI
          |
          v
         Hub
      /       \
DriverRepo   ReleaseBuilder
   |              |
Providers      CMake/CI
   |
NVIDIA / Google / Cache
```

This preserves the existing principle that the GUI is a frontend and the Hub is the user-space control entry point.
