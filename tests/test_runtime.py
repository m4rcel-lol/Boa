"""Tests for Boa interpreter runtime."""

from __future__ import annotations

from io import StringIO
from pathlib import Path
import sys

from boa.compiler import run_source


def _capture_output(source: str) -> str:
    old = sys.stdout
    buf = StringIO()
    sys.stdout = buf
    try:
        run_source(source)
    finally:
        sys.stdout = old
    return buf.getvalue()


def test_run_function_and_out() -> None:
    out = _capture_output("fn add(a: i, b: i) -> i:\n    ret a + b\nout add(2, 3)\n")
    assert out.strip() == "5"


def test_if_else_and_boolean_ops() -> None:
    src = "x: b = yes\nif x && !no:\n    out \"ok\"\nelse:\n    out \"bad\"\n"
    out = _capture_output(src)
    assert out.strip() == "ok"


def test_fstring_interpolation() -> None:
    src = "name: s = \"Boa\"\nout f\"Hello, {name}!\"\n"
    out = _capture_output(src)
    assert out.strip() == "Hello, Boa!"


def test_run_hello_sample(monkeypatch) -> None:
    sample = Path(__file__).resolve().parent / "samples" / "hello.boa"
    monkeypatch.setattr("builtins.input", lambda _prompt="": "Boa")
    out = _capture_output(sample.read_text(encoding="utf-8"))
    assert out.strip().splitlines() == [
        "Hello, Boa!",
        "5",
        "2",
        "4",
        "6",
        "8",
        "10",
    ]
