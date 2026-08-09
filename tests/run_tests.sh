#!/usr/bin/env bash
# ==================================================================
# HireLens - Automated Test Suite
# Builds the engine, runs it against fixture data, and asserts on
# the produced JSON output using only standard shell + python3
# (python3's json module here is just an assertion helper for the
# test harness -- it plays no role in the actual analysis engine).
# ==================================================================
set -e

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "=== HireLens Test Suite ==="

echo "[1/5] Building engine..."
make clean > /dev/null
make > /dev/null
if [ ! -f bin/hirelens_engine ]; then
    echo "FAIL: engine binary not built"
    exit 1
fi
echo "  OK: build succeeded"

TEST_OUT="tests/tmp_output"
rm -rf "$TEST_OUT"
mkdir -p "$TEST_OUT"

echo "[2/5] Running engine against sample dataset..."
./bin/hirelens_engine data/jd/job_description.txt data/resumes "$TEST_OUT" > "$TEST_OUT/run.log"
if [ ! -f "$TEST_OUT/ranking.json" ]; then
    echo "FAIL: ranking.json not produced"
    exit 1
fi
echo "  OK: ranking.json produced"

echo "[3/5] Validating JSON structure..."
python3 tests/validate_output.py "$TEST_OUT/ranking.json"
echo "  OK: JSON structure valid"

echo "[4/5] Running engine against edge-case fixtures..."
./bin/hirelens_engine tests/fixtures/edge_case_jd.txt tests/fixtures/edge_case_resumes "$TEST_OUT/edge" > "$TEST_OUT/edge_run.log"
if [ ! -f "$TEST_OUT/edge/ranking.json" ]; then
    echo "FAIL: edge case ranking.json not produced"
    exit 1
fi
python3 tests/validate_output.py "$TEST_OUT/edge/ranking.json"
echo "  OK: edge case run succeeded"

echo "[5/5] Checking known-value assertions on sample dataset..."
python3 tests/assert_known_values.py "$TEST_OUT/ranking.json"
echo "  OK: known-value assertions passed"

echo ""
echo "=== ALL TESTS PASSED ==="
