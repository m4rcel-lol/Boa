"""Lightweight line-aware lexer support for Boa."""

from __future__ import annotations

from dataclasses import dataclass


@dataclass(frozen=True)
class BoaLine:
    """A single source line split into indentation and body text."""

    number: int
    indent: str
    body: str
    raw: str


def split_indent(text: str) -> tuple[str, str]:
    """Split a line into leading indentation and remainder."""
    index = 0
    while index < len(text) and text[index] in (" ", "\t"):
        index += 1
    return text[:index], text[index:]


def lex_lines(source: str) -> list[BoaLine]:
    """Tokenize source into indentation-preserving line records."""
    result: list[BoaLine] = []
    for idx, raw_line in enumerate(source.splitlines(), start=1):
        indent, body = split_indent(raw_line)
        result.append(BoaLine(number=idx, indent=indent, body=body, raw=raw_line))
    return result
