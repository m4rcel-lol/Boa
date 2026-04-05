"""AST node definitions for the Boa language."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class Program:
    statements: list[Stmt]


class Stmt:
    pass


class Expr:
    pass


@dataclass(frozen=True)
class UseStmt(Stmt):
    module: str
    names: list[str] | None = None


@dataclass(frozen=True)
class Param:
    name: str
    annotation: str | None = None


@dataclass(frozen=True)
class FunctionDef(Stmt):
    name: str
    params: list[Param]
    return_annotation: str | None
    body: list[Stmt]


@dataclass(frozen=True)
class ClassDef(Stmt):
    name: str
    base_name: str | None
    body: list[Stmt]


@dataclass(frozen=True)
class ReturnStmt(Stmt):
    value: Expr | None


@dataclass(frozen=True)
class AssignStmt(Stmt):
    name: str
    annotation: str | None
    value: Expr


@dataclass(frozen=True)
class ExprStmt(Stmt):
    expr: Expr


@dataclass(frozen=True)
class OutStmt(Stmt):
    expr: Expr


@dataclass(frozen=True)
class IfStmt(Stmt):
    condition: Expr
    body: list[Stmt]
    elif_blocks: list[tuple[Expr, list[Stmt]]]
    else_body: list[Stmt] | None


@dataclass(frozen=True)
class ForStmt(Stmt):
    var_name: str
    iterable: Expr
    body: list[Stmt]


@dataclass(frozen=True)
class PassStmt(Stmt):
    pass


@dataclass(frozen=True)
class NameExpr(Expr):
    name: str


@dataclass(frozen=True)
class NumberExpr(Expr):
    value: int | float


@dataclass(frozen=True)
class StringExpr(Expr):
    value: str
    is_fstring: bool = False


@dataclass(frozen=True)
class BoolExpr(Expr):
    value: bool


@dataclass(frozen=True)
class NilExpr(Expr):
    pass


@dataclass(frozen=True)
class ListExpr(Expr):
    elements: list[Expr]


@dataclass(frozen=True)
class DictExpr(Expr):
    entries: list[tuple[Expr, Expr]]


@dataclass(frozen=True)
class UnaryExpr(Expr):
    op: str
    expr: Expr


@dataclass(frozen=True)
class BinaryExpr(Expr):
    left: Expr
    op: str
    right: Expr


@dataclass(frozen=True)
class CallExpr(Expr):
    func: Expr
    args: list[Expr]


@dataclass(frozen=True)
class AttrExpr(Expr):
    target: Expr
    name: str
