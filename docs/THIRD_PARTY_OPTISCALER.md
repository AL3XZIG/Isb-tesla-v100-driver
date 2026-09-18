# OptiScaler third-party component

ISB can distribute OptiScaler as an optional bundled component.

## Licensing

The official OptiScaler repository is GPL-3.0. ISB does not relicense OptiScaler as proprietary software.

When an ISB release conveys an OptiScaler component, the release must preserve the applicable GPL license text and provide the corresponding source/provenance required by the license. The ISB application remains a separate project; this integration is a component-management boundary rather than a copy of OptiScaler source.

## Upstream

- Project: OptiScaler
- Repository: https://github.com/optiscaler/OptiScaler
- Current upstream stable release verified during this change: v0.9.4
- Official release credits: cdozdil, FakeMichau, TheRazerMD, plus other contributors.

ISB is **not affiliated with or an official manager for OptiScaler**.

## Bundling policy

For an offline ISB release:

1. Obtain the OptiScaler artifact from an official upstream source.
2. Keep the upstream files unmodified.
3. Preserve its license and provenance.
4. Record the exact upstream version in `component.version`.
5. Record the exact artifact checksum in the release manifest.
6. Ship the corresponding source/provenance package required for GPL compliance.
7. Never silently download a replacement component at runtime.

If no bundled component is present, ISB remains fully usable and reports OptiScaler as `NotInstalled`.
