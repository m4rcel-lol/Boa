# Boa

Boa is a standalone custom language with its own lexer, parser, semantic analysis, and interpreter runtime.

## Philosophy

> Every syntax construct must be expressible in fewer characters than its Python equivalent.

Boa is not a source-to-source transpiler.

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
| and | && |
| or | \|\| |
| not | ! |
| in | ~ |
| not in | !~ |
| is | ==: |
| is not | !==: |
| pass | .. |
| elif | ef |
| print(x) | out x |
| input(x) | ask x |

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

```bash
python -m pip install -e .
```

or with dev dependencies:

```bash
python -m pip install -e .[dev]
```

## Usage

```bash
boa help
boa version
boa install /usr/local/bin/boa
boa check tests/samples/hello.boa
boa build tests/samples/hello.boa
boa run tests/samples/hello.boa
```

`build` emits a Boa compiler artifact (`.boac`) instead of Python code.
`install` copies the current Boa executable/script to the path you provide.

## Example Boa

```boa
fn greet(name: s) -> s:
    ret f"Hello, {name}!"

out greet("Boa")
```

## Development

```bash
python -m pip install -e .[dev]
pytest
```

## License

MIT
