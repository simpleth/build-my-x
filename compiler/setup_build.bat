@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM TinyCompiler Build Script
REM Compiles the project using MSYS2 UCRT64 toolchain
REM ============================================================

echo ======================================
echo Building TinyCompiler
echo ======================================
echo.

REM Setup environment
set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"
set "CC=gcc"
set "CXX=g++"

echo Environment setup complete.
echo.

REM Create build directory if not exists
if not exist build mkdir build
cd build

REM Run CMake
echo Running CMake configuration...
cmake -G "MinGW Makefiles" ..

if !errorlevel! neq 0 (
    echo.
    echo ======================================
    echo CMake configuration failed!
    echo ======================================
    echo.
    echo Please check errors above.
    pause
    cd ..
    exit /b 1
)

echo.
echo Building...
mingw32-make

if !errorlevel! neq 0 (
    echo.
    echo ======================================
    echo Build failed!
    echo ======================================
    echo.
    echo Please check errors above.
    pause
    cd ..
    exit /b 1
)

cd ..

echo.
echo ======================================
echo Build complete!
echo ======================================
echo.
echo Executable: build\TinyCompiler.exe
echo.
echo To run a test:
echo   run_test.bat 1
echo.
pause
