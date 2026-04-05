"""Boa command-line interface."""

from __future__ import annotations

import argparse
from pathlib import Path
import runpy
import sys
import tempfile

from . import __version__
from .errors import BoaError
from .transpiler import transpile_file


def _build_parser() -> argparse.ArgumentParser:
    """Create and return the CLI argument parser."""
    parser = argparse.ArgumentParser(prog="boa", description="Boa language CLI")
    sub = parser.add_subparsers(dest="command")

    run_p = sub.add_parser("run", help="Transpile and execute a .boa file")
    run_p.add_argument("source", type=str)

    build_p = sub.add_parser("build", help="Transpile a .boa file into .py")
    build_p.add_argument("source", type=str)
    build_p.add_argument("output", type=str, nargs="?")

    check_p = sub.add_parser("check", help="Syntax check a .boa file")
    check_p.add_argument("source", type=str)

    sub.add_parser("version", help="Print Boa version")
    sub.add_parser("help", help="Show usage")
    return parser


def _resolve_output(source: Path, output: str | None) -> Path:
    """Resolve output path for build command."""
    if output:
        return Path(output)
    return source.with_suffix(".py")


def _cmd_build(source: Path, output: Path) -> int:
    """Handle `boa build` command."""
    py_code = transpile_file(source)
    output.write_text(py_code, encoding="utf-8")
    print(f"Built {output}")
    return 0


def _cmd_check(source: Path) -> int:
    """Handle `boa check` command."""
    transpile_file(source)
    print(f"OK: {source}")
    return 0


def _cmd_run(source: Path) -> int:
    """Handle `boa run` command."""
    py_code = transpile_file(source)
    with tempfile.TemporaryDirectory(prefix="boa-") as tmp_dir:
        out = Path(tmp_dir) / f"{source.stem}.py"
        out.write_text(py_code, encoding="utf-8")
        runpy.run_path(str(out), run_name="__main__")
    return 0


def main(argv: list[str] | None = None) -> int:
    """Run Boa CLI and return process exit code."""
    parser = _build_parser()
    args = parser.parse_args(argv)

    if args.command is None or args.command == "help":
        parser.print_help()
        return 0
    if args.command == "version":
        print(__version__)
        return 0

    try:
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
    except BoaError as exc:
        print(str(exc), file=sys.stderr)
        return 1


if __name__ == "__main__":
    raise SystemExit(main())
