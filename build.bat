@echo off
REM Boa Language - Build Script for Windows
REM Requires: Visual Studio 2022 with C++ workload, CMake 3.16+

echo ========================================
echo  Boa Language Build Script
echo ========================================
echo.

REM Check for CMake
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo ERROR: CMake not found. Please install CMake or open a Developer Command Prompt.
    exit /b 1
)

REM Create build directory
if not exist build mkdir build
cd build

REM Configure
echo [1/3] Configuring CMake...
cmake .. -G "Visual Studio 17 2022" -A x64
if %errorlevel% neq 0 (
    echo ERROR: CMake configuration failed.
    cd ..
    exit /b 1
)

REM Build
echo [2/3] Building...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo ERROR: Build failed.
    cd ..
    exit /b 1
)

REM Test
echo [3/3] Running tests...
ctest --output-on-failure -C Release
if %errorlevel% neq 0 (
    echo WARNING: Some tests failed.
)

cd ..
echo.
echo ========================================
echo  Build complete!
echo  Binary: build\Release\boa.exe
echo ========================================
