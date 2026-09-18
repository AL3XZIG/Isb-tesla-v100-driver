# ISB OptiScaler integration

ISB treats OptiScaler as an **optional, managed external component**.

## Offline distribution

A release may bundle an unmodified official OptiScaler release under:

    components/optiscaler/

The component package must contain:

- the upstream OptiScaler files;
- `component.version` containing the bundled upstream version;
- the upstream `LICENSE`;
- `OptiScaler.ini`.

The ISB integration only detects and manages the component boundary. It does not replace or reimplement OptiScaler.

## Provenance and credit

Upstream project:

https://github.com/optiscaler/OptiScaler

OptiScaler is GPL-3.0. The ISB distribution must preserve the upstream license and corresponding source/provenance obligations when conveying the component.

The official OptiScaler v0.9.4 release credits contributors including `cdozdil`, `FakeMichau`, and `TheRazerMD`. These credits are upstream credits and do not imply that those developers endorse ISB.

OptiScaler's own release notes state that the project does not have an official manager application. ISB must therefore identify itself as an independent ISB integration, not as an official OptiScaler manager.

## Important boundary

OptiScaler being absent must never make ISB fail to start.

Expected states:

- `NotInstalled` — ISB works normally; OptiScaler-specific features are unavailable.
- `Installed` — the managed component is available for compatibility checks.
- `Invalid` — a package is present but incomplete/corrupt according to the ISB package markers.

Actual game installation/application is a separate operation and must remain explicit, reversible where possible, and capability-aware.
