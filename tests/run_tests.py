#!/usr/bin/env python3
"""Run the compiler test cases defined in 测试用例.md.

The harness intentionally uses only the Python standard library.  It runs each
case in a private temporary directory because the compiler writes parse.kp in
its current working directory.  Tests are static: no generated AArch64 is
assembled, linked, or executed.
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Callable, Iterable


ROOT = Path(__file__).resolve().parents[1]


@dataclass
class Case:
    id: int
    item: str
    title: str
    criticality: str
    phase: str
    source: str
    checks: list[str]
    forbidden: list[str]
    note: str


@dataclass
class Result:
    id: int
    title: str
    phase: str
    criticality: str
    status: str
    duration_ms: int
    details: str
    artifacts: str = ""


def c(i: int, item: str, title: str, phase: str, source: str,
      checks: Iterable[str], forbidden: Iterable[str] = (),
      criticality: str = "高", note: str = "等价类") -> Case:
    return Case(i, item, title, criticality, phase, source,
                list(checks), list(forbidden), note)


# Baseline IDs correspond to rows 1–30 of 测试用例.md. Executable
# expectations are independent of Excel/openpyxl installations.
CASES: list[Case] = [
    c(1, "IR Lexer / src/ir/lexer.cpp", "关键字、符号与分隔符识别", "frontend",
      "int main(){return 0;}", [r"fun @main", r"ret 0"], note="等价类：关键字、符号、分隔符"),
    c(2, "IR Lexer / src/ir/lexer.cpp", "注释与空白处理", "frontend",
      "/* header */\nint main() { // line comment\n  return 0;\n}\n",
      [r"fun @main", r"ret 0"], [r"//", r"/\*"], note="等价类：行注释、块注释、空白字符"),
    c(3, "IR Parser / src/ir/parser.cpp", "全局变量、声明与函数定义解析", "frontend",
      "int g = 0;\nint main(){ return g; }\n",
      [r"global @", r"fun @main", r"load"], note="等价类：全局定义、函数定义、返回"),
    c(4, "前端常量折叠 / src/front/ExpCalc.cpp", "整数运算优先级与常量折叠", "frontend",
      "int main(){ return 1 + 2 * 3 - 4 / 2; }\n",
      [r"ret 5"], [r"add .*1", r"mul .*2"], note="等价类：常量表达式和优先级"),
    c(5, "前端词法与 AST / src/front/sysy.l,sysy.y", "整数与浮点字面量", "frontend",
      "float g = 0.5;\nint main(){ int a=0; float b=5.0; return a; }\n",
      [r"f32", r"0\.5|0\.500000|#", r"alloc i32", r"store 0", r"load", r"ret %"],
      note="等价类：整数/浮点字面量及局部变量返回"),
    c(6, "符号表 / src/front/variable.cpp", "常量、局部作用域与遮蔽", "frontend",
      "const int k=14; int main(){ int a=k+1; {int a=2; a=a+1;} return a;}\n",
      [r"@a\s*=\s*alloc i32", r"store 15,\s*@a", r"@__filed\d+__a\s*=\s*alloc i32",
       r"load @a", r"ret %"], note="等价类：常量、嵌套作用域、同名遮蔽"),
    c(7, "函数管理 / src/front/funcManager.cpp", "非 void 函数定义与调用", "frontend",
      "int add(int a,int b){return a+b;} int main(){return add(2,3);}\n",
      [r"fun @add", r"call @add", r"ret"], note="等价类：非 void 函数和实参调用"),
    c(8, "基本块管理 / src/front/bbmanager_air.cpp", "if/else 控制流生成", "frontend",
      "int main(){int x=1; if(x){x=2;}else{x=3;} return x;}\n",
      [r"br ", r"then", r"else|merge", r"jump"], note="场景法：双分支控制流"),
    c(9, "IR Lexer / src/ir/lexer.cpp", "整数与浮点边界字面量", "frontend",
      "float g=1.0; int main(){return -1;}\n",
      [r"f32", r"ret -?1|ret 4294967295|ret -1"], note="边界值：-1、0、f32 位模式"),
    c(10, "IR Parser / src/ir/parser.cpp", "长度为 1 的数组类型", "frontend",
      "int read(int a[][1]){return a[0][0];} int main(){int a[1][1]={{7}};return read(a);}\n",
      [r"\[i32, 1\]", r"get(ptr|elemptr)", r"call @read"], criticality="中",
      note="边界值：数组参数维度和长度为 1"),
    c(11, "数组管理 / src/front/variable.cpp", "1x1 多维数组和下标 0", "frontend",
      "int main(){int a[1][1]={{7}}; a[0][0]=8; return a[0][0];}\n",
      [r"alloc", r"getelemptr", r"store", r"load"], note="边界值：最小数组和零下标"),
    c(12, "数组初始化 / src/front/variable.cpp", "多维数组部分初始化与补零", "frontend",
      "int g[2][3]={{1,2},{3}}; int main(){int a[2][2]={{1},{2,3}}; return a[1][0];}\n",
      [r"\[\[i32, 3\], 2\]|\[\[i32, 2\], 2\]", r"0|zeroinit"], note="边界值：省略尾部元素和二维聚合"),
    c(13, "前端/IR Builder", "Koopa 指令链构造", "frontend",
      "int main(){int a; a=1; a=a+2; return a;}\n",
      [r"alloc i32", r"store", r"load", r"add", r"ret"], note="等价类：alloc/store/load/binary/ret"),
    c(14, "活跃变量分析 / src/opt/dfa.cpp", "while、continue、break 的 CFG", "frontend",
      "int main(){int i=0;while(i<3){i=i+1;if(i==2)continue;if(i==3)break;}return i;}\n",
      [r"while_entry", r"while_body", r"while_end", r"br ", r"jump"], note="场景法：循环回边、continue、break"),
    c(15, "数组参数与 IR 优化", "数组形参与 getptr/getelemptr", "frontend",
      "int first(int a[][2]){return a[0][1];} int main(){int a[1][2]={{1,2}};return first(a);}\n",
      [r"fun @first", r"getptr", r"getelemptr", r"call @first"], note="场景法：数组实参、指针寻址、多级索引"),

    c(16, "Visit_ret", "整数常量返回", "backend",
      "int main(){return 42;}\n", [r"mov[zkw]*[ \\t]+x?0,?", r"ret"], note="等价类：整数返回基本类"),
    c(17, "Visit_ret", "负数及 32 位边界常量", "backend",
      "int main(){return -2147483648;}\n", [r"(mov|movz|movn|movk)", r"ret"], note="边界值：有符号 32 位下界"),
    c(18, "Visit_binary", "整数基本算术运算", "backend",
      "int main(){int a=20+7; int b=a*3-5; return b/2 + b%2;}\n",
      [r"add", r"mul", r"sdiv", r"(msub|s?rem|sub)"], note="等价类：add/sub/mul/div/mod"),
    c(19, "Visit_binary", "有符号除法与取模", "backend",
      "int main(){int a=-17; int b=5; return a/b + a%b;}\n",
      [r"sdiv", r"(msub|sub|mul)"], note="等价类+边界值：符号组合"),
    c(20, "Visit_binary", "整数比较运算", "backend",
      "int main(){int a=-1; if(a<0)return 1; if(a==0)return 2; if(a>=-1)return 3; return 4;}\n",
      [r"cmp|cset|slt|sle|sgt|sge|seqz|snez", r"b\."], note="边界值：0、-1 与关系运算"),
    c(21, "Visit_binary", "浮点算术与比较", "backend",
      "float add(float a,float b){return a+b;} int main(){float x=add(1.5,2.5); if(x>3.0)return 1; return 0;}\n",
      [r"f(add|sub|mul|div)|fcmp|fcmpe|s[0-9]+", r"\.text"], note="等价类：f32 算术、比较和寄存器"),
    c(22, "Visit_alloc/load/store", "局部变量分配与读写", "backend",
      "int main(){int a; a=123; return a;}\n", [r"(add|sub).*sp", r"str", r"ldr"], note="等价类：局部内存访问"),
    c(23, "Visit_global_alloc", "全局变量与全局数组", "backend",
      "int g=7; int main(){return g;}\n", [r"\.data", r"g", r"(adrp|add|ldr)"], note="场景法：全局数据访问"),
    c(24, "Visit_elem_ptr", "一维数组边界地址计算", "backend",
      "int main(){int a[10]; a[0]=1; a[9]=2; return a[9];}\n", [r"(add|lsl|mul)", r"str", r"ldr"], note="边界值：首元素和末元素"),
    c(25, "Visit_elem_ptr/Visit_ptr", "多维数组地址计算", "backend",
      "int main(){int a[2][3]={{1,2,3},{4,5,6}}; return a[1][2];}\n", [r"(add|lsl|mul)", r"ldr|str"], note="场景法：二维数组逐级寻址"),
    c(26, "Visit_branch/jump", "条件分支真假路径", "backend",
      "int main(){int x=0; if(x)return 1; else return 2;}\n", [r"(cmp|cbz|cbnz|tst)", r"b\.?[a-z]*", r"(else|then|merge|main_)"], note="等价类：真路径和假路径"),
    c(27, "Visit_branch/jump", "循环回边与退出控制流", "backend",
      "int main(){int i=0; while(i<3){i=i+1;} return i;}\n", [r"while_entry|while_body|while_end", r"b\.?[a-z]*"], note="场景法+边界值：零次/多次循环结构"),
    c(28, "Visit_call", "整数参数传递边界", "backend",
      "int f(int a,int b,int c,int d,int e,int f,int g,int h,int i){return i;} int main(){return f(1,2,3,4,5,6,7,8,9);}\n",
      [r"bl[ \\t]+f", r"x[0-7]", r"(str|stp).*sp"], note="边界值：8 个寄存器参数与第 9 个栈参数"),
    c(29, "Visit_call", "浮点及混合参数传递", "backend",
      "float f(float a,float b){return a+b;} int main(){float x=f(1.0,2.0); return x>2.0;}\n",
      [r"bl[ \\t]+f", r"s[0-9]+", r"(fmov|fadd|fcmp)"], note="边界值+场景法：浮点调用约定"),
    c(30, "reg_alloc / 栈帧与调用保护", "高寄存器压力与 spill/reload", "backend",
      "int id(int x){return x;} int main(){int a=1,b=2,c=3,d=4,e=5,f=6,g=7,h=8; return id(a+b+c+d+e+f+g+h);}\n",
      [r"\.align", r"sp", r"(str|stp)", r"(ldr|ldp)", r"bl[ \\t]+id"], note="场景法：高寄存器压力、调用保存恢复"),
]

from extended_cases import make_cases, execute as execute_extended

CASES.extend(make_cases(c))


def find_compiler(explicit: str | None) -> Path | None:
    if explicit:
        p = Path(explicit).expanduser()
        # Each case runs from an isolated temporary cwd.  Resolve the compiler
        # before that cwd changes, otherwise ``test/compiler`` is looked up
        # relative to the temporary directory and every case reports ENOENT.
        if not p.is_absolute():
            cwd_candidate = (Path.cwd() / p).resolve()
            root_candidate = (ROOT / p).resolve()
            if cwd_candidate.exists():
                p = cwd_candidate
            elif root_candidate.exists():
                p = root_candidate
            else:
                # Also accept a compiler supplied through PATH (for example
                # ``--compiler sysyc``).  subprocess would otherwise emit a
                # misleading ENOENT only after entering the per-case cwd.
                on_path = shutil.which(explicit)
                if on_path:
                    p = Path(on_path).resolve()
        return p if p.exists() else None
    candidates = [ROOT / "test" / "compiler", ROOT / "test" / "compiler.exe",
                  ROOT / "compiler", ROOT / "compiler.exe",
                  ROOT / "build" / "compiler", ROOT / "build" / "compiler.exe"]
    for p in candidates:
        if p.exists():
            return p
    return None


def run_case(case: Case, compiler: Path | None, keep: Path | None) -> Result:
    start = time.perf_counter()
    if compiler is None:
        return Result(case.id, case.title, case.phase, case.criticality, "BLOCKED",
                      int((time.perf_counter() - start) * 1000),
                      "未找到编译器可执行文件；使用 --compiler 指定，或先运行构建脚本")
    work = Path(tempfile.mkdtemp(prefix=f"sysy_tc_{case.id:02d}_"))
    artifact_path = str(keep / f"TC{case.id:02d}") if keep else ""
    try:
        if case.id >= 31:
            execute_extended(case, compiler, work)
            return Result(case.id, case.title, case.phase, case.criticality, "PASS",
                          int((time.perf_counter() - start) * 1000), "所有静态断言通过",
                          str(keep / f"TC{case.id:02d}") if keep else "")
        src = work / "input.sy"
        asm = work / "output.s"
        src.write_text(case.source, encoding="utf-8")
        cmd = [str(compiler), "-S", str(src), "-o", str(asm)]
        proc = subprocess.run(cmd, cwd=work, text=True, capture_output=True, timeout=20)
        parse = work / "parse.kp"
        if proc.returncode != 0:
            detail = f"编译器退出码 {proc.returncode}; stderr={proc.stderr.strip()[-500:]}"
            status = "FAIL"
        elif not parse.exists() or (case.phase == "backend" and not asm.exists()):
            detail = "编译器未生成预期的 parse.kp 或 output.s"
            status = "FAIL"
        else:
            text = parse.read_text(encoding="utf-8", errors="replace")
            if case.phase == "backend":
                text += "\n" + asm.read_text(encoding="utf-8", errors="replace")
            missing = [p for p in case.checks if re.search(p, text, re.I | re.M) is None]
            forbidden = [p for p in case.forbidden if re.search(p, text, re.I | re.M)]
            if missing or forbidden:
                bits = []
                if missing:
                    bits.append("缺少: " + ", ".join(missing))
                if forbidden:
                    bits.append("不应出现: " + ", ".join(forbidden))
                detail, status = "; ".join(bits), "FAIL"
            else:
                detail, status = "所有静态断言通过", "PASS"
        if keep:
            dst = keep / f"TC{case.id:02d}"
            artifacts = str(dst)
        else:
            artifacts = ""
        return Result(case.id, case.title, case.phase, case.criticality, status,
                      int((time.perf_counter() - start) * 1000), detail, artifacts)
    except subprocess.TimeoutExpired:
        return Result(case.id, case.title, case.phase, case.criticality, "FAIL",
                      int((time.perf_counter() - start) * 1000), "单条用例超过 20 秒", artifact_path)
    except OSError as exc:
        # The target environment is Linux/AArch64.  ENOENT can mean that an
        # existing ELF cannot find its interpreter or a shared library, while
        # ENOEXEC indicates an architecture/format mismatch.  Both are
        # environment blocks, not compiler assertion failures.
        if getattr(exc, "errno", None) in (2, 8, 126) or getattr(exc, "winerror", None) == 193:
            status = "BLOCKED"
            if getattr(exc, "errno", None) == 2 and not compiler.exists():
                detail = "找不到编译器文件；请使用 --compiler 指定正确路径"
            elif getattr(exc, "errno", None) == 2:
                detail = ("编译器文件存在，但操作系统无法启动它（通常是 ELF 解释器/"
                          "动态库缺失或架构不匹配）；请检查运行时依赖")
            else:
                detail = "编译器文件不是目标 Linux 平台可执行格式；请提供 Linux/AArch64 编译环境"
        else:
            status = "FAIL"
            detail = repr(exc)
        return Result(case.id, case.title, case.phase, case.criticality, status,
                      int((time.perf_counter() - start) * 1000), detail, artifact_path)
    except Exception as exc:
        return Result(case.id, case.title, case.phase, case.criticality, "FAIL",
                      int((time.perf_counter() - start) * 1000), repr(exc), artifact_path)
    finally:
        if keep:
            dst = keep / f"TC{case.id:02d}"
            shutil.copytree(work, dst, dirs_exist_ok=True)
        shutil.rmtree(work, ignore_errors=True)


def write_reports(results: list[Result], outdir: Path) -> None:
    outdir.mkdir(parents=True, exist_ok=True)
    (outdir / "results.json").write_text(
        json.dumps([asdict(r) for r in results], ensure_ascii=False, indent=2), encoding="utf-8")
    with (outdir / "results.csv").open("w", newline="", encoding="utf-8-sig") as f:
        w = csv.DictWriter(f, fieldnames=list(Result.__dataclass_fields__))
        w.writeheader()
        w.writerows(asdict(r) for r in results)
    counts = {s: sum(r.status == s for r in results) for s in ("PASS", "FAIL", "BLOCKED")}
    lines = ["# 自动化测试结果", "", f"总用例数：{len(results)}",
             f"通过：{counts['PASS']}，失败：{counts['FAIL']}，阻塞：{counts['BLOCKED']}", "",
             "| ID | 阶段 | 标题 | 状态 | 耗时(ms) | 详情 |", "|---:|---|---|---|---:|---|"]
    for r in results:
        lines.append(f"| {r.id} | {r.phase} | {r.title} | {r.status} | {r.duration_ms} | {r.details.replace('|', '/') } |")
    (outdir / "results.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    ap = argparse.ArgumentParser(description="Run all SysY compiler test cases")
    ap.add_argument("--compiler", help="compiler executable path")
    ap.add_argument("--out", default=str(ROOT / "test-results"), help="report directory")
    ap.add_argument("--keep-artifacts", action="store_true", help="keep per-case parse.kp/output.s")
    ap.add_argument("--case", type=int, action="append", help="run selected ID(s); default all")
    args = ap.parse_args()
    selected = [x for x in CASES if not args.case or x.id in args.case]
    if not selected:
        ap.error("没有匹配的用例编号")
    compiler = find_compiler(args.compiler)
    keep = Path(args.out) / "artifacts" if args.keep_artifacts else None
    results = [run_case(case, compiler, keep) for case in selected]
    write_reports(results, Path(args.out))
    for r in results:
        print(f"TC{r.id:02d} [{r.status:7}] {r.title} - {r.details}")
    print(f"Reports: {Path(args.out).resolve()}")
    return 1 if any(r.status == "FAIL" for r in results) else 0


if __name__ == "__main__":
    sys.exit(main())
