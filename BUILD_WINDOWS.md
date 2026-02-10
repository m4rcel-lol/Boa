# Building Boa from Source (Windows)

This document describes how to build the Boa programming language from source on Windows.

## Prerequisites

- **Visual Studio 2017** (or later) with C++ desktop development workload
  - Or **MSVC Build Tools** (command-line only)
- **CMake 3.16+** (included with Visual Studio or install separately)
- **Git** (for cloning the repository)

## Build Steps

### 1. Clone the Repository

```cmd
git clone https://github.com/m4rcel-lol/Boa.git
cd Boa
```

### 2. Configure with CMake

Open a **Developer Command Prompt for Visual Studio** (or x64 Native Tools Command Prompt):

```cmd
mkdir build
cd build
cmake .. -A x64
```

Or for NMake:

```cmd
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
```

### 3. Build

```cmd
cmake --build . --config Release
```

The output binaries will be in `build\Release\`:
- `boa.exe` — The Boa interpreter and REPL

### 4. Run Tests

```cmd
ctest --output-on-failure -C Release
```

Or run the test binary directly:

```cmd
Release\boa_tests.exe
```

### 5. Install (Optional)

```cmd
cmake --install . --prefix "C:\Program Files\Boa"
```

This installs `boa.exe` to `C:\Program Files\Boa\bin\`.

## Build Configuration Options

| Option | Default | Description |
|--------|---------|-------------|
| `CMAKE_BUILD_TYPE` | `Debug` | Build type (Debug/Release/RelWithDebInfo) |
| `CMAKE_INSTALL_PREFIX` | System default | Installation directory |

## Batch Build Script

A convenience script `build.bat` is provided:

```cmd
build.bat
```

This will:
1. Create the build directory
2. Configure CMake with MSVC
3. Build in Release mode
4. Run tests

## Troubleshooting

### CMake not found
Ensure CMake is in your PATH. If using Visual Studio, open the **Developer Command Prompt** which sets up the environment automatically.

### Compiler errors
Ensure you're using MSVC (cl.exe) and not MinGW. The project requires C++17 support.

### NMake Makefiles error with `-A x64`
If you see an error like `Generator NMake Makefiles does not support platform specification`, you are likely running from a **Developer Command Prompt** where CMake defaults to NMake. Use the NMake command instead:

```cmd
cmake .. -G "NMake Makefiles" -DCMAKE_BUILD_TYPE=Release
```

The `build.bat` script handles this automatically by falling back to NMake Makefiles when the Visual Studio generator is not available.

### Test failures
Run with verbose output: `ctest --output-on-failure -V -C Release`

## Cross-Platform Development Note

While the source code is standard C++17 and may compile on Linux/macOS for development purposes, **only Windows is officially supported**. Linux and macOS users must produce their own ports independently.
