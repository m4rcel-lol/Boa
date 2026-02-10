# Boa IDLE Editor

Notepad++-style integrated editor for the Boa language.

## Status

**Planned** — Not yet implemented.

## Design

The IDLE editor will be a native Windows desktop application featuring:
- Scintilla-based text editing
- Tabbed document interface
- Syntax highlighting for Boa
- Code folding
- Line numbers and breakpoint gutter
- Find/Replace with regex support
- Integrated terminal panel for Boa REPL
- Run (F5) and Stop (Shift+F5) controls
- LSP integration for diagnostics and hover
- Status bar with line/column info
- Toolbar with common actions

## Implementation

The editor will be implemented in C++ with Win32 API, embedding the Scintilla control for editing. It will follow Notepad++ UX patterns.

## Third-Party Dependencies

- **Scintilla** — Text editing component ([License](https://www.scintilla.org/License.txt))
