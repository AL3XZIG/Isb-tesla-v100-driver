# ISB V100 Hub Documentation

## Start here

For AI agents and new contributors, read in this order:

1. **MASTER_SPEC.md** — consolidated product and technical specification from the project's accumulated requirements.
2. **AGENT_GUIDE.md** — operational rules and architectural boundaries.
3. **PRODUCT_REQUIREMENTS.md** — user-facing product requirements.
4. **CURRENT_IMPLEMENTATION.md** — conservative implementation-state snapshot.
5. **AGENT_WORKFLOW.md** — how to modify the repository safely.
6. **DRIVER_AND_RELEASE_PIPELINE.md** — driver lifecycle and release engineering.

MASTER_SPEC.md is the best single document to give to a new coding agent when the agent needs the complete project context. It intentionally describes the target architecture and requirements; it does not override source code or CI when describing implementation status.

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

For product intent, prefer:

1. MASTER_SPEC.md;
2. PRODUCT_REQUIREMENTS.md;
3. README.md.

If documents disagree, inspect the source and update the documentation rather than guessing.
