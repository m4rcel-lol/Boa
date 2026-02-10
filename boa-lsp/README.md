# Boa LSP Server

Language Server Protocol implementation for the Boa language.

## Status

**Planned** — Not yet implemented.

## Design

The LSP server will be a native Windows binary that provides:
- Diagnostics (syntax errors, type warnings)
- Completions (keywords, variable names, module members)
- Hover information (type info, documentation)
- Go-to-definition

Communication will be via stdin/stdout (standard LSP transport).

## Implementation

The LSP server will reuse the tokenizer and parser from `boa-lang` to provide real-time diagnostics. It will be implemented in C++ as a native binary.
