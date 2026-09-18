"""Pytest entry point for all data-driven compiler cases (TC01–TC46)."""

from __future__ import annotations

from pathlib import Path

import pytest

from run_tests import CASES, Result, run_case, write_reports


@pytest.fixture(scope="session")
def case_results(request: pytest.FixtureRequest):
    values: list[Result] = []
    def finalize() -> None:
        write_reports(values, Path(request.config.getoption("--report-dir")).expanduser().resolve())
    request.addfinalizer(finalize)
    return values


@pytest.mark.parametrize(
    "case",
    [pytest.param(case, marks=pytest.mark.extended if case.id >= 31 else pytest.mark.baseline)
     for case in CASES],
    ids=lambda item: f"TC{item.id:02d}-{item.title}",
)
def test_compiler_case(case, compiler_path, request, case_results):
    """Compile one SysY sample and statically check IR/ARM text.

    Parameterization is the data-driven core: adding a case only requires one
    Case record, while execution, isolation and assertions remain shared.
    """
    keep = request.config.getoption("--report-dir") if request.config.getoption("--keep-artifacts") else None
    artifact_root = None
    if keep:
        from pathlib import Path
        artifact_root = Path(keep).resolve() / "artifacts"
    result = run_case(case, compiler_path, artifact_root)
    case_results.append(result)
    if result.status == "BLOCKED":
        pytest.skip(result.details)
    assert result.status == "PASS", result.details
