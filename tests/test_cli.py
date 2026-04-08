"""CLI smoke tests."""

from __future__ import annotations

from pathlib import Path
import stat
import subprocess
import sys

import pytest

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


def test_cli_install_to_new_path(tmp_path: Path, monkeypatch: pytest.MonkeyPatch) -> None:
    source = tmp_path / "boa-source"
    source.write_text("boa", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    destination = tmp_path / "bin" / "boa"
    code = main(["install", str(destination)])

    assert code == 0
    assert destination.exists()
    assert destination.read_text(encoding="utf-8") == "boa"


def test_cli_install_to_existing_directory(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
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
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
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
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
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


def test_cli_install_handles_permission_error(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch, capsys: pytest.CaptureFixture[str]
) -> None:
    source = tmp_path / "boa-source"
    source.write_text("new", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    def raise_permission_error(*_args: object, **_kwargs: object) -> None:
        raise PermissionError("Access denied")

    monkeypatch.setattr("boa.cli.shutil.copy2", raise_permission_error)

    destination = tmp_path / "protected" / "boa"
    code = main(["install", str(destination), "--force"])
    captured = capsys.readouterr()

    assert code == 1
    assert "Access denied" in captured.err


@pytest.mark.skipif(sys.platform.startswith("win"), reason="POSIX mode bits only")
def test_cli_install_sets_user_executable_bit(
    tmp_path: Path, monkeypatch: pytest.MonkeyPatch
) -> None:
    source = tmp_path / "boa-source"
    source.write_text("boa", encoding="utf-8")

    monkeypatch.setattr("boa.cli._current_installable_path", lambda: source)
    destination = tmp_path / "bin" / "boa"
    code = main(["install", str(destination)])

    assert code == 0
    assert destination.stat().st_mode & stat.S_IXUSR
