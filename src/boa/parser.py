"""Recursive-descent parser for Boa."""

from __future__ import annotations

from dataclasses import dataclass

from .ast_nodes import (
    AssignStmt,
    AttrExpr,
    BinaryExpr,
    BoolExpr,
    CallExpr,
    ClassDef,
    DictExpr,
    Expr,
    ExprStmt,
    ForStmt,
    FunctionDef,
    IfStmt,
    ListExpr,
    NameExpr,
    NilExpr,
    NumberExpr,
    OutStmt,
    Param,
    PassStmt,
    Program,
    ReturnStmt,
    StringExpr,
    UnaryExpr,
    UseStmt,
)
from .lexer import Token, tokenize


@dataclass
class _Stream:
    tokens: list[Token]
    index: int = 0

    def peek(self) -> Token:
        return self.tokens[self.index]

    def advance(self) -> Token:
        tok = self.tokens[self.index]
        self.index += 1
        return tok

    def match(self, kind: str, value: str | None = None) -> bool:
        tok = self.peek()
        if tok.kind != kind:
            return False
        if value is not None and tok.value != value:
            return False
        self.advance()
        return True

    def expect(self, kind: str, value: str | None = None) -> Token:
        tok = self.peek()
        if tok.kind != kind or (value is not None and tok.value != value):
            target = f"{kind}:{value}" if value is not None else kind
            raise ValueError(f"Expected {target} at {tok.line}:{tok.column}")
        return self.advance()


PRECEDENCE = {
    "||": 1,
    "&&": 2,
    "==": 3,
    "!=": 3,
    "==:": 3,
    "!==:": 3,
    "<": 4,
    ">": 4,
    "<=": 4,
    ">=": 4,
    "~": 4,
    "!~": 4,
    "+": 5,
    "-": 5,
    "*": 6,
    "/": 6,
    "%": 6,
}


def parse_source(source: str) -> Program:
    stream = _Stream(tokenize(source))
    stmts: list = []
    while stream.peek().kind != "EOF":
        if stream.match("NEWLINE"):
            continue
        stmts.append(_parse_stmt(stream))
    return Program(stmts)


def _parse_block(stream: _Stream) -> list:
    stream.expect("NEWLINE")
    stream.expect("INDENT")
    out: list = []
    while not stream.match("DEDENT"):
        if stream.match("NEWLINE"):
            continue
        out.append(_parse_stmt(stream))
    return out


def _parse_type_text(stream: _Stream, stops: set[tuple[str, str | None]]) -> str:
    parts: list[str] = []
    depth = 0
    while True:
        tok = stream.peek()
        if tok.kind == "EOF":
            break
        stop = any(tok.kind == k and (v is None or tok.value == v) for k, v in stops)
        if stop and depth == 0:
            break
        if tok.value in "[{":
            depth += 1
        elif tok.value in "]}":
            depth -= 1
        parts.append(stream.advance().value)
    return "".join(parts).strip()


def _parse_stmt(stream: _Stream):
    tok = stream.peek()

    if tok.kind == "KEYWORD" and tok.value == "use":
        stream.advance()
        module = stream.expect("IDENT").value
        names = None
        if stream.match("PUNCT", ":"):
            names = []
            names.append(stream.expect("IDENT").value)
            while stream.match("PUNCT", ","):
                names.append(stream.expect("IDENT").value)
        stream.expect("NEWLINE")
        return UseStmt(module, names)

    if tok.kind == "KEYWORD" and tok.value == "fn":
        stream.advance()
        name = stream.expect("IDENT").value
        stream.expect("PUNCT", "(")
        params: list[Param] = []
        if not stream.match("PUNCT", ")"):
            while True:
                param_name = stream.expect("IDENT").value
                annotation = None
                if stream.match("PUNCT", ":"):
                    annotation = _parse_type_text(stream, {("PUNCT", ","), ("PUNCT", ")")})
                params.append(Param(param_name, annotation))
                if stream.match("PUNCT", ")"):
                    break
                stream.expect("PUNCT", ",")
        ret_ann = None
        if stream.match("OP", "->"):
            ret_ann = _parse_type_text(stream, {("PUNCT", ":")})
        stream.expect("PUNCT", ":")
        body = _parse_block(stream)
        return FunctionDef(name, params, ret_ann, body)

    if tok.kind == "KEYWORD" and tok.value == "cls":
        stream.advance()
        name = stream.expect("IDENT").value
        base_name = None
        if stream.match("PUNCT", "("):
            base_name = stream.expect("IDENT").value
            stream.expect("PUNCT", ")")
        stream.expect("PUNCT", ":")
        body = _parse_block(stream)
        return ClassDef(name, base_name, body)

    if tok.kind == "KEYWORD" and tok.value == "ret":
        stream.advance()
        if stream.peek().kind == "NEWLINE":
            stream.advance()
            return ReturnStmt(None)
        expr = _parse_expr(stream)
        stream.expect("NEWLINE")
        return ReturnStmt(expr)

    if tok.kind == "KEYWORD" and tok.value == "out":
        stream.advance()
        expr = _parse_expr(stream)
        stream.expect("NEWLINE")
        return OutStmt(expr)

    if tok.kind == "KEYWORD" and tok.value == "if":
        stream.advance()
        cond = _parse_expr(stream)
        stream.expect("PUNCT", ":")
        body = _parse_block(stream)

        elif_blocks: list[tuple[Expr, list]] = []
        else_body = None

        while stream.peek().kind == "KEYWORD" and stream.peek().value == "ef":
            stream.advance()
            ec = _parse_expr(stream)
            stream.expect("PUNCT", ":")
            eb = _parse_block(stream)
            elif_blocks.append((ec, eb))

        if stream.peek().kind == "KEYWORD" and stream.peek().value == "else":
            stream.advance()
            stream.expect("PUNCT", ":")
            else_body = _parse_block(stream)

        return IfStmt(cond, body, elif_blocks, else_body)

    if tok.kind == "KEYWORD" and tok.value == "for":
        stream.advance()
        name = stream.expect("IDENT").value
        stream.expect("KEYWORD", "~")
        iterable = _parse_expr(stream)
        stream.expect("PUNCT", ":")
        body = _parse_block(stream)
        return ForStmt(name, iterable, body)

    if tok.kind == "KEYWORD" and tok.value == "..":
        stream.advance()
        stream.expect("NEWLINE")
        return PassStmt()

    if tok.kind == "IDENT":
        look = stream.tokens[stream.index + 1]
        if (look.kind == "OP" and look.value == "=") or (look.kind == "PUNCT" and look.value == ":"):
            name = stream.advance().value
            annotation = None
            if stream.match("PUNCT", ":"):
                annotation = _parse_type_text(stream, {("OP", "=")})
            stream.expect("OP", "=")
            expr = _parse_expr(stream)
            stream.expect("NEWLINE")
            return AssignStmt(name, annotation, expr)

    expr = _parse_expr(stream)
    stream.expect("NEWLINE")
    return ExprStmt(expr)


def _parse_expr(stream: _Stream, min_prec: int = 1) -> Expr:
    left = _parse_unary(stream)
    while True:
        tok = stream.peek()
        if tok.kind not in {"OP", "KEYWORD"}:
            break
        op = tok.value
        prec = PRECEDENCE.get(op)
        if prec is None or prec < min_prec:
            break
        stream.advance()
        right = _parse_expr(stream, prec + 1)
        left = BinaryExpr(left, op, right)
    return left


def _parse_unary(stream: _Stream) -> Expr:
    tok = stream.peek()
    if tok.kind == "OP" and tok.value in {"-", "!"}:
        op = stream.advance().value
        return UnaryExpr(op, _parse_unary(stream))
    return _parse_postfix(stream)


def _parse_postfix(stream: _Stream) -> Expr:
    expr = _parse_primary(stream)
    while True:
        if stream.match("PUNCT", "("):
            args: list[Expr] = []
            if not stream.match("PUNCT", ")"):
                while True:
                    args.append(_parse_expr(stream))
                    if stream.match("PUNCT", ")"):
                        break
                    stream.expect("PUNCT", ",")
            expr = CallExpr(expr, args)
            continue
        if stream.match("PUNCT", "."):
            name = stream.expect("IDENT").value
            expr = AttrExpr(expr, name)
            continue
        break
    return expr


def _parse_primary(stream: _Stream) -> Expr:
    tok = stream.advance()
    if tok.kind == "NUMBER":
        if "." in tok.value:
            return NumberExpr(float(tok.value))
        return NumberExpr(int(tok.value))
    if tok.kind == "STRING":
        is_f = tok.value.startswith("f")
        value = tok.value[1:] if is_f else tok.value
        return StringExpr(value, is_f)
    if tok.kind == "IDENT":
        return NameExpr(tok.value)
    if tok.kind == "KEYWORD" and tok.value == "ask":
        arg = _parse_primary(stream)
        return CallExpr(NameExpr("ask"), [arg])
    if tok.kind == "KEYWORD" and tok.value == "yes":
        return BoolExpr(True)
    if tok.kind == "KEYWORD" and tok.value == "no":
        return BoolExpr(False)
    if tok.kind == "KEYWORD" and tok.value == "nil":
        return NilExpr()
    if tok.kind == "PUNCT" and tok.value == "(":
        expr = _parse_expr(stream)
        stream.expect("PUNCT", ")")
        return expr
    if tok.kind == "PUNCT" and tok.value == "[":
        elements: list[Expr] = []
        if not stream.match("PUNCT", "]"):
            while True:
                elements.append(_parse_expr(stream))
                if stream.match("PUNCT", "]"):
                    break
                stream.expect("PUNCT", ",")
        return ListExpr(elements)
    if tok.kind == "PUNCT" and tok.value == "{":
        entries: list[tuple[Expr, Expr]] = []
        if not stream.match("PUNCT", "}"):
            while True:
                k = _parse_expr(stream)
                stream.expect("PUNCT", ":")
                v = _parse_expr(stream)
                entries.append((k, v))
                if stream.match("PUNCT", "}"):
                    break
                stream.expect("PUNCT", ",")
        return DictExpr(entries)

    raise ValueError(f"Unexpected token {tok.kind}:{tok.value} at {tok.line}:{tok.column}")
