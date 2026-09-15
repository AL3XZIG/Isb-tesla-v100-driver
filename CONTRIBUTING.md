# Contributing to ISB V100

## Development workflow

1. Discuss architecture and research assumptions before implementation when a change crosses subsystem boundaries.
2. Implement focused changes on a branch.
3. Add tests and reproducibility information where applicable.
4. Submit a pull request with the motivation, scope, validation, and known limitations.
5. Keep stable, experimental, and research code explicitly separated.

## Evidence policy

Do not present unverified hardware behavior, benchmark numbers, compatibility claims, or driver behavior as established fact.

Use these labels when appropriate:

- VERIFIED
- ASSUMED
- VERIFY BEFORE FREEZE
- BLOCKED

## Benchmark policy

A benchmark report should identify at minimum:

- exact V100 variant;
- OS/kernel;
- driver/runtime versions;
- clocks/power/ECC configuration;
- workload and input dimensions;
- warm-up policy;
- measurement mode;
- number of repetitions;
- random seed where relevant;
- profiler/tooling state;
- raw or reproducible result provenance.

## Baseline fairness

ISB strategies must be compared against appropriate existing baselines. A baseline must not be intentionally weakened to make ISB appear faster.

## Proprietary components

Do not commit proprietary NVIDIA binaries, credentials, private keys, or downloaded model weights unless their redistribution rights are explicitly established.
