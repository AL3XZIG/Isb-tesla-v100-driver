# ISB Installer and Qualification

Linux: ./installer/install.sh

The Linux installer installs build prerequisites on Debian/Ubuntu when apt-get is available, builds ISB, runs CTest, and installs isb to the user-local bin directory. It never installs or replaces the NVIDIA driver.

Windows: powershell -ExecutionPolicy Bypass -File installer/windows/install.ps1

The Windows bootstrap expects CMake and a C++17 toolchain and does not install NVIDIA drivers.

Qualification: tools/qualification/run.sh

The runner stores raw output under reports/runs/<UTC timestamp>/logs/ and writes machine-readable environment, capabilities, telemetry, diagnosis, verification, benchmark, summary, and manifest files. A failing probe remains visible in the report. Mock output is never treated as real V100 evidence.
