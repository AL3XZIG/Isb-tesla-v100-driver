# Driver issue database

Each confirmed issue must have:

- a stable issue ID;
- exact GPU variant;
- OS/platform;
- base-driver identity/version;
- API/runtime;
- reproducible symptom;
- trigger conditions;
- workaround action;
- verification procedure/result;
- provenance/evidence;
- rollback information.

Do not add a workaround solely because it sounds plausible. An issue can remain `observed` or `hypothesis` until independently reproduced.

## Status vocabulary

- `hypothesis` — suspected but not reproduced.
- `observed` — reproduced locally, evidence captured.
- `confirmed` — independently reproducible or strongly validated.
- `fixed` — workaround verified on the affected combination.
- `regressed` — a previously working workaround no longer passes verification.
