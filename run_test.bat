@echo off
setlocal EnableDelayedExpansion
REM ============================================================
REM TinyCompiler Test Runner
REM Compiles and runs test programs
REM Usage: run_test.bat <number>
REM ============================================================

set "PATH=C:\msys64\ucrt64\bin;C:\msys64\usr\bin;%PATH%"

if "%1"=="" (
    echo Usage: run_test.bat ^<number^>
    echo.
    echo Available tests:
    echo   1 - Basic print
    echo   2 - Arithmetic operations
    echo   3 - Conditional branching
    echo   4 - Loop
    echo   5 - Function definition and call
    echo.
    pause
    exit /b 1
)

set TEST_NUM=%1
set TEST_FILE=tests\test%TEST_NUM%.tiny

if not exist %TEST_FILE% (
    echo Error: Test file %TEST_FILE% not found!
    pause
    exit /b 1
)

echo Compiling %TEST_FILE%...
build\TinyCompiler.exe %TEST_FILE% -o test_temp

if !errorlevel! neq 0 (
    echo Compiler failed!
    pause
    exit /b 1
)

echo.
echo Linking...
gcc test_temp.o runtime.c -o test%TEST_NUM%.exe

if !errorlevel! neq 0 (
    echo Linking failed!
    pause
    exit /b 1
)

echo.
echo ======================================
echo Running test%TEST_NUM%.exe:
echo ======================================
echo.

test%TEST_NUM%.exe

echo.
echo ======================================
echo Test complete!
echo ======================================
pause
