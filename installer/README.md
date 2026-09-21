# ISB Linux Installer and Qualification

## Installer

Run:

```bash
./installer/install.sh
```

The Linux installer:

- checks prerequisites;
- builds ISB;
- runs CTest;
- installs `isb`;
- initializes Linux configuration;
- never silently installs or replaces the NVIDIA base driver.

Supported initial target: **Linux x86-64**.

## Render configuration

The intended V100 topology is:

- iGPU/secondary dGPU — display;
- Tesla V100 — render + compute.

Render routing is application-scoped where possible. ISB must verify the actual Vulkan/OpenGL device rather than assuming that configuration succeeded.

## Qualification

Run:

```bash
tools/qualification/run.sh
```

The runner stores raw output under `reports/runs/<UTC timestamp>/logs/` and writes machine-readable environment, capabilities, telemetry, diagnosis, verification, benchmark, summary and manifest files.

A failing probe remains visible in the report. Mock output is never treated as real V100 evidence.

## Qualification result semantics

- `PASS` means the command succeeded, the run uses a real provider, and the result has no explicit unverified/synthetic state.
- `UNKNOWN` means the command ran but hardware evidence is unavailable, synthetic or unverified. Unknown is never promoted to PASS.
- `FAIL` means the qualification command itself failed.

Exit codes:

- `0` — fully real passing qualification;
- `1` — at least one case failed;
- `2` — evidence is unavailable or remains unknown.

The report records `qualification_mode` and PASS/FAIL/UNKNOWN counts.
