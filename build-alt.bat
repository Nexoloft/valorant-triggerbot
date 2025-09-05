@echo off
REM Alternative build script using Microsoft Visual C++ compiler
REM This script tries multiple compilers and build methods

echo ==========================================
echo Educational Triggerbot Build Script
echo ==========================================

REM Create output directory
if not exist "bin" mkdir bin

REM Method 1: Try g++ (MinGW)
echo Checking for g++ compiler...
where g++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found g++ compiler, building...
    g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o bin/triggerbot.exe -lgdi32 -luser32
    if %ERRORLEVEL% EQU 0 (
        echo BUILD SUCCESSFUL with g++!
        goto :success
    )
)

REM Method 2: Try cl (Visual Studio)
echo Checking for cl compiler...
where cl >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found Visual Studio compiler, building...
    cl main.cpp /std:c++17 /O2 /Fe:bin/triggerbot.exe user32.lib gdi32.lib
    if %ERRORLEVEL% EQU 0 (
        echo BUILD SUCCESSFUL with Visual Studio!
        goto :success
    )
)

REM Method 3: Try clang++ if available
echo Checking for clang++ compiler...
where clang++ >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    echo Found clang++ compiler, building...
    clang++ -std=c++17 -Wall -Wextra -O2 main.cpp -o bin/triggerbot.exe -lgdi32 -luser32
    if %ERRORLEVEL% EQU 0 (
        echo BUILD SUCCESSFUL with clang++!
        goto :success
    )
)

REM If we get here, no compiler worked
echo ==========================================
echo BUILD FAILED - NO COMPILER FOUND!
echo ==========================================
echo.
echo Please install one of the following:
echo.
echo 1. MinGW-w64 (Recommended)
echo    Download from: https://www.mingw-w64.org/downloads/
echo    Or use MSYS2: https://www.msys2.org/
echo.
echo 2. Visual Studio Community (Free)
echo    Download from: https://visualstudio.microsoft.com/
echo    Make sure to install "Desktop development with C++" workload
echo.
echo 3. Build Tools for Visual Studio
echo    Download from: https://visualstudio.microsoft.com/downloads/
echo    Select "C++ build tools"
echo.
echo After installation, make sure the compiler is in your PATH
echo or run this script from a "Developer Command Prompt"
echo.
goto :end

:success
echo.
echo ==========================================
echo Executable created: bin\triggerbot.exe
echo ==========================================
echo.
echo To run the program:
echo   bin\triggerbot.exe
echo.
echo Or run this script with 'run' parameter:
echo   build-alt.bat run
echo.

REM If 'run' parameter is passed, execute the program
if "%1"=="run" (
    echo Starting triggerbot...
    echo.
    bin\triggerbot.exe
)

:end
echo.
pause
