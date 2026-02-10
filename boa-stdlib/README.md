# Boa Standard Library

Native standard library modules for the Boa language.

## Status

Core modules (`io`, `fs`) are implemented in the interpreter. Additional modules are planned.

## Built-in Modules

### io — Console I/O
- `io.print(args...)` — Print to stdout
- `io.input(prompt)` — Read line from stdin

### fs — File System
- `fs.read_text(path)` — Read file as text
- `fs.write_text(path, content)` — Write text to file
- `fs.read_all_bytes(path)` — Read file as binary
- `fs.write_all_bytes(path, data)` — Write binary data to file

## Planned Modules

- `math` — Mathematical functions
- `json` — JSON parsing and serialization
- `http` — HTTP client (using WinHTTP)
- `os` — Operating system interaction
- `regex` — Regular expressions
