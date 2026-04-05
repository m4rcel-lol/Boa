# Boa

Boa is a statically-transpiled shorthand superset of Python. Every valid Boa program compiles to valid Python 3.x and runs on the host Python interpreter.

## Philosophy

> Every syntax construct must be expressible in fewer characters than its Python equivalent.

Boa is not a new runtime; it is a source-to-source transpiler.

## Syntax Cheat Sheet

### Keywords and built-ins

| Python | Boa |
|---|---|
| def | fn |
| return | ret |
| import | use |
| from X import Y | use X: Y |
| class | cls |
| self | s |
| None | nil |
| True | yes |
| False | no |
| lambda | \\ |
| and | && |
| or | \|\| |
| not | ! |
| in | ~ |
| not in | !~ |
| is | ==: |
| is not | !==: |
| pass | .. |
| raise | err |
| assert | chk |
| yield | yld |
| async def | afn |
| await | aw |
| global | gbl |
| nonlocal | nlo |
| with X as Y | with X>Y |
| try / except / finally | try / catch / end |
| elif | ef |
| print(x) | out x |
| input(x) | ask x |
| len(x) | #x |
| range(x) | ..x |
| range(x, y) | x..y |
| range(x, y, z) | x..y..z |

### Type hints

| Python | Boa |
|---|---|
| int | i |
| str | s |
| float | f |
| bool | b |
| list[T] | [T] |
| dict[K, V] | {K:V} |
| Optional[T] | T? |
| Union[A, B] | A\|B |

## Install

### Pip (editable/dev)

```bash
python -m pip install -e .
```

or with dev dependencies:

```bash
python -m pip install -e .[dev]
```

### Standalone binary

Download the latest release artifact for your platform:
- `boa-linux-x86_64`
- `boa-windows-x86_64.exe`

Then execute directly:

```bash
./boa-linux-x86_64 help
```

## Usage

```bash
boa help
boa version
boa check tests/samples/hello.boa
boa build tests/samples/hello.boa
boa run tests/samples/hello.boa
```

### Example Boa

```boa
use os

fn greet(name: s) -> s:
    ret f"Hello, {name}!"

out greet("Boa")
```

## Development

### Run tests

```bash
python -m pip install -e .[dev]
pytest
```

### Contributing Guide

1. Fork and clone the repository.
2. Create a feature branch.
3. Install dev dependencies and run tests.
4. Keep changes small and focused.
5. Submit a pull request with clear context and examples.

## License

MIT
