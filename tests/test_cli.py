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


def test_cli_install_to_file_path(tmp_path: Path, monkeypatch) -> None:
    source = tmp_path / "boa-source"
    source.write_text("boa", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    destination = tmp_path / "bin" / "boa"
    code = main(["install", str(destination)])

    assert code == 0
    assert destination.exists()
    assert destination.read_text(encoding="utf-8") == "boa"


def test_cli_install_to_existing_directory(tmp_path: Path, monkeypatch) -> None:
    source = tmp_path / "boa-source"
    source.write_text("boa", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    destination_dir = tmp_path / "bin"
    destination_dir.mkdir()
    code = main(["install", str(destination_dir)])

    expected = destination_dir / source.name
    assert code == 0
    assert expected.exists()
    assert expected.read_text(encoding="utf-8") == "boa"


def test_cli_install_rejects_existing_file_without_force(
    tmp_path: Path, monkeypatch
) -> None:
    source = tmp_path / "boa-source"
    source.write_text("new", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    destination = tmp_path / "bin" / "boa"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text("old", encoding="utf-8")

    code = main(["install", str(destination)])
    assert code == 1
    assert destination.read_text(encoding="utf-8") == "old"


def test_cli_install_overwrites_existing_file_with_force(
    tmp_path: Path, monkeypatch
) -> None:
    source = tmp_path / "boa-source"
    source.write_text("new", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    destination = tmp_path / "bin" / "boa"
    destination.parent.mkdir(parents=True, exist_ok=True)
    destination.write_text("old", encoding="utf-8")

    code = main(["install", str(destination), "--force"])
    assert code == 0
    assert destination.read_text(encoding="utf-8") == "new"
