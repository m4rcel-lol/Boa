"""Tests for Boa transpiler."""

from __future__ import annotations

from pathlib import Path

from boa.transpiler import transpile_file, transpile_source


def test_use_import() -> None:
    assert transpile_source("use os\n") == "import os\n"


def test_use_from_import() -> None:
    py = transpile_source("use pathlib: Path\n")
    assert py == "from pathlib import Path\n"


def test_fn_and_ret() -> None:
    py = transpile_source("fn x() -> i:\n    ret 1\n")
    assert "def x()" in py
    assert "return 1" in py


def test_afn_and_await() -> None:
    py = transpile_source("afn x() -> s:\n    aw y()\n")
    assert "async def x()" in py
    assert "await y()" in py


def test_class_and_self() -> None:
    py = transpile_source("cls A:\n    fn m(s):\n        ret s\n")
    assert "class A:" in py
    assert "def m(self):" in py


def test_none_true_false() -> None:
    py = transpile_source("x = nil\ny = yes\nz = no\n")
    assert "x = None" in py
    assert "y = True" in py
    assert "z = False" in py


def test_bool_operators() -> None:
    py = transpile_source("x = a && b || !c\n")
    assert "a and b or not c" in py


def test_in_and_not_in() -> None:
    py = transpile_source("x = a ~ b\ny = a !~ b\n")
    assert "a in b" in py
    assert "a not in b" in py


def test_is_and_is_not() -> None:
    py = transpile_source("x = a ==: b\ny = a !==: b\n")
    assert "a is b" in py
    assert "a is not b" in py


def test_pass_and_raise_assert_yield() -> None:
    py = transpile_source("fn x():\n    ..\n    chk 1\n    err ValueError()\n    yld 2\n")
    assert "pass" in py
    assert "assert 1" in py
    assert "raise ValueError()" in py
    assert "yield 2" in py


def test_global_nonlocal() -> None:
    py = transpile_source("gbl a\nnlo b\n")
    assert "global a" in py
    assert "nonlocal b" in py


def test_with_alias() -> None:
    py = transpile_source("with open('x')>f:\n    out f.read()\n")
    assert "with open('x') as f:" in py


def test_try_catch_end() -> None:
    py = transpile_source("try:\n    out 1\ncatch Exception:\n    out 2\nend:\n    out 3\n")
    assert "except Exception:" in py
    assert "finally:" in py


def test_elif_keyword() -> None:
    py = transpile_source("if x:\n    out 1\nef y:\n    out 2\n")
    assert "elif y:" in py


def test_out_and_ask() -> None:
    py = transpile_source("x = ask \"name\"\nout x\n")
    assert "x = input(\"name\")" in py
    assert "print(x)" in py


def test_len_shorthand() -> None:
    py = transpile_source("out #nums\n")
    assert "print(len(nums))" in py


def test_comment_preserved_and_not_len() -> None:
    py = transpile_source("# just comment\nx = 1  # inline\n")
    assert "# just comment" in py
    assert "# inline" in py


def test_string_hash_untouched() -> None:
    py = transpile_source("out \"#not_comment\"\n")
    assert 'print("#not_comment")' in py


def test_range_one_arg() -> None:
    py = transpile_source("for x ~ ..5:\n    out x\n")
    assert "for x in range(5):" in py


def test_range_two_arg() -> None:
    py = transpile_source("for x ~ 1..5:\n    out x\n")
    assert "for x in range(1, 5):" in py


def test_range_three_arg() -> None:
    py = transpile_source("for x ~ 1..5..2:\n    out x\n")
    assert "for x in range(1, 5, 2):" in py


def test_type_hints_simple() -> None:
    py = transpile_source("fn a(x: i, y: s) -> b:\n    ret yes\n")
    assert "x: int" in py
    assert "y: str" in py
    assert "-> bool" in py


def test_type_hints_compound() -> None:
    py = transpile_source("mapping: {s:i} = {}\nmaybe: s? = nil\n")
    assert "mapping: dict[str,int]" in py
    assert "maybe: Optional[str]" in py


def test_multiline_string_preserved() -> None:
    src = 'x = """line1\nline2\n"""\nout x\n'
    py = transpile_source(src)
    assert '"""line1' in py
    assert "print(x)" in py


def test_blank_lines_collapsed() -> None:
    py = transpile_source("out 1\n\n\n\nout 2\n")
    assert "\n\n\n" not in py


def test_sample_files_transpile() -> None:
    base = Path(__file__).parent / "samples"
    for name in ["hello.boa", "classes.boa", "async_example.boa", "types.boa"]:
        py = transpile_file(base / name)
        assert py.strip() != ""
