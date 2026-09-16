# HAL — Technical Task

HAL is retained as a low-level abstraction for future hardware-backed paths and diagnostic primitives.

## Must implement later
- Provider-neutral hardware access contracts.
- No direct userspace BAR/MMIO access in production.
- Fake backend for tests.
- Clean boundary to OSAL/KMD when real hardware access is required.

## Current priority
HAL is foundation/research infrastructure, not the primary Driver Fixer MVP path.
