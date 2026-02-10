@echo off
REM Boa Language - Build Script for Windows
REM Requires: Visual Studio (2017 or later) with C++ workload, CMake 3.16+

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

REM Try Visual Studio generator first (supports -A x64 platform flag)
cmake .. -A x64 >nul 2>&1
if %errorlevel% equ 0 (
    echo Configured with Visual Studio generator.
    set MULTI_CONFIG=1
) else (
    REM Fall back to NMake Makefiles (e.g., from Developer Command Prompt)
    echo Visual Studio generator not available, trying NMake Makefiles...
    cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
    if %errorlevel% neq 0 (
        echo ERROR: CMake configuration failed.
        cd ..
        exit /b 1
    )
    set MULTI_CONFIG=0
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
if "%MULTI_CONFIG%"=="1" (
    echo  Binary: build\Release\boa.exe
) else (
    echo  Binary: build\boa.exe
)
echo ========================================
