@echo off
REM Boa Language - Test Runner for Windows

echo Running Boa Test Suite...
echo.

if not exist build\Release\boa_tests.exe (
    if not exist build\Debug\boa_tests.exe (
        echo ERROR: Tests not built. Run build.bat first.
        exit /b 1
    )
    set TEST_EXE=build\Debug\boa_tests.exe
) else (
    set TEST_EXE=build\Release\boa_tests.exe
)

echo Running unit tests...
%TEST_EXE%
if %errorlevel% neq 0 (
    echo UNIT TESTS FAILED
    exit /b 1
)

echo.
echo Running integration tests...

if not exist build\Release\boa.exe (
    if not exist build\Debug\boa.exe (
        echo ERROR: boa.exe not built. Run build.bat first.
        exit /b 1
    )
    set BOA_EXE=build\Debug\boa.exe
) else (
    set BOA_EXE=build\Release\boa.exe
)

for %%f in (examples\*.boa) do (
    echo   Running %%f...
    %BOA_EXE% %%f
    if %errorlevel% neq 0 (
        echo   FAILED: %%f
        exit /b 1
    )
)

echo.
echo All tests passed!
