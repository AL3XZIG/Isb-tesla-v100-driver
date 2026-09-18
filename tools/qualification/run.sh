#!/usr/bin/env bash
set -u
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
BUILD_DIR="${ROOT}/build"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
REPORT_DIR="${ROOT}/reports/runs/${STAMP}"
mkdir -p "${REPORT_DIR}/logs"
PASS=0; FAIL=0; UNKNOWN=0
RUN_MODE="unknown"

classify_json() {
  local output_file="$1"
  if grep -q '"mode":"unavailable"' "$output_file" 2>/dev/null; then
    echo "UNKNOWN"; return
  fi
  if grep -q '"synthetic":true' "$output_file" 2>/dev/null; then
    echo "UNKNOWN"; return
  fi
  if grep -q '"verified":false' "$output_file" 2>/dev/null; then
    echo "UNKNOWN"; return
  fi
  if grep -q '"correctness_verified":false' "$output_file" 2>/dev/null; then
    echo "UNKNOWN"; return
  fi
  echo "PASS"
}

run_case() {
  local name="$1"; shift
  local log="${REPORT_DIR}/logs/${name}.log"
  local output="${REPORT_DIR}/logs/${name}.stdout"
  echo "=== ${name} ===" | tee "${log}"
  if "$@" >"${output}" 2>>"${log}"; then
    cat "${output}" >>"${log}"
    local state
    state="$(classify_json "${output}")"
    if [[ "${RUN_MODE}" != "real" ]]; then
      state="UNKNOWN"
    fi
    case "${state}" in
      PASS) printf '%s|PASS|%s\n' "${name}" "${log}" >>"${REPORT_DIR}/results.tsv"; PASS=$((PASS+1)) ;;
      UNKNOWN) printf '%s|UNKNOWN|%s\n' "${name}" "${log}" >>"${REPORT_DIR}/results.tsv"; UNKNOWN=$((UNKNOWN+1)) ;;
    esac
  else
    cat "${output}" >>"${log}"
    printf '%s|FAIL|%s\n' "${name}" "${log}" >>"${REPORT_DIR}/results.tsv"
    FAIL=$((FAIL+1))
  fi
}

printf 'ISB V100 qualification run\nstarted_utc=%s\nhost=%s\nkernel=%s\n' "${STAMP}" "$(hostname)" "$(uname -srmo 2>/dev/null || true)" > "${REPORT_DIR}/run.txt"
if [[ ! -x "${BUILD_DIR}/cli/isb" ]]; then
  echo "isb binary not found; run installer/install.sh first." | tee "${REPORT_DIR}/logs/installer.log"
  exit 2
fi

status_output="${REPORT_DIR}/logs/status.stdout"
if "${BUILD_DIR}/cli/isb" --json status >"${status_output}" 2>"${REPORT_DIR}/logs/status.stderr"; then
  if grep -q '"mode":"real"' "${status_output}"; then
    RUN_MODE="real"
  elif grep -q '"mode":"mock"' "${status_output}"; then
    RUN_MODE="mock"
  elif grep -q '"mode":"unavailable"' "${status_output}"; then
    RUN_MODE="unavailable"
  fi
else
  RUN_MODE="unknown"
fi

printf 'qualification_mode=%s\n' "${RUN_MODE}" >> "${REPORT_DIR}/run.txt"
printf 'name|state|log\n' > "${REPORT_DIR}/results.tsv"
printf '%s|%s|%s\n' "status" "${RUN_MODE}" "${REPORT_DIR}/logs/status.stderr" >> "${REPORT_DIR}/results.tsv"

run_case capabilities "${BUILD_DIR}/cli/isb" --json capabilities
run_case telemetry "${BUILD_DIR}/cli/isb" --json telemetry
run_case diagnose "${BUILD_DIR}/cli/isb" --json diagnose
run_case verify "${BUILD_DIR}/cli/isb" --json verify
run_case benchmark "${BUILD_DIR}/cli/isb" --json benchmark

"${BUILD_DIR}/cli/isb" --json capabilities > "${REPORT_DIR}/capabilities.json" 2>"${REPORT_DIR}/logs/capabilities.stderr" || true
"${BUILD_DIR}/cli/isb" --json status > "${REPORT_DIR}/environment.json" 2>"${REPORT_DIR}/logs/status-second.stderr" || true
"${BUILD_DIR}/cli/isb" --json telemetry > "${REPORT_DIR}/telemetry.json" 2>"${REPORT_DIR}/logs/telemetry.stderr" || true
"${BUILD_DIR}/cli/isb" --json diagnose > "${REPORT_DIR}/diagnose.json" 2>"${REPORT_DIR}/logs/diagnose.stderr" || true
"${BUILD_DIR}/cli/isb" --json verify > "${REPORT_DIR}/verification.json" 2>"${REPORT_DIR}/logs/verify.stderr" || true
"${BUILD_DIR}/cli/isb" --json benchmark > "${REPORT_DIR}/benchmark.json" 2>"${REPORT_DIR}/logs/benchmark.stderr" || true

cat > "${REPORT_DIR}/summary.json" <<EOF
{"schema_version":"1","run_id":"${STAMP}","qualification_mode":"${RUN_MODE}","results":{"pass":${PASS},"fail":${FAIL},"unknown":${UNKNOWN}},"artifacts":["environment.json","capabilities.json","telemetry.json","diagnose.json","verification.json","benchmark.json","results.tsv"]}
EOF

cat > "${REPORT_DIR}/manifest.json" <<EOF
{"schema_version":"1","report_type":"qualification","run_id":"${STAMP}","qualification_mode":"${RUN_MODE}","synthetic":${RUN_MODE=="mock" && echo true || echo false},"source":"isb qualification runner","summary":"summary.json"}
EOF

echo "[ISB] Qualification report: ${REPORT_DIR}"
echo "[ISB] PASS=${PASS} FAIL=${FAIL} UNKNOWN=${UNKNOWN}"

if [[ "${FAIL}" -gt 0 ]]; then
  exit 1
fi
if [[ "${UNKNOWN}" -gt 0 || "${RUN_MODE}" != "real" ]]; then
  exit 2
fi
exit 0
