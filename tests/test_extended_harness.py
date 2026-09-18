"""Executor/validator regression tests; these do not replace compiler tests."""
from pathlib import Path
from subprocess import CompletedProcess

import pytest

from extended_cases import blocks, check_outputs, constant_argument, constant_offsets, execute
from run_tests import CASES, run_case, write_reports


def case(number):
    return next(item for item in CASES if item.id == number)


def test_case_ids():
    assert [item.id for item in CASES] == list(range(1, 46))


@pytest.mark.parametrize("body", ["%entry:\n%0 = add 1, 2", "%entry:\nret 0\n%0 = add 1, 2", "%entry:\njump %missing"])
def test_cfg_rejects_invalid_blocks(body):
    with pytest.raises(AssertionError):
        blocks(body)


def test_cfg_accepts_constant_branch():
    graph = blocks("%entry:\nbr 0, %a, %b\n%a:\nret 1\n%b:\nret 0")
    assert len(graph) == 3


def test_ir_cannot_satisfy_assembly_checks():
    ir = "fun @half(@x: f32): f32 {\n%entry:\nret @x\n}\nfun @main(): i32 {\n%entry:\n%0 = call @half(7)\nret 0\n}\n"
    with pytest.raises(AssertionError, match="缺少模式"):
        check_outputs(39, ir, "main:\nret\n")


@pytest.mark.parametrize("returncode,stderr", [(0, "syntax error"), (-6, "syntax error"), (1, "syntax error: Assertion failed")])
def test_invalid_source_must_not_pass_on_crash(monkeypatch, tmp_path, returncode, stderr):
    monkeypatch.setattr("extended_cases.subprocess.run", lambda *a, **kw: CompletedProcess(a[0], returncode, "", stderr))
    with pytest.raises(AssertionError):
        execute(case(42), Path("compiler"), tmp_path)


def test_invalid_source_clean_rejection(monkeypatch, tmp_path):
    monkeypatch.setattr("extended_cases.subprocess.run", lambda *a, **kw: CompletedProcess(a[0], 1, "", "syntax error"))
    execute(case(42), Path("compiler"), tmp_path)


@pytest.mark.parametrize("number", [43, 44, 45])
def test_cli_sequences(monkeypatch, tmp_path, number):
    calls = []

    def fake_run(args, **kwargs):
        calls.append(args)
        cwd = Path(kwargs["cwd"])
        source_path = Path(args[args.index("-S") + 1]) if "-S" in args else Path(args[-1])
        source = source_path.read_text(encoding="utf-8")
        first = "first" in source
        value = 1 if first else 2 if number == 45 else 7
        ir = ("fun @first(): i32 {\n%entry:\nret 1\n}\n" if first else "")
        ir += "fun @main(): i32 {\n%entry:\n" + ("%0 = call @first()\n" if first else "") + f"ret {value}\n}}\n"
        (cwd / "parse.kp").write_text(ir, encoding="utf-8")
        if "-S" in args:
            output = Path(args[args.index("-o") + 1])
            output.write_text(("first:\nret\n" if first else "") + "main:\n" + ("bl first\n" if first else "") + "ret\n", encoding="utf-8")
        return CompletedProcess(args, 0, "", "")

    monkeypatch.setattr("extended_cases.subprocess.run", fake_run)
    execute(case(number), Path("compiler"), tmp_path)
    if number == 43:
        assert "-S" not in calls[0] and "-O2" in calls[1]
        assert calls[1][-1].endswith("named-output.s")
    elif number == 44:
        assert calls[0][2].endswith("input with space.sy")
        assert calls[0][-1].endswith("output with space.s")
    else:
        assert len(calls) == 2
        assert (tmp_path / "first.kp").exists() and (tmp_path / "second.kp").exists()


def test_failure_keeps_diagnostics_and_snapshots(monkeypatch, tmp_path):
    monkeypatch.setattr("extended_cases.subprocess.run", lambda *a, **kw: CompletedProcess(a[0], -6, "", "assertion"))
    result = run_case(case(42), Path("compiler"), tmp_path)
    assert result.status == "FAIL"
    assert Path(result.artifacts, "invalid.diagnostics.json").exists()
    assert Path(result.artifacts, "invalid.sy").exists()


def test_empty_report(tmp_path):
    write_reports([], tmp_path)
    assert (tmp_path / "results.json").read_text(encoding="utf-8") == "[]"


def test_frontend_constant_copies():
    assert constant_argument("%3= add 0, 4\n", "%3") == 4
    assert constant_argument("", "3") == 3
    assert constant_argument("%3= call @getint()", "%3") is None


def test_array_offsets_track_materialization_and_reject_wrong_stride():
    assert 4092 in constant_offsets("movz x8, #1023\nlsl x8, x8, #2\nadd x9, x28, x8\n")
    assert 4092 in constant_offsets("movz x8, #4092\nadd x9, x28, x8\n")
    assert 4092 not in constant_offsets("movz x8, #1023\nlsl x8, x8, #3\nadd x9, x28, x8\n")
    assert 4092 not in constant_offsets("movz x8, #4092\nldr x8, [sp]\nadd x9, x28, x8\n")


@pytest.mark.parametrize("bypass_calls_side", [False, True])
def test_short_circuit_rejects_side_on_bypass_path(bypass_calls_side):
    ir = "fun @main(): i32 {\n%entry:\n%0 = eq 0, 0\nbr %0, %and1part1, %and2part1\n%and1part1:\n%1 = call @side()\n%2 = and %0, %1\njump %and2part1\n%and2part1:\nbr %0, %or1part1, %or2part1\n%or2part1:\n%3 = call @side()\njump %or1part1\n%or1part1:\nret 0\n}\n"
    if bypass_calls_side:
        ir = ir.replace("br %0, %and1part1, %and2part1", "br %0, %and1part1, %and1part1")
    asm = "main:\nb.eq next\nbl side\nret\n"
    if bypass_calls_side:
        with pytest.raises(AssertionError, match="短路路径"):
            check_outputs(31, ir, asm)
    else:
        check_outputs(31, ir, asm)
