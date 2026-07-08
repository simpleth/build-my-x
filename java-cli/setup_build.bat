@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM java-cli Build Script
REM Installs dependencies and links the CLI globally
REM ============================================================

echo ======================================
echo Building java-cli
echo ======================================
echo.

REM Navigate to script directory
cd /d "%~dp0"

REM Install dependencies
echo Installing npm dependencies...
call npm install
if !errorlevel! neq 0 (
    echo.
    echo ======================================
    echo npm install failed!
    echo ======================================
    echo.
    echo Please check errors above.
    pause
    exit /b 1
)
echo [OK] Dependencies installed.

REM Link globally
echo.
echo Linking CLI globally...
call npm link
if !errorlevel! neq 0 (
    echo.
    echo ======================================
    echo npm link failed!
    echo ======================================
    echo.
    echo You may need to run this as Administrator.
    pause
    exit /b 1
)
echo [OK] CLI linked globally.

echo.
echo ======================================
echo Build complete!
echo ======================================
echo.
echo The "java-cli" command is now available globally.
echo.
echo Quick start:
echo   java-cli init my-app
echo   cd my-app
echo   java-cli build
echo   java-cli run
echo.
pause
