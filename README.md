# Boa Programming Language

**Boa** is a Python-inspired programming language with shorter syntax, implemented as a native compiled interpreter. No Python, Node, JVM, or other high-level runtimes are used — the entire runtime executes Boa source directly in native code.

## Features

- **Python-inspired syntax** with indentation-based blocks
- **Shorter keywords**: `fn` for functions, `imp` for imports, `ret` for return
- **Dynamic typing** with automatic memory management
- **Native compiled interpreter** (C++17, no external runtime dependencies)
- **Interactive REPL** with history and built-in commands
- **Standard library** modules: `io`, `fs`
- **Built-in functions**: `len`, `str`, `int`, `float`, `type`, `range`, `append`, `print`

## Platform Support

**Windows only** (Windows 10/11, x64). This project is strictly Windows-centric. Linux and macOS ports are **out of scope** and must be produced independently by third parties. All delivered artifacts, installers, and documentation are Windows-specific.

> **Note for other OS users:** The C++ source code is standard C++17 and may compile on other platforms for development/testing purposes, but only Windows is officially supported and tested.

## Quick Start

### Running the REPL

```cmd
boa
```

This starts the interactive Boa shell:

```
Boa v0.1.0 REPL (type :help for commands, Ctrl+C to exit)
>>> imp io
>>> io.print("Hello, Boa!")
Hello, Boa!
>>> fn add(a, b): a + b
>>> print(add(3, 4))
7
```

### Running a Script

```cmd
boa myscript.boa
```

### Hello World

```boa
imp io
io.print("Hello, Boa!")
```

### Fibonacci

```boa
fn fib(n):
    if n < 2:
        n
    else:
        fib(n - 1) + fib(n - 2)

print(fib(10))
```

## REPL Commands

| Command | Description |
|---------|-------------|
| `:help` | Show available commands |
| `:run <file>` | Run a Boa script file |
| `:load <file>` | Load and execute a file in current session |
| `:doc <symbol>` | Show documentation for a symbol |
| `:quit` | Exit the REPL |

## Building from Source

See [BUILD_WINDOWS.md](BUILD_WINDOWS.md) for detailed build instructions.

### Quick Build (Windows, MSVC)

```cmd
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

### Quick Build (Linux/macOS, development only)

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

## Running Tests

```cmd
cd build
ctest --output-on-failure
```

Or run the test executable directly:

```cmd
build\boa_tests.exe
```

## Project Structure

```
/boa-lang/          Core runtime (C++/MSVC) - tokenizer, parser, AST, interpreter
/boa-lang/include/  Header files
/boa-lang/src/      Source files
/boa-vm/            Optional bytecode VM & assembler (planned)
/boa-stdlib/        Native stdlib modules (planned)
/boa-lsp/           LSP server (planned)
/boa-idle/          Notepad++-style editor (planned)
/examples/          Example .boa scripts
/tests/             Unit and integration tests
/installer/         NSIS installer scripts
/docs/              Documentation
```

## Language Reference

See [docs/LANGUAGE_SPEC.md](docs/LANGUAGE_SPEC.md) for the complete language specification.

## Examples

See the [examples/](examples/) directory for sample Boa programs:

- `hello.boa` — Hello World
- `fibonacci.boa` — Fibonacci sequence with recursion
- `calculator.boa` — Simple calculator functions
- `loops.boa` — For and while loops
- `lists.boa` — List operations
- `dict.boa` — Dictionary operations
- `string_ops.boa` — String manipulation
- `error_handling.boa` — Try/except error handling
- `recursion.boa` — Recursive algorithms
- `scope.boa` — Scoping and closures
- `fizzbuzz.boa` — Classic FizzBuzz

## Security

By default, executing arbitrary Boa code is **unsafe** — it has full access to the filesystem and system. See [docs/SECURITY.md](docs/SECURITY.md) for details.

## License

MIT License. See [LICENSE](LICENSE) for details.

## Third-Party Components

- **Scintilla** (planned, for IDLE editor) — License: [Scintilla License](https://www.scintilla.org/License.txt)

## Contributing

See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

## Implementation Notes

- The interpreter is a tree-walking interpreter with reference-counted objects (`std::shared_ptr`)
- No transpilation to any other language occurs at runtime
- No Python, Node.js, JVM, or other high-level language runtime is used
- The entire runtime is compiled native C++ code
