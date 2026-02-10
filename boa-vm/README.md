# Boa Bytecode VM

Optional bytecode virtual machine for the Boa language.

## Status

**Planned** — Not yet implemented.

## Design

The bytecode VM will provide faster execution than tree-walking interpretation:
- Custom compact instruction set (stack-based VM)
- Bytecode compiler (AST → bytecode)
- Bytecode assembler/disassembler tools
- All implemented in native C++ code

## Instruction Set (Draft)

| Opcode | Description |
|--------|-------------|
| `LOAD_CONST` | Push constant onto stack |
| `LOAD_VAR` | Push variable value onto stack |
| `STORE_VAR` | Pop and store to variable |
| `ADD` | Pop two, push sum |
| `SUB` | Pop two, push difference |
| `MUL` | Pop two, push product |
| `DIV` | Pop two, push quotient |
| `CMP_EQ` | Pop two, push comparison result |
| `JUMP` | Unconditional jump |
| `JUMP_IF_FALSE` | Conditional jump |
| `CALL` | Function call |
| `RETURN` | Return from function |
| `PRINT` | Print top of stack |
