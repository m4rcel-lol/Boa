"""CLI smoke tests."""

from __future__ import annotations

from pathlib import Path
import subprocess
import sys

from boa import __version__
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


def test_cli_script_mode_version() -> None:
    cli_path = Path(__file__).resolve().parents[1] / "src" / "boa" / "cli.py"
    result = subprocess.run(
        [sys.executable, str(cli_path), "version"],
        capture_output=True,
        text=True,
        check=False,
    )
    assert result.returncode == 0
    assert result.stdout.strip() == __version__
