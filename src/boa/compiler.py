"""Boa compiler pipeline entry points."""

from __future__ import annotations

from dataclasses import asdict
import json
from pathlib import Path

from .parser import parse_source
from .runtime import eval_program
from .semantic import analyze


def compile_source(source: str) -> dict:
    program = parse_source(source)
    analyze(program)
    return asdict(program)


def check_source(source: str) -> None:
    program = parse_source(source)
    analyze(program)


def run_source(source: str) -> None:
    program = parse_source(source)
    analyze(program)
    eval_program(program)


def check_file(path: str | Path) -> None:
    source = Path(path).read_text(encoding="utf-8")
    check_source(source)


def run_file(path: str | Path) -> None:
    source = Path(path).read_text(encoding="utf-8")
    run_source(source)


def build_file(path: str | Path, output: str | Path) -> None:
    source = Path(path).read_text(encoding="utf-8")
    unit = compile_source(source)
    Path(output).write_text(json.dumps(unit, indent=2), encoding="utf-8")
