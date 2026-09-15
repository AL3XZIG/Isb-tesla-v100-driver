# Driver Fixer

The `fixer` module is the main MVP implementation area.

## Responsibilities

- normalize the current driver/API environment;
- evaluate deterministic compatibility rules;
- produce explainable workaround plans;
- apply only explicitly supported, reversible actions;
- verify the result;
- record evidence for regression testing.

## Non-responsibilities

The Fix Engine does not implement a kernel driver, WDDM KMD, DRM GPU scheduler, GPUVM, or proprietary NVIDIA functionality.

## Pipeline

```text
Probe -> Fingerprint -> Diagnose -> Match Rules -> Plan -> Apply -> Verify -> Report
```

The first implementation should keep `Match Rules` pure and side-effect-free. Applying a workaround belongs to a separate executor so that rules can be unit-tested without a GPU or administrator privileges.

## Action classes

Initial actions should be declarative:

- environment/configuration change;
- API feature selection;
- application profile selection;
- external-component configuration;
- enable/disable a validated ISB shim;
- diagnostic-only recommendation.

Actions that mutate the installed driver package itself are out of the MVP.
