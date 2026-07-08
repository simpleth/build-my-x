@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM TinyCompiler Uninstall/Recovery Tool
REM Removes installed packages or cleans up MSYS2
REM ============================================================

echo ======================================
echo TinyCompiler Uninstall/Recovery Tool
echo ======================================
echo.
echo Select action:
echo.
echo 1. Uninstall LLVM, Clang, CMake only (Recommended)
echo    - Removes packages installed by install_llvm.bat
echo    - Keeps GCC and other MSYS2 software
echo.
echo 2. Clean download cache
echo    - Frees disk space
echo    - Does not affect installed packages
echo.
echo 3. Cancel
echo.

set /p CHOICE="Enter choice (1/2/3): "

if "!CHOICE!"=="1" goto UNINSTALL_PACKAGES
if "!CHOICE!"=="2" goto CLEAN_CACHE
if "!CHOICE!"=="3" goto CANCEL

echo Invalid choice!
pause
exit /b 1

:UNINSTALL_PACKAGES
echo.
echo ======================================
echo Uninstall LLVM, Clang, CMake
echo ======================================
echo.
echo This will remove:
echo   - mingw-w64-ucrt-x86_64-llvm
echo   - mingw-w64-ucrt-x86_64-clang
echo   - mingw-w64-ucrt-x86_64-cmake
echo.
echo GCC and other MSYS2 packages will be kept.
echo.

set /p CONFIRM="Confirm uninstall? (y/n): "
if /i not "!CONFIRM!"=="y" (
    echo Cancelled.
    pause
    exit /b 0
)

echo.
echo Uninstalling...
C:\msys64\usr\bin\bash.exe -lc "pacman -Rns --noconfirm mingw-w64-ucrt-x86_64-llvm mingw-w64-ucrt-x86_64-clang mingw-w64-ucrt-x86_64-cmake"

echo.
echo ======================================
echo Uninstall complete!
echo ======================================
echo To reinstall, run: install_llvm.bat
pause
exit /b 0

:CLEAN_CACHE
echo.
echo ======================================
echo Clean download cache
echo ======================================
echo.
echo This will remove downloaded package files.
echo Installed packages will not be affected.
echo.

set /p CONFIRM="Confirm clean cache? (y/n): "
if /i not "!CONFIRM!"=="y" (
    echo Cancelled.
    pause
    exit /b 0
)

echo.
echo Cleaning...
C:\msys64\usr\bin\bash.exe -lc "pacman -Scc --noconfirm"

echo.
echo ======================================
echo Cache cleaned!
echo ======================================
pause
exit /b 0

:CANCEL
echo.
echo Operation cancelled.
pause
exit /b 0
