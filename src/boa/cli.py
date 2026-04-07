"""Boa command-line interface."""

from __future__ import annotations

import argparse
import shutil
from pathlib import Path
import stat
import sys

if __package__:
    from . import __version__
    from .compiler import build_file, check_file, run_file
    from .errors import BoaError
else:  # pragma: no cover - used when executed as a direct script/frozen entrypoint
    src_dir = Path(__file__).resolve().parents[1]
    if str(src_dir) not in sys.path:
        sys.path.insert(0, str(src_dir))
    from boa import __version__
    from boa.compiler import build_file, check_file, run_file
    from boa.errors import BoaError


def _build_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(prog="boa", description="Boa language CLI")
    sub = parser.add_subparsers(dest="command")

    run_p = sub.add_parser("run", help="Compile and execute a .boa file")
    run_p.add_argument("source", type=str)

    build_p = sub.add_parser("build", help="Compile a .boa file into .boac JSON IR")
    build_p.add_argument("source", type=str)
    build_p.add_argument("output", type=str, nargs="?")

    check_p = sub.add_parser("check", help="Parse and type-check a .boa file")
    check_p.add_argument("source", type=str)

    install_p = sub.add_parser(
        "install",
        help="Install the current Boa executable/script to a target path",
    )
    install_p.add_argument("destination", type=str)

    sub.add_parser("version", help="Print Boa version")
    sub.add_parser("help", help="Show usage")
    return parser


def _resolve_output(source: Path, output: str | None) -> Path:
    if output:
        return Path(output)
    return source.with_suffix(".boac")


def _cmd_build(source: Path, output: Path) -> int:
    build_file(source, output)
    print(f"Built {output}")
    return 0


def _cmd_check(source: Path) -> int:
    check_file(source)
    print(f"OK: {source}")
    return 0


def _cmd_run(source: Path) -> int:
    run_file(source)
    return 0


def _current_installable_path() -> Path:
    if getattr(sys, "frozen", False):
        return Path(sys.executable).resolve()
    argv0 = Path(sys.argv[0]).resolve()
    if argv0.exists():
        return argv0
    return Path(__file__).resolve()


def _resolve_install_destination(destination: Path, source_name: str) -> Path:
    if destination.exists() and destination.is_dir():
        return destination / source_name
    return destination


def _cmd_install(destination: Path) -> int:
    source = _current_installable_path()
    target = _resolve_install_destination(destination, source.name)
    target.parent.mkdir(parents=True, exist_ok=True)

    if source == target:
        print(f"Already installed at {target}")
        return 0

    shutil.copy2(source, target)
    if not sys.platform.startswith("win"):
        target.chmod(target.stat().st_mode | stat.S_IXUSR | stat.S_IXGRP)

    print(f"Installed {target}")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = _build_parser()
    args = parser.parse_args(argv)

    if args.command is None or args.command == "help":
        parser.print_help()
        return 0
    if args.command == "version":
        print(__version__)
        return 0

    try:
        if args.command == "install":
            return _cmd_install(Path(args.destination))

        source = Path(args.source)
        if not source.exists():
            print(f"File not found: {source}", file=sys.stderr)
            return 2

        if args.command == "build":
            output = _resolve_output(source, args.output)
            return _cmd_build(source, output)
        if args.command == "check":
            return _cmd_check(source)
        if args.command == "run":
            return _cmd_run(source)

        parser.print_help()
        return 1
    except (BoaError, ValueError) as exc:
        print(str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
