# ISB Driver Patch Research

Research-only area for investigating software restrictions in the installed NVIDIA V100 driver stack.

Reference methodology:
- reproducible patch manifests;
- exact input/version identification;
- expected-byte/signature guards;
- source-to-binary verification;
- post-change verification;
- rollback;
- fail-closed behavior.

Primary external reference:
https://github.com/SupraGSX/Forceware-382.69

This directory must remain isolated from the stable Hub runtime. Do not add NVIDIA proprietary binaries, firmware, or vendor-derived source here.

Initial implementation should be **scanner/verifier/dry-run only**. Actual mutation requires a separately reviewed implementation, explicit approval, backups, rollback and regression tests.
