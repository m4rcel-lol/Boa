# Boa Security & Sandboxing

## Security Model

**By default, executing arbitrary Boa code is unsafe.** The interpreter has full access to:
- The filesystem (read/write any file the process can access)
- Standard I/O (console input/output)
- Any system resource accessible to the process

**Do not run untrusted Boa code without understanding these risks.**

## Warnings

- Boa scripts can read and write arbitrary files
- Boa scripts can execute long-running computations
- There is no memory limit enforcement in v1
- There is no execution time limit in v1

## Planned Sandbox Mode (Future)

A future version will include an optional sandbox mode that restricts:
- Filesystem access (configurable allowed paths)
- Network calls (disabled by default)
- Resource limits (memory, CPU time)

The sandbox will be implemented in native code as policy-based checks in the runtime.

## Recommendations

- Only run Boa scripts from trusted sources
- Review scripts before executing them
- Use filesystem permissions to restrict the interpreter's access
- Run in a sandboxed environment (VM, container) for untrusted code
