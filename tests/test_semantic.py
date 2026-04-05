"""Tests for semantic analysis."""

import pytest

from boa.parser import parse_source
from boa.semantic import SemanticError, analyze


def test_reject_return_outside_function() -> None:
    unit = parse_source("ret 1\n")
    with pytest.raises(SemanticError):
        analyze(unit)


def test_reject_bad_type_annotation() -> None:
    unit = parse_source("x: ?? = 1\n")
    with pytest.raises(SemanticError):
        analyze(unit)


def test_accept_valid_program() -> None:
    unit = parse_source("fn a(x: i) -> i:\n    ret x\n")
    analyze(unit)
