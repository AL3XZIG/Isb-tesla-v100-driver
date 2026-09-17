# Performance — V100 Telemetry and Tuning

## Goal
Expose V100-aware monitoring and supported performance controls through the common Hub transaction model.

## Must implement
- Utilization, temperature, power, clocks, ECC and process telemetry where available.
- PCIe link/traffic and NVLink state where exposed.
- Thermal/power throttling detection.
- Persistence mode, application clocks, power limit and compute mode only when the base management API supports them and permissions allow.
- Profiles: Balanced, Gaming, Compute, AI/Tensor, Maximum Performance, Low Power, Custom.
- `requested -> applied -> verified` state for every mutable setting.
- Explicit temporary/persistent/reset/restart metadata.

## Safety
Never report an unsupported or rejected setting as applied. Do not silently change hardware limits.

## Acceptance
All read/write operations have capability checks, structured results, logs and verification coverage.
