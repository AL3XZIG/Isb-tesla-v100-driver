# Third-Party Upstream Components

ISB may integrate or reuse open-source upstream projects. This file records the projects currently evaluated for the graphics/driver stack.

| Project | Role in ISB | License status | Current treatment |
|---|---|---|---|
| Mesa 3D | Graphics API / layered-driver architecture and possible future integration | Multi-license; core Mesa and Gallium are MIT, individual components vary | Architecture reference; no source copied by this change |
| NVIDIA Open GPU Kernel Modules | OS/kernel architecture reference | MIT for many files; linked Linux kernel modules are MIT/GPLv2; hardware scope is Turing+ | Architecture reference only; not a V100 backend |
| OptiScaler | External upscaling/frame-generation compatibility backend | GPL-3.0 | External component; do not copy into ISB core under this change |
| DLSS-Enabler | Application interception/compatibility architecture; possible future source reuse | MIT | Architecture reference; no source copied by this change |

## Source reuse procedure

Before importing source code from an upstream project, record:

1. upstream repository URL;
2. exact commit/tag/version;
3. source file path(s);
4. SPDX/license identifier;
5. copyright notices;
6. required license texts/notices;
7. ISB modifications;
8. whether the imported component changes the licensing obligations of the containing artifact.

Do not infer a repository-wide license from a single file when the upstream project is multi-license.

## Important boundaries

- Proprietary NVIDIA binaries are not redistributed by ISB unless separate redistribution rights are established.
- V100 support must not be inferred from NVIDIA's current open kernel module repository, whose documented supported GPU scope is Turing and later.
- GPL components such as OptiScaler remain isolated external dependencies unless the affected ISB distribution boundary is intentionally made license-compatible.
