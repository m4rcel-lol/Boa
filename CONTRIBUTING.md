# Contributing to Boa

Thank you for your interest in contributing to Boa!

## Getting Started

1. Fork the repository
2. Clone your fork
3. Create a feature branch: `git checkout -b my-feature`
4. Make your changes
5. Run tests: `cd build && ctest --output-on-failure`
6. Commit your changes
7. Push to your fork and open a Pull Request

## Development Environment

- **Windows 10/11** with Visual Studio 2022
- **CMake 3.16+**
- C++17 compiler (MSVC recommended)

## Code Style

- Use C++17 features where appropriate
- Follow existing code patterns in the repository
- Use `snake_case` for functions and variables
- Use `PascalCase` for types and classes
- Keep headers self-contained (`#pragma once`)

## Testing

- Add unit tests for new features in `tests/unit/test_main.cpp`
- Add integration test scripts in `examples/`
- All tests must pass before submitting a PR

## Reporting Issues

- Use GitHub Issues
- Include Boa version, Windows version, and steps to reproduce
- Include the Boa source code that triggers the issue

## License

By contributing, you agree that your contributions will be licensed under the MIT License.
