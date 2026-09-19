# ISB V100 Hub Documentation

## Start here

For AI agents and new contributors:

1. AGENT_GUIDE.md — project intent, rules and architectural boundaries.
2. PRODUCT_REQUIREMENTS.md — what the product should become.
3. CURRENT_IMPLEMENTATION.md — conservative current-state snapshot.
4. AGENT_WORKFLOW.md — how to modify the repository safely.
5. DRIVER_AND_RELEASE_PIPELINE.md — driver discovery/download, URL parsing, cache, verification and ISB release engineering.

## Architecture and roadmap

- ARCHITECTURE.md
- IMPLEMENTATION_PLAN.md
- TASKS.md
- COMPONENT_TASKS.md
- ROADMAP_STATUS.md
- SUPPORT_MATRIX.md
- RENDER_PATH.md
- UPSTREAM_INTEGRATION.md

## Build and verification

- build.md
- FIRST_BUILD.md
- verification-tools-v1.md

## Driver and release engineering

DRIVER_AND_RELEASE_PIPELINE.md is the canonical specification for:

- NVIDIA and Google driver source providers;
- driver version discovery;
- driver URL parsing;
- normalized driver metadata;
- safe driver downloads;
- checksum/signature verification;
- local driver cache;
- Hub/CLI/Control Center driver integration;
- explicit installation boundaries;
- Windows .zip/.exe packaging;
- Linux .deb/.tar.gz packaging;
- release manifests;
- SHA256SUMS;
- GitHub Actions build/test/package/release flow;
- release provenance and security gates.

The specification does not imply that every listed feature is already implemented. Current source code and CI remain authoritative for implementation status.

## Historical material

PR-specific audit documents are historical evidence. Current source, CI and current architecture documents take precedence.

## Conflict rule

For implementation state, prefer:

1. source code;
2. CI/test evidence;
3. CURRENT_IMPLEMENTATION.md;
4. TASKS.md / IMPLEMENTATION_PLAN.md;
5. historical documents.

For product intent, prefer PRODUCT_REQUIREMENTS.md and README.md.

If documents disagree, inspect the source and update the documentation rather than guessing.
