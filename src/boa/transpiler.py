"""Boa to Python transpilation."""

from __future__ import annotations

from dataclasses import dataclass
from pathlib import Path
import re

from .errors import BoaSyntaxError
from .parser import parse_source


TYPE_PATTERN = re.compile(r"\b(i|s|f|b)\b")


@dataclass(frozen=True)
class TypeHintToken:
    """Represents a compact Boa type hint token."""

    boa: str
    py: str


TYPE_MAP = {
    "i": "int",
    "s": "str",
    "f": "float",
    "b": "bool",
}


def _find_comment_start(line: str) -> int:
    """Find index where a real comment starts, respecting strings and len shorthand."""
    in_single = False
    in_double = False
    escaped = False
    for idx, ch in enumerate(line):
        if escaped:
            escaped = False
            continue
        if ch == "\\":
            escaped = True
            continue
        if ch == "'" and not in_double:
            in_single = not in_single
            continue
        if ch == '"' and not in_single:
            in_double = not in_double
            continue
        if ch == "#" and not in_single and not in_double:
            prev = line[idx - 1] if idx > 0 else ""
            nxt = line[idx + 1] if idx + 1 < len(line) else ""
            if nxt.isspace() or nxt == "":
                return idx
            if prev == "" or prev.isspace() or prev in "=([,{:+-*/%<>&|!~":
                continue
    return -1


def _replace_outside_strings(text: str, pattern: re.Pattern[str], repl: str) -> str:
    """Apply regex replacement while preserving string literal regions."""
    out: list[str] = []
    segment: list[str] = []
    in_single = False
    in_double = False
    escaped = False

    def flush_segment() -> None:
        if segment:
            out.append(pattern.sub(repl, "".join(segment)))
            segment.clear()

    for ch in text:
        if escaped:
            if in_single or in_double:
                out.append(ch)
            else:
                segment.append(ch)
            escaped = False
            continue

        if ch == "\\":
            if in_single or in_double:
                out.append(ch)
            else:
                segment.append(ch)
            escaped = True
            continue

        if ch == "'" and not in_double:
            if not in_single:
                flush_segment()
            in_single = not in_single
            out.append(ch)
            continue

        if ch == '"' and not in_single:
            if not in_double:
                flush_segment()
            in_double = not in_double
            out.append(ch)
            continue

        if in_single or in_double:
            out.append(ch)
        else:
            segment.append(ch)

    flush_segment()
    return "".join(out)


def _convert_range_sugar(expr: str) -> str:
    """Convert Boa range shorthand forms into Python range() calls."""
    expr = re.sub(r"\b([A-Za-z_][\w]*)\.\.([A-Za-z_][\w]*)\.\.([A-Za-z_][\w]*)\b", r"range(\1, \2, \3)", expr)
    expr = re.sub(r"\b(\d+)\.\.(\d+)\.\.(\d+)\b", r"range(\1, \2, \3)", expr)
    expr = re.sub(r"\b([A-Za-z_][\w]*)\.\.([A-Za-z_][\w]*)\b", r"range(\1, \2)", expr)
    expr = re.sub(r"\b(\d+)\.\.(\d+)\b", r"range(\1, \2)", expr)
    expr = re.sub(r"\.\.([A-Za-z_][\w]*)\b", r"range(\1)", expr)
    expr = re.sub(r"\.\.(\d+)\b", r"range(\1)", expr)
    return expr


def _convert_types(expr: str) -> str:
    """Convert compact Boa type hints to Python typing names."""
    expr = re.sub(r"\[([^\[\]]+)\]", r"list[\1]", expr)
    expr = re.sub(r"\{([^:{}]+):([^{}]+)\}", r"dict[\1,\2]", expr)
    expr = re.sub(r"\b([A-Za-z_][\w]*)\?(?!\w)", r"Optional[\1]", expr)

    def _map_simple(match: re.Match[str]) -> str:
        token = match.group(1)
        return TYPE_MAP.get(token, token)

    expr = TYPE_PATTERN.sub(_map_simple, expr)
    return expr


def _convert_type_annotations(line: str) -> str:
    """Convert only annotation contexts, avoiding runtime expression rewrites."""
    out = line
    out = re.sub(
        r"->\s*([^:]+)(:)",
        lambda m: f"-> {_convert_types(m.group(1).strip())}{m.group(2)}",
        out,
    )
    out = re.sub(
        r":\s*([^=,\)\n]+?)(?=\s*(?:=|,|\)|$))",
        lambda m: f": {_convert_types(m.group(1).strip())}",
        out,
    )
    return out


def _convert_len_shorthand(expr: str) -> str:
    """Convert #name style shorthand into len(name)."""
    return _replace_outside_strings(
        expr,
        re.compile(r"(?<!\w)#([A-Za-z_][\w]*)"),
        r"len(\1)",
    )


def _convert_self_shorthand(expr: str) -> str:
    """Convert Boa self shorthand in common method contexts."""
    out = re.sub(r"\bs(?=\.)", "self", expr)

    def _fix_params(match: re.Match[str]) -> str:
        params = match.group(1).split(",")
        fixed: list[str] = []
        for param in params:
            stripped = param.strip()
            if re.fullmatch(r"s(\s*(:.*)?)?", stripped):
                if ":" in stripped:
                    fixed.append(param.replace("s", "self", 1))
                else:
                    fixed.append(param.replace("s", "self", 1))
            else:
                fixed.append(param)
        return "(" + ",".join(fixed) + ")"

    out = re.sub(r"\(([^)]*)\)", _fix_params, out)
    out = re.sub(r"\breturn\s+s\b", "return self", out)
    return out


def _convert_keywords(expr: str) -> str:
    """Convert Boa keyword/operator shorthands to Python equivalents."""
    replacements = [
        (r"\bfn\b", "def"),
        (r"\bafn\b", "async def"),
        (r"\bret\b", "return"),
        (r"\bcls\b", "class"),
        (r"\bnil\b", "None"),
        (r"\byes\b", "True"),
        (r"\bno\b", "False"),
        (r"\\", "lambda "),
        (r"\bchk\b", "assert"),
        (r"\berr\b", "raise"),
        (r"\byld\b", "yield"),
        (r"\baw\b", "await"),
        (r"\bgbl\b", "global"),
        (r"\bnlo\b", "nonlocal"),
        (r"\bef\b", "elif"),
        (r"!==:", "is not"),
        (r"==:", "is"),
        (r"!~", "not in"),
        (r"(?<![=!])~", "in"),
        (r"&&", "and"),
        (r"\|\|", "or"),
    ]
    out = expr
    for pat, repl in replacements:
        out = _replace_outside_strings(out, re.compile(pat), repl)

    out = _replace_outside_strings(out, re.compile(r"(?<![=!<>])!(?!=)"), "not ")
    return out


def _convert_with_alias(expr: str) -> str:
    """Convert with X>Y into with X as Y."""
    return re.sub(r"\bwith\s+([^:>]+?)>([A-Za-z_][\w]*)", r"with \1 as \2", expr)


def _convert_use_statements(expr: str) -> str:
    """Convert use and use X: imports to Python import syntax."""
    stripped = expr.strip()
    if stripped.startswith("use ") and ":" in stripped and not stripped.startswith("use http"):
        prefix, tail = stripped.split(":", maxsplit=1)
        module = prefix.replace("use", "", 1).strip()
        if not module:
            return expr
        imports = tail.strip()
        if not imports:
            raise ValueError("missing imported names after 'use X:'")
        return expr.replace(stripped, f"from {module} import {imports}")
    if stripped.startswith("use "):
        module = stripped.replace("use", "", 1).strip()
        if not module:
            raise ValueError("missing module name after 'use'")
        return expr.replace(stripped, f"import {module}")
    return expr


def _convert_out_ask(expr: str) -> str:
    """Convert out/ask shorthand forms."""
    stripped = expr.strip()
    if stripped.startswith("out "):
        payload = stripped[4:]
        return expr.replace(stripped, f"print({payload})")
    return re.sub(r"(?<![\w\"'])\bask\s+([^\n]+)", r"input(\1)", expr)


def _normalize_blank_lines(lines: list[str]) -> list[str]:
    """Collapse repeated blank lines to at most one."""
    result: list[str] = []
    blank = False
    for line in lines:
        if line.strip() == "":
            if not blank:
                result.append("")
            blank = True
        else:
            result.append(line.rstrip())
            blank = False
    return result


def transpile_source(source: str, file_name: str = "<memory>") -> str:
    """Transpile Boa source text into Python source text."""
    parsed = parse_source(source)
    out_lines: list[str] = []
    in_triple: str | None = None

    for line in parsed.lines:
        raw = line.raw
        if in_triple is not None:
            out_lines.append(raw.rstrip())
            if in_triple in raw:
                in_triple = None
            continue

        if raw.count("\"\"\"") % 2 == 1:
            in_triple = "\"\"\""
            out_lines.append(raw.rstrip())
            continue
        if raw.count("'''") % 2 == 1:
            in_triple = "'''"
            out_lines.append(raw.rstrip())
            continue

        comment_index = _find_comment_start(raw)
        code_part = raw if comment_index < 0 else raw[:comment_index]
        comment_part = "" if comment_index < 0 else raw[comment_index:]

        if code_part.strip() == "":
            out_lines.append(raw.rstrip())
            continue

        try:
            transformed = code_part
            if transformed.strip() == "..":
                transformed = transformed.replace("..", "pass", 1)
                rebuilt = f"{transformed.rstrip()}{comment_part}"
                out_lines.append(rebuilt.rstrip())
                continue

            transformed = _convert_use_statements(transformed)
            transformed = _convert_with_alias(transformed)
            transformed = _convert_out_ask(transformed)
            transformed = _convert_range_sugar(transformed)
            transformed = _convert_keywords(transformed)
            transformed = _convert_self_shorthand(transformed)
            transformed = _convert_len_shorthand(transformed)
            transformed = _convert_type_annotations(transformed)
            transformed = re.sub(r"\bcatch\b", "except", transformed)
            transformed = re.sub(r"\bend\b", "finally", transformed)

            rebuilt = f"{transformed.rstrip()}{comment_part}"
            out_lines.append(rebuilt.rstrip())
        except Exception as exc:  # pragma: no cover
            raise BoaSyntaxError(file_name, line.number, line.raw, str(exc)) from exc

    return "\n".join(_normalize_blank_lines(out_lines)).strip() + "\n"


def transpile_file(path: str | Path) -> str:
    """Transpile a Boa file to Python source text."""
    source_path = Path(path)
    source = source_path.read_text(encoding="utf-8")
    return transpile_source(source, file_name=str(source_path))
