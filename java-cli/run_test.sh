#!/usr/bin/env bash
# ============================================================
# java-cli Test Runner
# Runs the full test suite
# ============================================================

set -e

echo "======================================"
echo "java-cli Test Suite"
echo "======================================"
echo ""

cd "$(dirname "$0")/.."

echo "Running tests..."
echo ""
node tests/run_tests.js
echo ""

echo "======================================"
echo "All tests PASSED!"
echo "======================================"
echo ""
