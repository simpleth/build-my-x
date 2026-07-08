@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM java-cli Uninstaller
REM Removes global link and node_modules
REM ============================================================

echo ======================================
echo java-cli Uninstaller
echo ======================================
echo.

cd /d "%~dp0"

echo This will:
echo   1. Remove the global "java-cli" command link
echo   2. Remove node_modules directory
echo.
echo Your SDK installations at %%USERPROFILE%%\.java-cli\ will NOT be deleted.
echo.

set /p CONFIRM="Continue? (y/N): "
if /i not "!CONFIRM!"=="y" (
    echo Cancelled.
    pause
    exit /b 0
)

echo.
echo Step 1: Removing global link...
call npm unlink -g 2>nul
echo [OK] Global link removed.

echo.
echo Step 2: Removing node_modules...
if exist node_modules (
    rmdir /s /q node_modules
    echo [OK] node_modules removed.
)

echo.
echo ======================================
echo Uninstall complete!
echo ======================================
echo.
echo To restore, run: setup_build.bat
echo.
pause
