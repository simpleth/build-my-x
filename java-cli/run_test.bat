@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM java-cli Test Runner
REM Runs the full test suite
REM ============================================================

echo ======================================
echo java-cli Test Suite
echo ======================================
echo.

cd /d "%~dp0\.."

echo Running tests...
echo.
node tests\run_tests.js

if !errorlevel! neq 0 (
    echo.
    echo ======================================
    echo Some tests FAILED!
    echo ======================================
    echo.
    pause
    exit /b 1
)

echo.
echo ======================================
echo All tests PASSED!
echo ======================================
echo.
pause
