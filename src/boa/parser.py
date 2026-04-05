"""Parser façade for Boa v1 transpilation pipeline."""

from __future__ import annotations

from dataclasses import dataclass

from .lexer import BoaLine, lex_lines


@dataclass(frozen=True)
class ParsedBoa:
    """Parsed representation for Boa v1 (line-oriented)."""

    lines: list[BoaLine]


def parse_source(source: str) -> ParsedBoa:
    """Parse Boa source into a line-preserving intermediate form."""
    return ParsedBoa(lines=lex_lines(source))
