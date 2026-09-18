"""TC31–TC46: static integration checks, matching 测试用例.md.

IR and assembly are checked separately. These checks do not prove runtime
semantics or inspect the optimizer's private analysis structures.
"""
from __future__ import annotations

import json
import re
import shutil
import subprocess
from pathlib import Path


def make_cases(c):
    rows = [
        (31, "逻辑与、逻辑或和逻辑非的短路控制流", "int side(){putint(1);return 1;} int main(){int x=0;if(x&&side())return 1;if(!x||side())return 0;return 2;}"),
        (32, "整数与浮点混合运算及比较转换", "float f(float x){return x+0.5;} int main(){int i=2;float a=f(i)+i;return a>4.0;}"),
        (33, "void 函数调用与隐式返回", "void touch(int x){if(x)putint(x);} int main(){touch(1);return 0;}"),
        (34, "整数与浮点 I/O 及计时函数调用", "int main(){starttime();int a=getint();float f=getfloat();putint(a);putfloat(f);stoptime();return 0;}"),
        (35, "递归函数的返回值、分支与链接寄存器保护", "int fact(int n){if(n<=1)return 1;return n*fact(n-1);} int main(){return fact(5);}"),
        (36, "嵌套多函数调用与中间返回值保存", "int h(int x){return x+1;} int g(int x){return x*2;} int f(int a,int b){return a-b;} int main(){return f(g(h(3)),h(4));}"),
        (37, "全局常量、零初始化标量和零初始化数组", "const int C=3;int g;float gf;int a[2][2];int main(){return C+g+a[1][1];}"),
        (38, "大局部数组的首尾元素读写", "int main(){int a[1024];a[0]=1;a[1023]=2;return a[0]+a[1023];}"),
        (39, "9 个整数与 9 个浮点混合参数溢出到栈", "int mix(int a0,int a1,int a2,int a3,int a4,int a5,int a6,int a7,int a8,float b0,float b1,float b2,float b3,float b4,float b5,float b6,float b7,float b8){return a8+(b8>0.0);} int main(){return mix(0,1,2,3,4,5,6,7,8,1.0,2.0,3.0,4.0,5.0,6.0,7.0,8.0,9.0);}"),
        (40, "浮点函数返回值参与后续比较", "float half(float x){return x/2.0;} int main(){float y=half(7.0);return y>3.0;}"),
        (41, "内层 break/continue 的最近循环目标", "int main(){int i=0,j=0;while(i<3){i=i+1;j=0;while(j<3){j=j+1;if(j==1)continue;if(j==2)break;}}return i+j;}"),
        (42, "菱形 CFG 合流后的变量定义与使用", "int main(){int y;if(getint())y=1;else y=2;return y+3;}"),
        (43, "非法 SysY 语法的诊断与产物阻断", "int main( { return 0; }"),
        (44, "无 -S、-O 等级与指定输出路径", "int main(){return 7;}"),
        (45, "含空格的输入和汇编输出文件名", "int main(){return 7;}"),
        (46, "同目录连续编译的 IR 覆盖与状态重置", "int first(){return 1;} int main(){return first();}"),
    ]
    return [c(i, "全流程 / 测试用例.md", title, "backend", source, [],
              criticality="中" if i >= 43 else "高", note="静态集成：IR、汇编及命令行结构断言")
            for i, title, source in rows]


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def patterns(text, *expected):
    for pattern in expected:
        require(re.search(pattern, text, re.M) is not None, f"缺少模式：{pattern}")


def function(ir, name):
    match = re.search(r"fun\s+@" + re.escape(name) + r"\([^\n]*\)\s*(?::\s*\w+)?\s*\{(.*?)\}", ir, re.S)
    require(match is not None, f"缺少函数 @{name}")
    return match.group(1)


def blocks(body):
    matches = list(re.finditer(r"^\s*(%[\w.]+):\s*$", body, re.M))
    require(bool(matches), "函数没有基本块")
    result = {}
    for index, match in enumerate(matches):
        end = matches[index + 1].start() if index + 1 < len(matches) else len(body)
        lines = [line.strip() for line in body[match.end():end].splitlines() if line.strip()]
        require(bool(lines), f"空基本块 {match[1]}")
        require(re.match(r"(?:br|jump|ret)\b", lines[-1]) is not None,
                f"基本块 {match[1]} 缺少终结指令")
        require(not any(re.match(r"(?:br|jump|ret)\b", line) for line in lines[:-1]),
                f"基本块 {match[1]} 终结后仍有指令")
        result[match[1]] = lines
    for label, lines in result.items():
        for target in edges(lines):
            require(target in result, f"{label} 跳转到不存在的块 {target}")
    return result


def edges(lines):
    last = lines[-1]
    if last.startswith("br "):
        return re.findall(r"%[\w.]+", last.split(",", 1)[1])
    return re.findall(r"%[\w.]+", last) if last.startswith("jump ") else []


def reachable(graph, start, target):
    pending, seen = [start], set()
    while pending:
        node = pending.pop()
        if node == target:
            return True
        if node not in seen:
            seen.add(node)
            pending.extend(edges(graph[node]))
    return False


def constant_argument(body, operand):
    """Accept a literal or the frontend's `%n = add 0, literal` copy."""
    operand = operand.strip()
    if re.fullmatch(r"-?\d+", operand):
        return int(operand)
    match = re.search(re.escape(operand) + r"\s*=\s*add\s+0,\s*(-?\d+)\s*$", body, re.M)
    return int(match[1]) if match else None


def constant_offsets(asm):
    """Track straight-line constant materialization used by array addresses."""
    known, offsets = {}, set()
    for line in asm.splitlines():
        if re.match(r"\s*[\w.]+:\s*$", line):
            known.clear()
        match = re.match(r"\s*(\w+)\s+([wx]\d+),\s*(.*)", line)
        if not match:
            continue
        op, dest, operands = match.groups()
        previous = known.pop(dest, None)
        immediate = re.fullmatch(r"#(\d+)(?:,\s*lsl\s*#(\d+))?", operands)
        if op in ("mov", "movz", "movk") and immediate:
            value, shift = int(immediate[1]), int(immediate[2] or 0)
            if op != "movk" or previous is not None:
                known[dest] = ((previous & ~(65535 << shift)) if op == "movk" else 0) | (value << shift)
        elif op == "mov" and operands in known:
            known[dest] = known[operands]
        elif op == "lsl":
            shifted = re.fullmatch(r"([wx]\d+),\s*#(\d+)", operands)
            if shifted:
                source = previous if shifted[1] == dest else known.get(shifted[1])
                if source is not None:
                    known[dest] = source << int(shifted[2])
        elif op == "add":
            parts = operands.split(",")
            if len(parts) == 2:
                rhs = parts[1].strip()
                value = int(rhs[1:]) if re.fullmatch(r"#\d+", rhs) else previous if rhs == dest else known.get(rhs)
                if value is not None:
                    offsets.add(value)
    return offsets


def check_outputs(case_id, ir, asm):
    main = function(ir, "main")
    graph = blocks(main)
    patterns(asm, r"^\s*main:", r"^\s*ret\b")
    if case_id == 31:
        calls = [label for label, lines in graph.items() if any("call @side(" in line for line in lines)]
        require(len(calls) == 2, "应生成两个受短路控制的 side 调用")
        entry = next(iter(graph))
        require(entry not in calls, "side 不得在入口块无条件调用")
        for prefix, call_label, call_edge in zip(("and", "or"), calls, (0, 1)):
            candidates = [(label, edges(lines)) for label, lines in graph.items()
                          if lines[-1].startswith("br ") and any(prefix in dest for dest in edges(lines))]
            require(len(candidates) == 1, f"缺少唯一的 {prefix} 短路分支")
            _, destinations = candidates[0]
            require(reachable(graph, destinations[call_edge], call_label), f"{prefix} 求值路径不能到达 side")
            require(not reachable(graph, destinations[1-call_edge], call_label), f"{prefix} 短路路径仍能到达 side")
        patterns(main, r"\beq\s+0,", r"\b(?:and|or)\s+")
        patterns(asm, r"^\s*b\.\w+\s+", r"^\s*bl\s+side\b")
    elif case_id in (32, 40):
        name = "f" if case_id == 32 else "half"
        patterns(ir, rf"fun @{name}\([^\n]*f32[^\n]*\)\s*:\s*f32", rf"call @{name}\(")
        patterns(asm, rf"^\s*bl\s+{name}\b", r"^\s*fcmp\w*\s+", r"\bs0\b", r"\b[wx]0\b")
        patterns(asm, r"^\s*fadd\s+" if case_id == 32 else r"^\s*fdiv\s+")
        if case_id == 32:
            patterns(asm, r"^\s*scvtf\s+")
    elif case_id == 33:
        touch = function(ir, "touch")
        require(re.search(r"fun @touch\([^\n]*\)\s*\{", ir) is not None, "void 函数不应声明值返回类型")
        blocks(touch)
        patterns(touch, r"^\s*ret\s*$", r"call @putint\(")
        patterns(main, r"^\s*call @touch\(", r"ret 0")
        require(not re.search(r"=\s*call @touch", main), "void 调用不应赋值")
        patterns(asm, r"^\s*bl\s+touch\b", r"^\s*bl\s+putint\b")
    elif case_id == 34:
        for name, signature in (("getint", r"\(\):i32"), ("getfloat", r"\(\):f32"),
                                ("putint", r"\(i32\)"), ("putfloat", r"\(f32\)"),
                                ("_sysy_starttime", r"\(i32\)"), ("_sysy_stoptime", r"\(i32\)")):
            patterns(ir, rf"^decl @{name}{signature}\s*$")
            patterns(main, rf"call @{name}\(")
            patterns(asm, rf"^\s*bl\s+{name}\b")
        for name in ("_sysy_starttime", "_sysy_stoptime"):
            argument = re.search(r"call @" + name + r"\(([^)]*)\)", main)[1]
            line = constant_argument(main, argument)
            require(line is not None and line > 0, f"{name} 未传入有效源代码行号")
        patterns(asm, r"\bs0\b", r"\b[wx]0\b")
    elif case_id == 35:
        fact = function(ir, "fact")
        blocks(fact)
        patterns(fact, r"\bbr\s+", r"\bmul\s+")
        call = re.search(r"(%\w+)\s*=\s*call @fact\(", fact)
        require(call is not None, "缺少递归调用结果")
        patterns(fact, r"mul[^\n]*" + re.escape(call[1]) + r"(?:\s|,|$)")
        patterns(asm, r"^\s*bl\s+fact\b", r"^\s*(?:str|stp)\s+[^\n]*\bx30\b", r"^\s*(?:ldr|ldp)\s+[^\n]*\bx30\b")
    elif case_id == 36:
        calls = re.findall(r"(%\w+)\s*=\s*call @(h|g|f)\(([^)]*)\)", main)
        require([name for _, name, _ in calls] == ["h", "g", "h", "f"], "调用依赖顺序应为 h→g→h→f")
        require(constant_argument(main, calls[0][2]) == 3 and constant_argument(main, calls[2][2]) == 4, "h 的输入错误")
        require(calls[1][2].strip() == calls[0][0], "g 未使用第一个 h 的结果")
        require([s.strip() for s in calls[3][2].split(",")] == [calls[1][0], calls[2][0]], "f 的实参来源错误")
        require(re.findall(r"^\s*bl\s+(h|g|f)\b", asm, re.M) == ["h", "g", "h", "f"], "汇编调用顺序错误")
        patterns(asm, r"^\s*(?:str|stp)\s+", r"^\s*(?:ldr|ldp)\s+")
    elif case_id == 37:
        for name, ty, size in (("g", "i32", 4), ("gf", "f32", 4), ("a", "[[i32, 2], 2]", 16)):
            match = re.search(r"^global @((?:[\w]+__)?" + name + r")\s*=\s*alloc\s+" + re.escape(ty) + r",\s*([^\n]+)", ir, re.M)
            require(match is not None, f"缺少全局对象 {name}")
            init = match[2]
            require(init.strip() == "zeroinit" or (not re.search(r"[1-9]", init) and "0" in init), f"{name} 未零初始化")
            common = re.search(r"^\s*\.comm\s+" + re.escape(match[1]) + r",\s*(\d+),\s*\d+", asm, re.M)
            if common:
                require(int(common[1]) == size, f"{name} 数据大小 {common[1]}，预期 {size}（SysY i32/f32 为 4 字节）")
                continue
            data = re.search(r"^" + re.escape(match[1]) + r":\s*\n((?:\s*\.(?:xword|word|zero|long|space)[^\n]*\n?)+)", asm, re.M)
            require(data is not None, f"汇编数据段缺少 {name}")
            total = 0
            for directive, value in re.findall(r"\.(xword|word|long|zero|space)\s+([^\n]+)", data[1]):
                nums = [int(v.strip(), 0) for v in value.split(",")]
                require(directive in ("zero", "space") or all(v == 0 for v in nums), f"{name} 数据不为零")
                total += nums[0] if directive in ("zero", "space") else (8 if directive == "xword" else 4) * len(nums)
            require(total == size, f"{name} 数据大小 {total}，预期 {size}")
        patterns(main, r"\badd\s+[^\n]*\b3\b")
    elif case_id == 38:
        patterns(main, r"alloc \[i32, 1024\]", r"getelemptr[^\n]*,\s*0\b", r"getelemptr[^\n]*,\s*1023\b", r"\bstore\s+", r"\bload\s+")
        patterns(asm, r"^\s*str\s+", r"^\s*ldr\s+", r"\bsp\b")
        require(4092 in constant_offsets(asm), "末元素地址未按 1023×4 = 4092 字节偏移构造")
        # Accept direct subtraction or the backend's temporary-register sequence.
        frames = re.findall(r"sub\s+(?:sp|x\d+),\s*sp,\s*#(\d+)", asm)
        require(any(int(n) >= 4096 and int(n) % 16 == 0 for n in frames), "缺少至少 4096 字节且 16 字节对齐的栈帧")
        for op, imm in re.findall(r"\b(add|sub)\s+[^\n]*?#(\d+)\s*$", asm, re.M):
            require(int(imm) <= 4095, f"{op} 立即数 {imm} 超出无移位编码范围")
    elif case_id == 39:
        patterns(main, r"call @mix\(")
        args = re.search(r"call @mix\(([^)]*)\)", main)[1].split(",")
        require(len(args) == 18, "mix 应接收 18 个参数")
        patterns(asm, r"^\s*bl\s+mix\b")
        for i in range(8):
            patterns(asm, rf"^\s*(?:mov|movz|movk|ldr|fmov)\s+[wx]{i}\b", rf"^\s*(?:fmov|ldr|scvtf)\s+s{i}\b")
        # Inspect only outgoing stores immediately before the call; prologue
        # register saves must not satisfy the overflow-argument checks.
        before = re.split(r"^\s*bl\s+mix\b", asm, maxsplit=1, flags=re.M)[0]
        before = re.split(r"^\s*main:\s*$", before, flags=re.M)[-1]
        outgoing = re.findall(r"^\s*str\s+([xws]\d+),\s*\[sp(?:,\s*#(\d+))?\]", before, re.M)
        require(any(reg[0] in "xw" and int(offset or 0) == 0 for reg, offset in outgoing), "第 9 个整数参数未存入 outgoing 栈槽 0")
        require(any(reg.startswith("s") and int(offset or 0) == 8 for reg, offset in outgoing), "第 9 个浮点参数未存入 outgoing 栈槽 8")
    elif case_id == 41:
        entries = [label for label in graph if "while_entry" in label]
        require(len(entries) == 2, "应有两层循环")
        inner = entries[1]
        suffix = inner.split("while_entry", 1)[1]
        end = "%while_end" + suffix
        require(end in graph, "缺少内层循环出口")
        conditionals = [edges(lines) for label, lines in graph.items()
                        if "then" not in label and lines[-1].startswith("br ")
                        and all("while_" not in dest for dest in edges(lines))]
        require(len(conditionals) == 2, "缺少内层两个 if 分支")
        for destinations, target in zip(conditionals, (inner, end)):
            require(graph[destinations[0]][-1] == "jump " + target, f"continue/break 应跳转至 {target}")
        patterns(asm, r"while_entry", r"while_end")
    elif case_id == 42:
        patterns(main, r"call @getint\(", r"\bbr\s+", r"\badd\s+[^\n]*\b3\b")
        stores = [re.search(r"store\s+" + str(n) + r",\s*([@%][\w.]+)", main) for n in (1, 2)]
        require(all(stores) and stores[0][1] == stores[1][1], "两个分支必须写入同一个 y")
        destinations = next(edges(lines) for lines in graph.values() if lines[-1].startswith("br "))
        ends = [edges(graph[d]) for d in destinations]
        require(ends[0] == ends[1] and len(ends[0]) == 1, "两个分支未合流")
        patterns("\n".join(graph[ends[0][0]]), r"load\s+" + re.escape(stores[0][1]))
        patterns(asm, r"^\s*bl\s+getint\b", r"^\s*ldr\s+", r"^\s*b\.\w+\s+")


def execute(case, compiler: Path, work: Path):
    """Raises on failed expectations; shared runner handles status/reporting."""
    def compile_step(source, tag, *, assembly=True, optimize=False, spaces=False):
        src = work / ("input with space.sy" if spaces else "input.sy")
        asm = work / ("output with space.s" if spaces else "named-output.s" if optimize else "output.s")
        src.write_text(source, encoding="utf-8")
        args = [str(compiler)] + (["-O2"] if optimize else [])
        args += ["-S", str(src), "-o", str(asm)] if assembly else [str(src)]
        try:
            proc = subprocess.run(args, cwd=work, text=True, encoding="utf-8", errors="replace", capture_output=True, timeout=20)
        except subprocess.TimeoutExpired as exc:
            (work / f"{tag}.timeout.txt").write_text(str(exc), encoding="utf-8")
            raise
        (work / f"{tag}.diagnostics.json").write_text(json.dumps(dict(command=args, returncode=proc.returncode, stdout=proc.stdout, stderr=proc.stderr), ensure_ascii=False, indent=2), encoding="utf-8")
        parse = work / "parse.kp"
        ir = parse.read_text(encoding="utf-8", errors="replace") if parse.exists() else ""
        text = asm.read_text(encoding="utf-8", errors="replace") if asm.exists() else ""
        if parse.exists():
            shutil.copy2(parse, work / f"{tag}.kp")
        if asm.exists():
            shutil.copy2(asm, work / f"{tag}.s")
        shutil.copy2(src, work / f"{tag}.sy")
        return proc, ir, text, asm

    def success(output, assembly=True):
        proc, ir, asm, path = output
        require(proc.returncode == 0, f"编译器退出码 {proc.returncode}；stderr={proc.stderr[-1000:]}")
        require(bool(ir.strip()), "未生成非空 parse.kp")
        require(not assembly or (path.exists() and bool(asm.strip())), "未生成非空汇编文件")
        return ir, asm

    if case.id == 43:
        proc, ir, asm, _ = compile_step(case.source, "invalid")
        require(proc.returncode > 0, f"语法错误应正常非零退出，实际 {proc.returncode}（负数表示信号崩溃）")
        patterns(proc.stderr, r"(?i)(syntax|parse|语法).*error|语法错误")
        require(not re.search(r"(?i)assert|abort|segmentation|core dumped", proc.stderr), "语法错误触发了断言或崩溃")
        require(not asm.strip() and not re.search(r"fun\s+@main", ir), "非法输入仍生成有效程序产物")
    elif case.id == 44:
        ir, _ = success(compile_step(case.source, "frontend", assembly=False), assembly=False)
        patterns(ir, r"ret 7")
        require(not list(work.glob("*.s")), "无 -S 时不应生成汇编")
        ir, asm = success(compile_step(case.source, "optimized", optimize=True))
        patterns(ir, r"ret 7")
        patterns(asm, r"^\s*main:", r"^\s*ret\b")
    elif case.id == 45:
        ir, asm = success(compile_step(case.source, "spaces", spaces=True))
        patterns(ir, r"ret 7")
        patterns(asm, r"^\s*main:", r"^\s*ret\b")
        require(not (work / "output.s").exists(), "输出路径被截断或忽略")
    elif case.id == 46:
        ir, asm = success(compile_step(case.source, "first"))
        patterns(ir, r"fun @first", r"call @first\(")
        patterns(asm, r"^\s*first:", r"^\s*bl\s+first\b")
        ir, asm = success(compile_step("int main(){return 2;}", "second"))
        patterns(ir, r"ret 2")
        require("first" not in ir + asm, "第二次产物残留 first")
        patterns(asm, r"^\s*main:", r"^\s*ret\b")
    else:
        ir, asm = success(compile_step(case.source, "compile"))
        check_outputs(case.id, ir, asm)
