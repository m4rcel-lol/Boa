"""Boa command-line interface."""

from __future__ import annotations

import argparse
from pathlib import Path
import sys

if __package__:
    from . import __version__
    from .compiler import build_file, check_file, run_file
    from .errors import BoaError
else:  # pragma: no cover - used when executed as a direct script/frozen entrypoint
    package_root = Path(__file__).resolve().parents[1]
    if str(package_root) not in sys.path:
        sys.path.insert(0, str(package_root))
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
