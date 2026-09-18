#!/usr/bin/env bash
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT}/build"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
REPORT_DIR="${ROOT}/reports/runs/${STAMP}"
mkdir -p "${REPORT_DIR}/logs"
PASS=0; FAIL=0
run_case() {
  local name="$1"; shift; local log="${REPORT_DIR}/logs/${name}.log"
  echo "=== ${name} ===" | tee "${log}"
  if "$@" >>"${log}" 2>&1; then
    printf '%s|PASS|%s\n' "${name}" "${log}" >>"${REPORT_DIR}/results.tsv"; PASS=$((PASS+1))
  else
    printf '%s|FAIL|%s\n' "${name}" "${log}" >>"${REPORT_DIR}/results.tsv"; FAIL=$((FAIL+1))
  fi
}
printf 'ISB V100 qualification run\nstarted_utc=%s\nhost=%s\nkernel=%s\n' "${STAMP}" "$(hostname)" "$(uname -srmo 2>/dev/null || true)" > "${REPORT_DIR}/run.txt"
if [[ ! -x "${BUILD_DIR}/cli/isb" ]]; then echo "isb binary not found; run installer/install.sh first." | tee "${REPORT_DIR}/logs/installer.log"; exit 2; fi
run_case status "${BUILD_DIR}/cli/isb" --json status
run_case capabilities "${BUILD_DIR}/cli/isb" --json capabilities
run_case telemetry "${BUILD_DIR}/cli/isb" --json telemetry
run_case diagnose "${BUILD_DIR}/cli/isb" --json diagnose
run_case verify "${BUILD_DIR}/cli/isb" --json verify
run_case benchmark "${BUILD_DIR}/cli/isb" --json benchmark
"${BUILD_DIR}/cli/isb" --json capabilities > "${REPORT_DIR}/capabilities.json" 2>"${REPORT_DIR}/logs/capabilities.stderr" || true
"${BUILD_DIR}/cli/isb" --json status > "${REPORT_DIR}/environment.json" 2>"${REPORT_DIR}/logs/status.stderr" || true
"${BUILD_DIR}/cli/isb" --json telemetry > "${REPORT_DIR}/telemetry.json" 2>"${REPORT_DIR}/logs/telemetry.stderr" || true
"${BUILD_DIR}/cli/isb" --json diagnose > "${REPORT_DIR}/diagnose.json" 2>"${REPORT_DIR}/logs/diagnose.stderr" || true
"${BUILD_DIR}/cli/isb" --json verify > "${REPORT_DIR}/verification.json" 2>"${REPORT_DIR}/logs/verify.stderr" || true
"${BUILD_DIR}/cli/isb" --json benchmark > "${REPORT_DIR}/benchmark.json" 2>"${REPORT_DIR}/logs/benchmark.stderr" || true
cat > "${REPORT_DIR}/summary.json" <<EOF
{"schema_version":"1","run_id":"${STAMP}","results":{"pass":${PASS},"fail":${FAIL}},"artifacts":["environment.json","capabilities.json","telemetry.json","diagnose.json","verification.json","benchmark.json","results.tsv"]}
EOF
cat > "${REPORT_DIR}/manifest.json" <<EOF
{"schema_version":"1","report_type":"qualification","run_id":"${STAMP}","synthetic":false,"source":"isb qualification runner","summary":"summary.json"}
EOF
echo "[ISB] Qualification report: ${REPORT_DIR}"; echo "[ISB] PASS=${PASS} FAIL=${FAIL}"
[[ "${FAIL}" -eq 0 ]]
