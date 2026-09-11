"""pytest fixtures and CLI options for the Linux-targeted compiler tests."""

from __future__ import annotations

import os
from pathlib import Path

import pytest

ROOT = Path(__file__).resolve().parents[1]


def pytest_addoption(parser: pytest.Parser) -> None:
    group = parser.getgroup("compiler")
    group.addoption("--compiler", action="store", default=None,
                    help="Linux compiler executable (default: test/compiler or compiler)")
    group.addoption("--keep-artifacts", action="store_true",
                    help="保留每条用例的输入、parse.kp、汇编和诊断信息")
    group.addoption("--report-dir", action="store", default=str(ROOT / "test-results"),
                    help="JSON/CSV/Markdown 报告目录")


@pytest.fixture(scope="session")
def compiler_path(request: pytest.FixtureRequest) -> Path | None:
    from run_tests import find_compiler
    return find_compiler(request.config.getoption("--compiler"))


@pytest.fixture(scope="session")
def report_dir(request: pytest.FixtureRequest) -> Path:
    path = Path(request.config.getoption("--report-dir")).expanduser().resolve()
    path.mkdir(parents=True, exist_ok=True)
    return path


@pytest.fixture(scope="session", autouse=True)
def configure_environment(request: pytest.FixtureRequest) -> None:
    # The compiler and generated assembly are Linux/AArch64 artifacts.  This
    # variable documents the target without pretending to execute assembly.
    os.environ.setdefault("SYSY_TARGET", "linux-aarch64")
