# Boa Language Specification v0.1.0

## Philosophy

Boa is a Python-inspired language designed for brevity and readability. It uses indentation-based blocks, dynamic typing, minimal keywords, and implicit returns for single-expression functions. The design prioritizes predictable scoping and helpful error messages.

**Key differences from Python:**
- `fn` instead of `def`
- `imp` instead of `import`
- `ret` instead of `return` (optional — last expression is implicit return)
- No colon-less compound statements
- No decorators, generators, or async/await in v1

## Lexical Rules

### Identifiers
Identifiers start with a letter or underscore, followed by letters, digits, or underscores.

### Numeric Literals
- Integers: `42`, `0`, `-1`
- Floats: `3.14`, `0.5`, `1e10`, `2.5e-3`

### String Literals
- Double-quoted: `"hello world"`
- Single-quoted: `'hello world'`
- Escape sequences: `\n`, `\t`, `\\`, `\"`, `\'`

### Comments
Single-line comments start with `#`:
```boa
# This is a comment
x = 42  # Inline comment
```

### Whitespace and Indentation
- Indentation defines blocks (like Python)
- Use consistent indentation (spaces recommended, 4 spaces standard)
- Tabs and spaces should not be mixed

## Keywords

| Keyword | Purpose |
|---------|---------|
| `fn` | Function definition |
| `imp` | Import module |
| `ret` | Return from function (optional) |
| `if` | Conditional |
| `elif` | Else-if branch |
| `else` | Else branch |
| `for` | For loop |
| `in` | Membership/iteration |
| `while` | While loop |
| `try` | Try block |
| `except` | Exception handler |
| `finally` | Finally block |
| `pass` | No-op statement |
| `and` | Logical AND |
| `or` | Logical OR |
| `not` | Logical NOT |
| `true` | Boolean true |
| `false` | Boolean false |
| `none` | Null value |
| `class` | Class definition |

## EBNF Grammar

```ebnf
program       = { statement } ;
statement     = simple_stmt | compound_stmt ;
simple_stmt   = expression_stmt | return_stmt | import_stmt | pass_stmt ;
compound_stmt = if_stmt | for_stmt | while_stmt | fn_def | try_stmt | class_def ;

fn_def        = "fn" identifier "(" [ param_list ] ")" ":" block ;
param_list    = identifier { "," identifier } ;

if_stmt       = "if" expression ":" block
                { "elif" expression ":" block }
                [ "else" ":" block ] ;

for_stmt      = "for" identifier "in" expression ":" block ;
while_stmt    = "while" expression ":" block ;

try_stmt      = "try" ":" block
                [ "except" [ identifier ] ":" block ]
                [ "finally" ":" block ] ;

class_def     = "class" identifier ":" block ;

return_stmt   = "ret" [ expression ] ;
import_stmt   = "imp" identifier { "," identifier } ;
pass_stmt     = "pass" ;

expression_stmt = expression ;

expression    = assignment ;
assignment    = or_expr [ ( "=" | "+=" | "-=" | "*=" | "/=" ) expression ] ;
or_expr       = and_expr { "or" and_expr } ;
and_expr      = not_expr { "and" not_expr } ;
not_expr      = [ "not" ] comp_expr ;
comp_expr     = arith_expr { ( "==" | "!=" | "<" | "<=" | ">" | ">=" ) arith_expr } ;
arith_expr    = term { ( "+" | "-" ) term } ;
term          = factor { ( "*" | "/" | "%" ) factor } ;
factor        = ( "+" | "-" ) factor | power ;
power         = postfix [ "**" factor ] ;
postfix       = atom { call_or_index } ;
call_or_index = "(" [ arg_list ] ")"
              | "[" expression "]"
              | "." identifier ;
arg_list      = expression { "," expression } ;
atom          = identifier | literal | "(" expression ")"
              | list_literal | dict_literal ;
list_literal  = "[" [ expression { "," expression } ] "]" ;
dict_literal  = "{" [ dict_entry { "," dict_entry } ] "}" ;
dict_entry    = expression ":" expression ;
literal       = INTEGER | FLOAT | STRING | "true" | "false" | "none" ;

block         = NEWLINE INDENT { statement } DEDENT ;
```

## Operator Precedence (lowest to highest)

| Precedence | Operators | Associativity |
|-----------|-----------|---------------|
| 1 (lowest) | `=` `+=` `-=` `*=` `/=` | Right |
| 2 | `or` | Left |
| 3 | `and` | Left |
| 4 | `not` | Unary |
| 5 | `==` `!=` `<` `<=` `>` `>=` | Left |
| 6 | `+` `-` | Left |
| 7 | `*` `/` `%` | Left |
| 8 | Unary `+` `-` | Unary |
| 9 | `**` | Right |
| 10 (highest) | `.` `[]` `()` | Left |

## Types

| Type | Examples | Description |
|------|----------|-------------|
| `int` | `42`, `-1`, `0` | 64-bit signed integer |
| `float` | `3.14`, `1e10` | 64-bit floating point |
| `bool` | `true`, `false` | Boolean |
| `string` | `"hello"`, `'world'` | UTF-8 string |
| `list` | `[1, 2, 3]` | Dynamic array |
| `dict` | `{"a": 1}` | Key-value mapping |
| `none` | `none` | Null value |
| `function` | `fn f(): ...` | Function object |

## Truthiness

| Value | Truthy? |
|-------|---------|
| `false` | No |
| `none` | No |
| `0` (int) | No |
| `0.0` (float) | No |
| `""` (empty string) | No |
| `[]` (empty list) | No |
| `{}` (empty dict) | No |
| Everything else | Yes |

## Runtime Semantics

### Variables and Scoping
Variables are dynamically typed and lexically scoped. Assignment to a variable that exists in an outer scope updates that variable. New variables are created in the current scope if not found in any outer scope.

### Functions
Functions are first-class values. They capture their defining environment (closures). The last expression in a function body is the implicit return value. Use `ret` for early return.

```boa
fn add(a, b):
    a + b          # Implicit return

fn max_val(a, b):
    if a > b:
        ret a      # Explicit return
    ret b
```

### Modules
`imp foo` loads `foo.boa` from the same directory as the current file (or configured module path). Modules are cached after first load.

Built-in modules:
- `io` — Console I/O (`io.print()`, `io.input()`)
- `fs` — File system (`fs.read_text()`, `fs.write_text()`, `fs.read_all_bytes()`, `fs.write_all_bytes()`)

### Error Handling
```boa
try:
    risky_operation()
except e:
    io.print("Error:", e)
finally:
    cleanup()
```

### Memory Model
Automatic memory management using reference counting (`std::shared_ptr` in the C++ implementation). Objects are freed when no references remain. Cycle detection is not implemented in v1 — avoid circular references.

### Concurrency
Out of scope for v1. All execution is single-threaded.

## Built-in Functions

| Function | Description |
|----------|-------------|
| `print(args...)` | Print arguments separated by spaces, followed by newline |
| `len(x)` | Length of string, list, or dict |
| `str(x)` | Convert to string |
| `int(x)` | Convert to integer |
| `float(x)` | Convert to float |
| `type(x)` | Return type name as string |
| `range(stop)` | List of integers `[0, stop)` |
| `range(start, stop)` | List of integers `[start, stop)` |
| `range(start, stop, step)` | List of integers with step |
| `append(list, val)` | Append value to list |

## Standard Library

### io Module
```boa
imp io
io.print("Hello!")           # Print with newline
io.print("a", "b", "c")     # Print with spaces: "a b c"
name = io.input("Name: ")   # Read input with prompt
```

### fs Module
```boa
imp fs
text = fs.read_text("file.txt")
fs.write_text("out.txt", text)
data = fs.read_all_bytes("binary.dat")
fs.write_all_bytes("copy.dat", data)
```
