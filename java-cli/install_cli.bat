@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM java-cli Dependency Installer
REM Checks and installs Node.js if missing
REM ============================================================

echo ======================================
echo java-cli Dependency Installer
echo ======================================
echo.

REM Check Node.js
echo Checking Node.js...
where node >nul 2>&1
if !errorlevel! equ 0 (
    for /f "tokens=*" %%v in ('node --version') do set NODE_VER=%%v
    echo [OK] Node.js is installed: !NODE_VER!
) else (
    echo [INFO] Node.js not found.
    echo.
    echo Please install Node.js from: https://nodejs.org/
    echo Or use a package manager:
    echo   winget install OpenJS.NodeJS.LTS
    echo   choco install nodejs-lts
    echo.
    pause
    exit /b 1
)

REM Check npm
echo Checking npm...
where npm >nul 2>&1
if !errorlevel! equ 0 (
    for /f "tokens=*" %%v in ('npm --version') do set NPM_VER=%%v
    echo [OK] npm is installed: !NPM_VER!
) else (
    echo [ERROR] npm not found. Please reinstall Node.js.
    pause
    exit /b 1
)

echo.
echo ======================================
echo All dependencies are installed!
echo ======================================
echo.
echo Run setup_build.bat to install the CLI.
echo.
pause
endlocal
