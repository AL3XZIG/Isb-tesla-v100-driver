# ISB V100 Hub Documentation

## Start here

For AI agents and new contributors, read in this order:

1. **MASTER_SPEC.md** — consolidated product and technical specification.
2. **MASTER_TZ_ADDENDUM.md** — verified addendum containing the accumulated requirements for graphics enhancement, OptiScaler, render path, Driver Manager, URL parser, downloader/cache, Release Builder and CI publication.
3. **AGENT_GUIDE.md** — operational rules and architectural boundaries.
4. **PRODUCT_REQUIREMENTS.md** — user-facing product requirements.
5. **CURRENT_IMPLEMENTATION.md** — conservative implementation-state snapshot.
6. **AGENT_WORKFLOW.md** — how to modify the repository safely.
7. **DRIVER_AND_RELEASE_PIPELINE.md** — detailed driver lifecycle and release engineering.

MASTER_SPEC.md plus MASTER_TZ_ADDENDUM.md are the preferred project-context pair for a new coding agent. They describe target architecture and requirements; they do not override source code or CI when describing implementation status.

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

## Source-of-truth rule

For implementation state, prefer:

1. source code;
2. CI/test evidence;
3. CURRENT_IMPLEMENTATION.md;
4. TASKS.md / IMPLEMENTATION_PLAN.md;
5. historical documents.

For product intent, prefer:

1. MASTER_SPEC.md;
2. MASTER_TZ_ADDENDUM.md;
3. PRODUCT_REQUIREMENTS.md;
4. README.md.

If documents disagree, inspect the source and update the documentation rather than guessing.

The existence of a specification never proves that the corresponding feature is implemented.
