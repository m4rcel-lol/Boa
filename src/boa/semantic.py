"""Semantic analysis and lightweight type checks for Boa."""

from __future__ import annotations

from .ast_nodes import (
    AssignStmt,
    BoolExpr,
    ClassDef,
    DictExpr,
    FunctionDef,
    IfStmt,
    ListExpr,
    NameExpr,
    NilExpr,
    NumberExpr,
    Program,
    ReturnStmt,
    StringExpr,
)


BUILTIN_TYPES = {"i", "s", "f", "b", "[i]", "[s]", "[f]", "[b]", "{s:i}", "{s:s}"}


class SemanticError(ValueError):
    pass


def _is_valid_type_name(name: str) -> bool:
    if not name:
        return False
    if name in BUILTIN_TYPES:
        return True
    if name.endswith("?"):
        return _is_valid_type_name(name[:-1])
    if "|" in name:
        return all(_is_valid_type_name(part.strip()) for part in name.split("|"))
    if name.startswith("[") and name.endswith("]"):
        return _is_valid_type_name(name[1:-1].strip())
    if name.startswith("{") and name.endswith("}") and ":" in name:
        left, right = name[1:-1].split(":", maxsplit=1)
        return _is_valid_type_name(left.strip()) and _is_valid_type_name(right.strip())
    return name.isidentifier()


def _literal_type(expr) -> str | None:
    if isinstance(expr, StringExpr):
        return "s"
    if isinstance(expr, BoolExpr):
        return "b"
    if isinstance(expr, NumberExpr):
        return "f" if isinstance(expr.value, float) else "i"
    if isinstance(expr, ListExpr):
        return "[any]"
    if isinstance(expr, DictExpr):
        return "{any:any}"
    if isinstance(expr, NilExpr):
        return "nil"
    if isinstance(expr, NameExpr):
        return None
    return None


def analyze(program: Program) -> None:
    symbols: set[str] = set()

    def walk(stmts, fn_depth: int = 0) -> None:
        for stmt in stmts:
            if isinstance(stmt, FunctionDef):
                if stmt.name in symbols:
                    raise SemanticError(f"Duplicate symbol '{stmt.name}'")
                symbols.add(stmt.name)
                for p in stmt.params:
                    if p.annotation and not _is_valid_type_name(p.annotation):
                        raise SemanticError(f"Invalid parameter type '{p.annotation}'")
                if stmt.return_annotation and not _is_valid_type_name(stmt.return_annotation):
                    raise SemanticError(f"Invalid return type '{stmt.return_annotation}'")
                walk(stmt.body, fn_depth + 1)
            elif isinstance(stmt, ClassDef):
                if stmt.name in symbols:
                    raise SemanticError(f"Duplicate symbol '{stmt.name}'")
                symbols.add(stmt.name)
                walk(stmt.body, fn_depth)
            elif isinstance(stmt, ReturnStmt):
                if fn_depth == 0:
                    raise SemanticError("'ret' used outside function")
            elif isinstance(stmt, AssignStmt):
                if stmt.annotation and not _is_valid_type_name(stmt.annotation):
                    raise SemanticError(f"Invalid annotation '{stmt.annotation}'")
                if stmt.annotation:
                    lt = _literal_type(stmt.value)
                    if lt == "nil":
                        continue
                    if lt in {"i", "s", "f", "b"} and stmt.annotation not in {lt, f"{lt}?"}:
                        raise SemanticError(
                            f"Type mismatch for '{stmt.name}': expected {stmt.annotation}, got {lt}"
                        )
            elif isinstance(stmt, IfStmt):
                walk(stmt.body, fn_depth)
                for _, body in stmt.elif_blocks:
                    walk(body, fn_depth)
                if stmt.else_body is not None:
                    walk(stmt.else_body, fn_depth)

    walk(program.statements)
