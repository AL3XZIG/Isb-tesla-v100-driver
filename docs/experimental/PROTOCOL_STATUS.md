# Experimental Protocol Status

## Workload-Aware Resource Utilization

Current status: **Draft / awaiting independent adversarial review**.

The protocol evaluates whether a GV100-aware execution policy can improve end-to-end execution against strong static and existing-stack baselines while accounting for decision, synchronization, transfer, and adaptation overhead.

### Required controls

- Exact hardware variant: V100 SXM2 or V100 PCIe.
- ECC state.
- Clock and power policy.
- Thermal state.
- OS, kernel, driver, CUDA/runtime, compiler and library versions.
- Warm-up policy.
- Measurement noise floor.
- Explicit production versus instrumented versus deep-profiling modes.
- Reproducible seeds and workload manifests.

### Baseline hierarchy

- **B0:** naive/reference implementation.
- **B1:** best practical existing stack.
- **B2:** static ISB strategy.
- **B3:** dynamic ISB strategy.

### Oracle hierarchy

- **O1:** offline oracle.
- **O2:** clairvoyant oracle.
- **O3:** realistic online oracle.

The O3 decision budget is provisional and must be measured rather than treated as a hardware constant.

### Main end-to-end quantity

`T_total = T_decision + T_execution + T_synchronization + T_transfer + T_adaptation`

Decision overhead is a first-class metric.

### Current research boundary

No result has been established by this repository yet. Numerical performance targets and compatibility claims remain experimental until reproduced on the exact target configuration.

Full protocol material will be frozen only after the DeepSeek feasibility review and Claude adversarial review have been reconciled.
