"""Tests for Boa parser."""

from boa.ast_nodes import FunctionDef, Program
from boa.parser import parse_source


def test_parse_function_definition() -> None:
    unit = parse_source("fn add(a: i, b: i) -> i:\n    ret a + b\n")
    assert isinstance(unit, Program)
    assert isinstance(unit.statements[0], FunctionDef)
    assert unit.statements[0].name == "add"


def test_parse_assignment_and_out() -> None:
    unit = parse_source("x: i = 1\nout x\n")
    assert len(unit.statements) == 2
