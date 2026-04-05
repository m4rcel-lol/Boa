"""CLI smoke tests."""

from __future__ import annotations

from pathlib import Path

from boa.cli import main


def test_cli_check_ok(tmp_path: Path) -> None:
    src = tmp_path / "ok.boa"
    src.write_text("x: i = 1\n", encoding="utf-8")
    code = main(["check", str(src)])
    assert code == 0


def test_cli_build_emits_boac(tmp_path: Path) -> None:
    src = tmp_path / "ok.boa"
    src.write_text("x: i = 1\n", encoding="utf-8")
    out = tmp_path / "ok.boac"
    code = main(["build", str(src), str(out)])
    assert code == 0
    assert out.exists()
