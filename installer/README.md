# ISB Installer and Qualification

Linux: ./installer/install.sh

The Linux installer installs build prerequisites on Debian/Ubuntu when apt-get is available, builds ISB, runs CTest, and installs isb to the user-local bin directory. It never installs or replaces the NVIDIA driver.

Windows: powershell -ExecutionPolicy Bypass -File installer/windows/install.ps1

The Windows bootstrap expects CMake and a C++17 toolchain and does not install NVIDIA drivers.

Qualification: tools/qualification/run.sh

The runner stores raw output under reports/runs/<UTC timestamp>/logs/ and writes machine-readable environment, capabilities, telemetry, diagnosis, verification, benchmark, summary, and manifest files. A failing probe remains visible in the report. Mock output is never treated as real V100 evidence.


## Qualification result semantics

The qualification runner distinguishes execution from hardware evidence:

- `PASS` means the command succeeded, the run is using a real provider, and the result does not contain an explicit unverified/synthetic state.
- `UNKNOWN` means the command ran but hardware evidence is unavailable, synthetic, or not verified. Unknown results are never promoted to PASS.
- `FAIL` means the qualification command itself failed.

Exit codes are `0` for a fully real passing qualification, `1` when at least one case fails, and `2` when evidence is unavailable or remains unknown. The report records `qualification_mode` and the PASS/FAIL/UNKNOWN counts.
