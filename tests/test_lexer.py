"""Tests for the standalone Boa lexer."""

from boa.lexer import tokenize


def test_tokenize_keywords_and_identifiers() -> None:
    tokens = tokenize("fn add(a: i, b: i):\n    ret a + b\n")
    kinds = [t.kind for t in tokens]
    assert "INDENT" in kinds
    assert "DEDENT" in kinds
    assert kinds[-1] == "EOF"


def test_tokenize_operators() -> None:
    tokens = tokenize("x = 1..5\ny = a && !b\n")
    values = [t.value for t in tokens if t.kind in {"OP", "KEYWORD"}]
    assert ".." in values
    assert "&&" in values
    assert "!" in values
