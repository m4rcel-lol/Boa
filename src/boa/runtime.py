"""Boa-native interpreter runtime."""

from __future__ import annotations

from dataclasses import dataclass
import re
from typing import Any

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
    PassStmt,
    Program,
    ReturnStmt,
    StringExpr,
    UnaryExpr,
    UseStmt,
)


class RuntimeErrorBoa(RuntimeError):
    pass


class _ReturnSignal(Exception):
    def __init__(self, value: Any) -> None:
        self.value = value


class Env:
    def __init__(self, parent: Env | None = None) -> None:
        self.parent = parent
        self.values: dict[str, Any] = {}

    def get(self, name: str) -> Any:
        if name in self.values:
            return self.values[name]
        if self.parent is not None:
            return self.parent.get(name)
        raise RuntimeErrorBoa(f"Unknown symbol '{name}'")

    def set(self, name: str, value: Any) -> None:
        self.values[name] = value


@dataclass
class BoaFunction:
    name: str
    params: list[str]
    body: list
    closure: Env


@dataclass
class BoaClass:
    name: str
    methods: dict[str, BoaFunction]


@dataclass
class BoaInstance:
    cls: BoaClass
    fields: dict[str, Any]


def _boa_range(*args: Any) -> list[int]:
    ints = [int(a) for a in args]
    return list(range(*ints))


def _eval_fstring(value: str, env: Env) -> str:
    def repl(match: re.Match[str]) -> str:
        name = match.group(1).strip()
        return str(env.get(name))

    return re.sub(r"\{([A-Za-z_][A-Za-z0-9_]*)\}", repl, value)


def _truthy(value: Any) -> bool:
    return bool(value)


def eval_program(program: Program, env: Env | None = None) -> Env:
    runtime = env or Env()
    runtime.set("ask", input)
    runtime.set("len", len)
    runtime.set("range", _boa_range)
    _exec_block(program.statements, runtime)
    return runtime


def _exec_block(stmts: list, env: Env) -> None:
    for stmt in stmts:
        _exec_stmt(stmt, env)


def _exec_stmt(stmt, env: Env) -> None:
    if isinstance(stmt, UseStmt):
        return
    if isinstance(stmt, FunctionDef):
        env.set(stmt.name, BoaFunction(stmt.name, [p.name for p in stmt.params], stmt.body, env))
        return
    if isinstance(stmt, ClassDef):
        methods: dict[str, BoaFunction] = {}
        temp = Env(env)
        for child in stmt.body:
            if isinstance(child, FunctionDef):
                fn = BoaFunction(child.name, [p.name for p in child.params], child.body, temp)
                methods[child.name] = fn
        env.set(stmt.name, BoaClass(stmt.name, methods))
        return
    if isinstance(stmt, ReturnStmt):
        value = None if stmt.value is None else _eval_expr(stmt.value, env)
        raise _ReturnSignal(value)
    if isinstance(stmt, AssignStmt):
        env.set(stmt.name, _eval_expr(stmt.value, env))
        return
    if isinstance(stmt, ExprStmt):
        _eval_expr(stmt.expr, env)
        return
    if isinstance(stmt, OutStmt):
        print(_eval_expr(stmt.expr, env))
        return
    if isinstance(stmt, IfStmt):
        if _truthy(_eval_expr(stmt.condition, env)):
            _exec_block(stmt.body, Env(env))
            return
        for cond, block in stmt.elif_blocks:
            if _truthy(_eval_expr(cond, env)):
                _exec_block(block, Env(env))
                return
        if stmt.else_body is not None:
            _exec_block(stmt.else_body, Env(env))
        return
    if isinstance(stmt, ForStmt):
        iterable = _eval_expr(stmt.iterable, env)
        for item in iterable:
            inner = Env(env)
            inner.set(stmt.var_name, item)
            _exec_block(stmt.body, inner)
        return
    if isinstance(stmt, PassStmt):
        return

    raise RuntimeErrorBoa(f"Unsupported statement {type(stmt).__name__}")


def _call_function(fn: BoaFunction, args: list[Any], bound_self: Any | None = None) -> Any:
    local = Env(fn.closure)
    params = fn.params
    if bound_self is not None:
        if not params:
            raise RuntimeErrorBoa("Method missing self parameter")
        local.set(params[0], bound_self)
        params = params[1:]
    if len(args) != len(params):
        raise RuntimeErrorBoa(f"{fn.name} expects {len(params)} args, got {len(args)}")
    for name, value in zip(params, args):
        local.set(name, value)
    try:
        _exec_block(fn.body, local)
    except _ReturnSignal as rs:
        return rs.value
    return None


def _eval_binary(op: str, left: Any, right: Any) -> Any:
    if op == "+":
        return left + right
    if op == "-":
        return left - right
    if op == "*":
        return left * right
    if op == "/":
        return left / right
    if op == "%":
        return left % right
    if op == "==":
        return left == right
    if op == "!=":
        return left != right
    if op == "<":
        return left < right
    if op == ">":
        return left > right
    if op == "<=":
        return left <= right
    if op == ">=":
        return left >= right
    if op == "==:":
        return left is right
    if op == "!==:":
        return left is not right
    if op == "~":
        return left in right
    if op == "!~":
        return left not in right
    if op == "&&":
        return _truthy(left) and _truthy(right)
    if op == "||":
        return _truthy(left) or _truthy(right)
    raise RuntimeErrorBoa(f"Unsupported operator '{op}'")


def _eval_expr(expr: Expr, env: Env) -> Any:
    if isinstance(expr, NameExpr):
        return env.get(expr.name)
    if isinstance(expr, NumberExpr):
        return expr.value
    if isinstance(expr, StringExpr):
        return _eval_fstring(expr.value, env) if expr.is_fstring else expr.value
    if isinstance(expr, BoolExpr):
        return expr.value
    if isinstance(expr, NilExpr):
        return None
    if isinstance(expr, ListExpr):
        return [_eval_expr(e, env) for e in expr.elements]
    if isinstance(expr, DictExpr):
        return {_eval_expr(k, env): _eval_expr(v, env) for k, v in expr.entries}
    if isinstance(expr, UnaryExpr):
        v = _eval_expr(expr.expr, env)
        if expr.op == "-":
            return -v
        if expr.op == "!":
            return not _truthy(v)
        raise RuntimeErrorBoa(f"Unsupported unary operator '{expr.op}'")
    if isinstance(expr, BinaryExpr):
        if expr.op == "&&":
            left = _eval_expr(expr.left, env)
            return _truthy(left) and _truthy(_eval_expr(expr.right, env))
        if expr.op == "||":
            left = _eval_expr(expr.left, env)
            return _truthy(left) or _truthy(_eval_expr(expr.right, env))
        return _eval_binary(expr.op, _eval_expr(expr.left, env), _eval_expr(expr.right, env))
    if isinstance(expr, AttrExpr):
        target = _eval_expr(expr.target, env)
        if isinstance(target, BoaInstance):
            if expr.name in target.fields:
                return target.fields[expr.name]
            if expr.name in target.cls.methods:
                fn = target.cls.methods[expr.name]
                return lambda *args: _call_function(fn, list(args), bound_self=target)
            raise RuntimeErrorBoa(f"Unknown member '{expr.name}'")
        raise RuntimeErrorBoa("Attribute access supported only on instances")
    if isinstance(expr, CallExpr):
        callee = _eval_expr(expr.func, env)
        args = [_eval_expr(a, env) for a in expr.args]
        if isinstance(callee, BoaFunction):
            return _call_function(callee, args)
        if isinstance(callee, BoaClass):
            inst = BoaInstance(callee, {})
            init = callee.methods.get("__init__")
            if init is not None:
                _call_function(init, args, bound_self=inst)
            return inst
        if callable(callee):
            return callee(*args)
        raise RuntimeErrorBoa("Attempted to call non-callable value")

    raise RuntimeErrorBoa(f"Unsupported expression {type(expr).__name__}")
