# ISB V100 Hub — Driver Lifecycle, URL Parsing and Release Engineering

This document defines the planned and required behavior for the ISB driver-management and release-distribution subsystems.

It complements:

- `PRODUCT_REQUIREMENTS.md` — product intent;
- `CURRENT_IMPLEMENTATION.md` — verified implementation state;
- `IMPLEMENTATION_PLAN.md` — execution order;
- `ARCHITECTURE.md` — ownership and boundaries.

## 1. Scope

ISB is not a replacement NVIDIA driver.

The installed NVIDIA/Google base driver remains underneath ISB:

`ISB Control Center / Hub -> installed NVIDIA or Google driver -> V100`

The driver subsystem exists to:

- discover driver releases;
- parse release metadata and download URLs;
- let users select a driver version;
- download official/external driver artifacts;
- verify downloaded artifacts;
- maintain a local cache;
- expose driver lifecycle information to the Hub and Control Center;
- provide safe installation/configuration workflows when explicitly supported;
- build and publish ISB's own software releases.

Downloading an artifact must never imply that ISB owns or replaces that driver's implementation.

## 2. Driver sources

The initial source model includes:

### NVIDIA

Support discovery of relevant NVIDIA driver families, including where applicable:

- Tesla/Data Center drivers;
- NVIDIA vGPU/vWS/GRID branches;
- other NVIDIA branches only when they are actually relevant to the detected V100 environment.

NVIDIA Data Center documentation publishes driver release information and exposes release metadata that can be consumed by automation. NVIDIA release pages also expose version, date, OS, CUDA toolkit version, file size and supported products. Tesla V100 is explicitly listed on documented Data Center releases. citeturn0search1turn0search3turn0search4

### Google

Support Google-provided NVIDIA GPU driver sources where the source is relevant to the target OS/GPU/environment.

Google-hosted drivers must remain distinguishable from NVIDIA-published drivers. ISB must preserve source provenance and must not merge different vendor branches into one ambiguous version namespace.

### Source identity

Every driver artifact must retain:

- vendor;
- source/provider;
- driver family;
- branch;
- version;
- release date when known;
- target OS;
- architecture;
- package type;
- GPU family;
- download URL;
- source/release page;
- checksum when published;
- signature information when available;
- discovery timestamp;
- parser/provider version.

If a field cannot be established reliably, use `UNKNOWN`.

## 3. Driver metadata model

The canonical model should be provider-neutral.

Conceptually:

```cpp
struct DriverArtifact {
    Vendor vendor;
    DriverFamily family;
    std::string branch;
    std::string version;
    OperatingSystem os;
    Architecture architecture;
    PackageType package_type;
    GpuFamily gpu_family;

    std::string download_url;
    std::string source_url;

    std::optional<std::string> release_date;
    std::optional<std::string> sha256;
    std::optional<std::string> signature;
    std::optional<std::uint64_t> size_bytes;

    Provenance provenance;
};
```

The exact repository model must be reused if an equivalent already exists.

Do not create duplicate driver models.

## 4. URL parser

The URL parser is a separate component from the downloader.

Its job is to normalize:

- direct artifact URLs;
- release-page URLs;
- NVIDIA driver result pages;
- Google driver download pages;
- redirect URLs;
- URLs containing query parameters;
- URLs whose metadata must be obtained from an HTML page.

Pipeline:

```
URL
  |
  v
Source detection
  |
  v
Provider-specific parser
  |
  v
Normalized DriverArtifact
  |
  v
Validation
  |
  v
DriverRepository
```

### Parser requirements

The parser must:

1. identify the source/vendor;
2. identify whether the URL is a release page or direct artifact;
3. normalize redirects;
4. extract version where possible;
5. extract OS/architecture where possible;
6. identify package format;
7. extract GPU/product family where published;
8. preserve the original URL;
9. preserve the final download URL;
10. retain provenance;
11. report ambiguous fields as `UNKNOWN`;
12. never invent metadata from a filename alone when the interpretation is ambiguous.

### Parser error handling

Use the project's existing `Result`, `Status`, `ErrorCode` and provenance model.

Distinguish:

- malformed URL;
- unsupported source;
- page not found;
- parser mismatch;
- missing metadata;
- redirect failure;
- network failure;
- checksum unavailable;
- checksum mismatch.

A parser that cannot understand a new NVIDIA URL must fail explicitly rather than silently returning an incorrect driver record.

## 5. Driver repository

Create one logical repository interface:

```
DriverRepository
  |
  +-- NvidiaDriverProvider
  +-- GoogleDriverProvider
  +-- LocalCacheProvider
```

The provider-specific implementations own discovery/parsing rules.

The Hub owns orchestration.

The GUI and CLI do not scrape vendor websites directly.

Suggested operations:

```
list()
search(query)
find(version)
findCompatible(gpu, os)
latest(filters)
inspect(artifact)
download(artifact)
verify(artifact)
cacheList()
removeFromCache(artifact)
```

Compatibility search must be evidence-based.

For example, a V100 query should consider:

- exact GPU family;
- V100 variant where known;
- OS;
- architecture;
- driver family;
- vendor;
- installed driver;
- required graphics/compute API;
- known project compatibility records.

## 6. Download pipeline

The safe download lifecycle is:

```
discover
  -> select
  -> show metadata
  -> explicit user confirmation
  -> download temporary file
  -> size validation
  -> checksum
  -> signature verification when available
  -> atomic move into cache
  -> record provenance
```

Rules:

- HTTPS only unless an explicit future exception is documented;
- support redirects;
- use temporary files;
- never treat a partial download as a valid artifact;
- delete corrupted partial files;
- verify SHA-256 when a trusted checksum is available;
- preserve the original source URL;
- do not execute a downloaded installer automatically;
- do not silently install or replace the active NVIDIA driver.

Download and installation are separate operations.

## 7. Local driver cache

The cache should be content-addressable or otherwise collision-safe.

Each cached artifact should have:

- artifact identity;
- local path;
- source URL;
- source vendor;
- version;
- SHA-256;
- file size;
- download timestamp;
- verification status;
- provenance;
- optional expiration policy.

Example logical layout:

```
cache/drivers/
  nvidia/
    <version>/
  google/
    <version>/
```

Do not assume a version number alone uniquely identifies an artifact. OS, architecture, vendor and package type are part of identity.

## 8. Driver installation boundary

ISB may eventually expose explicit installation/configuration workflows, but installation must remain separate from discovery/download.

Required sequence:

```
DETECT CURRENT DRIVER
  -> SHOW CANDIDATE
  -> CHECK COMPATIBILITY
  -> SHOW RISKS
  -> BACKUP / RECOVERY INFORMATION
  -> USER APPROVAL
  -> INSTALL
  -> REBOOT/RESTART IF REQUIRED
  -> RE-DETECT
  -> VERIFY
  -> REPORT
```

No automatic driver replacement during ISB installation.

The normal ISB installer installs ISB itself; it does not silently replace the system NVIDIA/Google driver.

## 9. Control Center integration

The Control Center should expose a Driver section backed exclusively by the Hub.

Possible views:

### Installed

- vendor;
- branch;
- version;
- package;
- OS;
- CUDA/runtime information;
- installation state;
- provenance.

### Available

- vendor;
- version;
- branch;
- release date;
- target OS;
- supported GPU family;
- package size;
- verification information.

### Actions

- Inspect;
- Download;
- Verify;
- Open release/source page;
- Install, only when an explicit supported installation backend exists;
- Remove cached artifact.

Do not expose an Install button when the installation backend is not implemented.

## 10. CLI

The CLI should mirror Hub semantics.

Suggested commands:

```
isb drivers list
isb drivers search <query>
isb drivers info <version>
isb drivers compatible
isb drivers download <artifact>
isb drivers verify <file>
isb drivers cache list
isb drivers cache remove <artifact>
```

Commands must return deterministic machine-readable JSON when the existing CLI supports JSON output.

## 11. Release engineering

ISB itself needs a reproducible release pipeline.

The desired lifecycle is:

```
tag vX.Y.Z
   |
   v
clean checkout
   |
   v
configure
   |
   v
build
   |
   v
unit/integration tests
   |
   v
package
   |
   v
checksum
   |
   v
manifest
   |
   v
GitHub Release
   |
   v
upload artifacts
```

GitHub Actions supports build/test artifacts and sharing them between jobs; release automation should use those mechanisms rather than committing generated binaries into `main`. citeturn0search0turn0search12

GitHub's documented release workflow also supports semantic version tags and automated release management. citeturn0search7turn0search15

## 12. Release targets

Initial release targets:

### Windows x64

- portable `.zip`;
- installer `.exe`.

### Linux x86_64

- `.deb`;
- portable `.tar.gz`.

Future targets may include:

- additional Linux distributions/packages;
- other architectures;
- symbols/debug packages;
- development SDK/package.

Do not claim an artifact is supported until its build and packaging job succeeds.

## 13. Package naming

Use deterministic names:

```
isb-<version>-<platform>-<arch>.<extension>
```

Examples:

```
isb-0.1.0-windows-x64.zip
isb-0.1.0-windows-x64.exe
isb-0.1.0-linux-x64.deb
isb-0.1.0-linux-x64.tar.gz
```

The exact version comes from the release tag or canonical project version source.

## 14. Release manifest

Every release should contain a machine-readable manifest.

Example:

```json
{
  "schema_version": 1,
  "version": "0.1.0",
  "commit": "<full-sha>",
  "artifacts": [
    {
      "name": "isb-0.1.0-windows-x64.zip",
      "platform": "windows",
      "architecture": "x86_64",
      "format": "zip",
      "size_bytes": 0,
      "sha256": "<hash>"
    }
  ]
}
```

Also generate:

```
SHA256SUMS
```

The manifest must not contain unverifiable claims.

## 15. Release notes

Release notes should contain:

- version;
- release date;
- source commit;
- major changes;
- fixed issues;
- supported platforms;
- known limitations;
- artifact list;
- verification information.

Generated release notes must not claim a feature is stable if the implementation has only mock or experimental evidence.

## 16. CI release gate

A release must not be published when:

- configure fails;
- build fails;
- required tests fail;
- package generation fails;
- checksum generation fails;
- manifest generation fails.

Recommended stages:

```
build-matrix
     |
     +--> test
     |
     +--> package-windows
     |
     +--> package-linux
     |
     v
collect-artifacts
     |
     v
generate-manifest
     |
     v
publish-release
```

The publish job must depend on successful build/test/package jobs.

## 17. Reproducibility and provenance

Every release artifact must be traceable to:

- Git commit;
- release tag;
- build workflow/run;
- platform;
- architecture;
- compiler/toolchain where practical;
- packaging configuration;
- dependency versions where practical.

Generated binaries should never be presented as if they were source-controlled files.

## 18. Security

Release automation must:

- use least-privilege GitHub permissions;
- avoid arbitrary execution of downloaded driver artifacts;
- verify third-party artifacts before installation;
- keep secrets out of logs;
- avoid accepting untrusted pull-request code into privileged release jobs;
- pin third-party GitHub Actions where the project's security policy requires it;
- preserve checksums and provenance.

Creating GitHub releases requires appropriate repository permissions; workflow changes and release creation also have specific token/permission requirements documented by GitHub. citeturn0search16

## 19. Legal/provenance boundary

ISB may discover and download third-party driver packages from their official sources, but it must not imply ownership of those packages.

Do not:

- redistribute proprietary NVIDIA/Google drivers without a legal basis;
- silently bundle proprietary driver binaries into ISB releases;
- strip vendor license/provenance information;
- mirror third-party packages into the ISB repository merely for convenience.

The cache is a user-side artifact cache, not an ISB-owned driver distribution repository.

## 20. Implementation order

Driver lifecycle work should be implemented in this order:

1. audit existing `drivers/` and installer code;
2. consolidate driver metadata model;
3. implement URL/source parser contracts;
4. implement NVIDIA provider;
5. implement Google provider;
6. implement local cache;
7. implement safe downloader;
8. implement checksum/signature verification;
9. integrate DriverRepository with Hub;
10. expose CLI;
11. expose Control Center;
12. add explicit installation backend only if technically and legally justified;
13. add release package definitions;
14. add CI package matrix;
15. add manifest/checksum generation;
16. add GitHub Release publishing;
17. add release verification job;
18. update documentation and support matrix.

## 21. Acceptance criteria

Driver lifecycle is complete when:

- vendor/source identity is preserved;
- NVIDIA and Google providers are distinct;
- URLs can be parsed into normalized metadata;
- parser failures are explicit;
- unknown metadata remains UNKNOWN;
- driver versions can be searched;
- V100 compatibility filtering is evidence-based;
- downloads use a temporary-file/verification pipeline;
- checksums are verified when available;
- cache entries are provenance-aware;
- GUI/CLI access drivers through Hub;
- download does not imply installation;
- ISB does not silently replace the base NVIDIA/Google driver;
- Windows ZIP/EXE artifacts build successfully;
- Linux DEB/TAR.GZ artifacts build successfully;
- SHA256SUMS is generated;
- release manifest is generated;
- release artifacts are traceable to a commit/tag;
- failed build/test/package gates block publication;
- GitHub Releases are produced automatically from version tags;
- proprietary third-party drivers are not silently redistributed;
- documentation accurately reflects verified implementation state.
