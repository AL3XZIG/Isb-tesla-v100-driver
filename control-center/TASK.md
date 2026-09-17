# Control Center — V100 Hub GUI

## Goal
Build a lightweight optional GUI frontend for the ISB V100 Hub. It should provide an NVIDIA App-like user experience for V100 without duplicating the Hub's business logic.

## Views
- **Home** — GPU identity, driver/API state, health, active profile and detected issues.
- **Performance** — telemetry, clocks/power/ECC and supported performance controls.
- **Games** — discovered games, compatibility state and per-game profiles.
- **Tools** — OptiScaler manager, benchmarks, diagnostics and reports.
- **Optimize V100** — capability-aware plan → approval → apply → verify workflow.

## Must implement
- Use the same Hub contracts as the CLI.
- Display hardware, base-driver and ISB capabilities separately.
- Display `UNKNOWN` explicitly instead of hiding unsupported/undetected features.
- Show requested/applied/verified state for mutable operations.
- Surface restart/reset/privilege requirements.
- Provide rollback for reversible operations.
- Generate the same structured audit/provenance records as CLI actions.
- Keep the application lightweight; prefer native C++ with Qt or ImGui/SDL over Electron-scale runtime.
- Core operation must not require an always-on daemon.

## Non-goals
- Reimplementing the NVIDIA driver UI wholesale.
- Implementing GPU management logic inside the GUI.
- Making the GUI mandatory for ISB operation.
- Silent global game modification.

## Acceptance
- All critical workflows remain usable from CLI.
- GUI actions are thin calls into Hub contracts.
- A user can inspect and optimize a supported V100 without understanding internal FixEngine/provider details.
- Failed reversible changes expose a clear rollback path.
