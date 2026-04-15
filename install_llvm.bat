@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM TinyCompiler Dependency Installer
REM Checks and installs LLVM, Clang, CMake via MSYS2
REM ============================================================

echo ======================================
echo TinyCompiler Dependency Installer
echo ======================================
echo.

set "BASH=C:\msys64\usr\bin\bash.exe"
set "PACKAGES_TO_INSTALL="

REM Check GCC
echo Checking GCC...
%BASH% -lc "gcc --version >nul 2>&1"
if !errorlevel! equ 0 (
    echo [OK] GCC is already installed
) else (
    echo [INFO] GCC not found
    set "PACKAGES_TO_INSTALL=!PACKAGES_TO_INSTALL! mingw-w64-ucrt-x86_64-gcc"
)

REM Check LLVM
echo Checking LLVM...
%BASH% -lc "llvm-config --version >nul 2>&1"
if !errorlevel! equ 0 (
    echo [OK] LLVM is already installed
) else (
    echo [INFO] LLVM not found
    set "PACKAGES_TO_INSTALL=!PACKAGES_TO_INSTALL! mingw-w64-ucrt-x86_64-llvm"
)

REM Check Clang
echo Checking Clang...
%BASH% -lc "clang --version >nul 2>&1"
if !errorlevel! equ 0 (
    echo [OK] Clang is already installed
) else (
    echo [INFO] Clang not found
    set "PACKAGES_TO_INSTALL=!PACKAGES_TO_INSTALL! mingw-w64-ucrt-x86_64-clang"
)

REM Check CMake
echo Checking CMake...
%BASH% -lc "cmake --version >nul 2>&1"
if !errorlevel! equ 0 (
    echo [OK] CMake is already installed
) else (
    echo [INFO] CMake not found
    set "PACKAGES_TO_INSTALL=!PACKAGES_TO_INSTALL! mingw-w64-ucrt-x86_64-cmake"
)

echo.

REM If nothing to install
if "!PACKAGES_TO_INSTALL!"=="" (
    echo ======================================
    echo All dependencies are installed!
    echo ======================================
    echo Run setup_build.bat to compile the project.
    echo.
    pause
    endlocal
    exit /b 0
)

echo ======================================
echo Installing missing dependencies
echo ======================================
echo.
echo Will install: !PACKAGES_TO_INSTALL!
echo.

REM Sync package database
echo Step 1: Syncing package database...
%BASH% -lc "pacman -Sy --noconfirm"

echo.
echo Step 2: Installing packages...
%BASH% -lc "pacman -S --noconfirm!PACKAGES_TO_INSTALL!"

if !errorlevel! equ 0 (
    echo.
    echo ======================================
    echo Installation complete!
    echo ======================================
    echo.
    echo You can now run: setup_build.bat
    echo.
) else (
    echo.
    echo ======================================
    echo Installation failed!
    echo ======================================
    echo.
    echo Please check errors above.
    echo You can also install packages manually via MSYS2 UCRT64 terminal.
    echo.
)

pause
endlocal
