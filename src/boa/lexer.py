"""Custom tokenizer for Boa source code."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Token:
    kind: str
    value: str
    line: int
    column: int


KEYWORDS = {
    "fn",
    "ret",
    "cls",
    "use",
    "nil",
    "yes",
    "no",
    "out",
    "ask",
    "if",
    "ef",
    "else",
    "for",
    "~",
    "..",
}


DOUBLE_OPS = {
    "->",
    "==",
    "!=",
    "<=",
    ">=",
    "&&",
    "||",
    "!~",
    "==:",
    "..",
}

TRIPLE_OPS = {"!==:"}

SINGLE = set("()[]{}:,.?+-*/%=<>!|")


def _split_indent(line: str) -> tuple[str, str]:
    idx = 0
    while idx < len(line) and line[idx] in (" ", "\t"):
        idx += 1
    return line[:idx], line[idx:]


def _indent_width(indent: str) -> int:
    width = 0
    for ch in indent:
        width += 4 if ch == "\t" else 1
    return width


def tokenize(source: str) -> list[Token]:
    tokens: list[Token] = []
    indent_stack = [0]

    lines = source.splitlines()
    for line_num, raw in enumerate(lines, start=1):
        indent, body = _split_indent(raw)
        if not body.strip():
            continue

        width = _indent_width(indent)
        if width > indent_stack[-1]:
            indent_stack.append(width)
            tokens.append(Token("INDENT", "", line_num, 1))
        while width < indent_stack[-1]:
            indent_stack.pop()
            tokens.append(Token("DEDENT", "", line_num, 1))
        if width != indent_stack[-1]:
            raise ValueError(f"Invalid indentation at line {line_num}")

        i = len(indent)
        while i < len(raw):
            ch = raw[i]
            if ch.isspace():
                i += 1
                continue
            if ch == "#":
                break

            long_op = raw[i : i + 4]
            if long_op in TRIPLE_OPS:
                tokens.append(Token("OP", long_op, line_num, i + 1))
                i += 4
                continue

            double = raw[i : i + 2]
            if double in DOUBLE_OPS:
                tokens.append(Token("OP", double, line_num, i + 1))
                i += 2
                continue

            if ch in ('"', "'") or (ch in {"f", "F"} and i + 1 < len(raw) and raw[i + 1] in ('"', "'")):
                is_f = False
                quote = ch
                if ch in {"f", "F"}:
                    is_f = True
                    i += 1
                    quote = raw[i]
                start = i
                i += 1
                value_chars: list[str] = []
                escaped = False
                while i < len(raw):
                    c = raw[i]
                    if escaped:
                        value_chars.append(c)
                        escaped = False
                    elif c == "\\":
                        escaped = True
                    elif c == quote:
                        break
                    else:
                        value_chars.append(c)
                    i += 1
                if i >= len(raw) or raw[i] != quote:
                    raise ValueError(f"Unterminated string at line {line_num}")
                prefix = "f" if is_f else ""
                tokens.append(Token("STRING", prefix + "".join(value_chars), line_num, start + 1))
                i += 1
                continue

            if ch.isdigit():
                start = i
                seen_dot = False
                while i < len(raw) and (raw[i].isdigit() or (raw[i] == "." and not seen_dot and raw[i : i + 2] != "..")):
                    if raw[i] == ".":
                        seen_dot = True
                    i += 1
                tokens.append(Token("NUMBER", raw[start:i], line_num, start + 1))
                continue

            if ch.isalpha() or ch == "_":
                start = i
                i += 1
                while i < len(raw) and (raw[i].isalnum() or raw[i] == "_"):
                    i += 1
                value = raw[start:i]
                kind = "KEYWORD" if value in KEYWORDS else "IDENT"
                tokens.append(Token(kind, value, line_num, start + 1))
                continue

            if ch in SINGLE:
                kind = "OP" if ch in "+-*/%=<>!|" else "PUNCT"
                tokens.append(Token(kind, ch, line_num, i + 1))
                i += 1
                continue

            if ch == "~":
                tokens.append(Token("KEYWORD", "~", line_num, i + 1))
                i += 1
                continue

            raise ValueError(f"Unexpected character '{ch}' at line {line_num}:{i + 1}")

        tokens.append(Token("NEWLINE", "", line_num, len(raw) + 1))

    while len(indent_stack) > 1:
        indent_stack.pop()
        tokens.append(Token("DEDENT", "", len(lines) + 1, 1))

    tokens.append(Token("EOF", "", len(lines) + 1, 1))
    return tokens
