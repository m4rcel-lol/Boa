"""Tests for Boa lexer utilities."""

from boa.lexer import lex_lines, split_indent


def test_split_indent_spaces() -> None:
    indent, body = split_indent("    hello")
    assert indent == "    "
    assert body == "hello"


def test_split_indent_tabs() -> None:
    indent, body = split_indent("\t\tvalue")
    assert indent == "\t\t"
    assert body == "value"


def test_lex_lines_preserves_numbering() -> None:
    lines = lex_lines("a\n  b\n")
    assert [line.number for line in lines] == [1, 2]
    assert lines[1].indent == "  "
    assert lines[1].body == "b"
