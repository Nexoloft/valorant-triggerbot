@echo off
REM Build script for Educational Triggerbot
REM This script compiles the triggerbot using g++ (MinGW)

echo ==========================================
echo Educational Triggerbot Build Script
echo ==========================================

REM Check if g++ is available
where g++ >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: g++ compiler not found!
    echo Please install MinGW or add it to your PATH.
    echo You can download MinGW from: https://www.mingw-w64.org/
    pause
    exit /b 1
)

REM Create output directory
if not exist "bin" mkdir bin

REM Compile the program
echo Compiling triggerbot...
g++ -std=c++17 -Wall -Wextra -O2 -pedantic main.cpp -o bin/triggerbot.exe -lgdi32 -luser32

REM Check if compilation was successful
if %ERRORLEVEL% EQU 0 (
    echo.
    echo ==========================================
    echo BUILD SUCCESSFUL!
    echo ==========================================
    echo Executable created: bin\triggerbot.exe
    echo.
    echo To run the program, use:
    echo   bin\triggerbot.exe
    echo.
    echo Or run this script with 'run' parameter:
    echo   build.bat run
    echo.
    
    REM If 'run' parameter is passed, execute the program
    if "%1"=="run" (
        echo Starting triggerbot...
        echo.
        bin\triggerbot.exe
    )
) else (
    echo.
    echo ==========================================
    echo BUILD FAILED!
    echo ==========================================
    echo Please check the error messages above.
    echo Make sure you have a C++17 compatible compiler.
)

echo.
pause
