# SysY to ARMv8 编译器

这是一个以 ARMv8（AArch64）汇编为目标后端的 SysY 编译器。项目采用“前端生成 Koopa IR、IR 分析与优化、后端生成 ARMv8 汇编”的处理流程。

## 前端支持概览

该编译器语法符合 2025 年全国大学生计算机系统能力大赛编译系统设计赛道 SysY 语法规则，其包含 `int`、`float`、`void`，常量和变量声明，多维数组，函数定义和调用，赋值、返回、`if/else`、`while`、`break`、`continue`，以及一元、算术、关系、相等、逻辑与/或表达式。前端会对可确定的常量表达式进行折叠，对需要运行时求值的表达式生成 IR 指令。

前端主要位于 `src/front` 和 `include/front`：

- `sysy.l`、`sysy.y`：SysY 词法和语法分析；
- `ast.cpp`：AST 遍历、符号查找以及 Koopa IR 文本生成；
- `variable.cpp`：作用域、变量/常量、函数参数和数组初始化管理；
- `bbmanager_air.cpp`：基本块、条件分支和循环控制流管理；
- `funcManager.cpp`：函数声明、函数返回类型和调用参数管理。

## IR 与优化测试边界

中间代码采用 Koopa IR。

`src/ir` 目前解析的重点包括全局定义、函数声明/定义、基本块、`alloc`、`load`、`store`、`getptr`、`getelemptr`、二元表达式、函数调用、分支、跳转和返回，以及整数、浮点数、`undef`、`zeroinit`、聚合初始化值。

`src/opt/dfa.cpp` 的主要可测行为是：建立基本块前驱/后继关系，收集每条指令的 def/use，迭代计算 live-in/live-out，使用 Tarjan 算法识别循环，并据此计算活跃区间。

`src/opt` 还维护支配关系、符号使用关系和部分优化所需的数据结构。当前 `Optimizer::init_used_by` 负责重建指令的使用关系；优化测试应以实际启用的 pass 为准，不应默认所有声明的优化接口都已经完成。

## 后端支持概览

后端位于 `src/back` 和 `include/back`，输入为已经构造并完成分析的 Koopa raw program，输出为 ARMv8/AArch64 汇编文本。

### 汇编遍历与指令选择

`src/back/visit.cpp` 按照 Program → Function → Basic Block → Instruction 的层次遍历 Koopa IR，并为主要 IR 指令选择汇编序列：

- 整数和浮点常量加载；
- 整数 `add/sub/mul/div/mod`、位运算和比较；
- 浮点 `fadd/fsub/fmul/fdiv` 及浮点比较；
- `alloc`、`load`、`store`；
- `getptr`、`getelemptr` 的地址计算；
- `br`、`jump`、`ret`；
- 函数调用以及整数/浮点参数传递；
- 全局变量、全局数组和聚合初始化。

`src/back/koopa_print.cpp` 封装了立即数加载、整数/浮点移动、算术、比较、访存和类型转换等汇编打印辅助函数。

### 寄存器分配与栈帧

`src/back/regalloc.cpp` 使用基于活跃区间的线性扫描寄存器分配：

- 整数值使用 `x` 寄存器，浮点值使用 `s` 寄存器；
- 分配前调用 `src/opt/dfa.cpp` 中的活跃变量和活跃区间分析；
- 通过 `reg_alloc_map` 保存 Koopa value 到物理寄存器的映射；
- 对区间结束的寄存器进行回收，并维护寄存器占用顺序；
- 没有可用寄存器时进入 spill 处理路径，并使用栈槽保存值。

`Visit_func` 为每个函数建立栈帧，处理局部临时值、数组空间、调用参数空间、返回地址以及寄存器保存/恢复。栈帧大小按 16 字节边界对齐。

### 调用约定相关处理

后端分别统计整数参数和浮点参数：前若干个参数优先使用参数寄存器，超出寄存器数量的参数放置到调用栈区域；返回值分别通过整数返回寄存器或浮点返回寄存器传递。函数调用前后会根据当前函数是否存在子调用决定是否保存和恢复链接寄存器。

由于当前实现仍保留实验性寄存器和栈帧策略，调用约定、被调用者保存寄存器、spill 以及混合整数/浮点参数应作为重点测试对象。

## 编译流程

项目的完整流程如下：

```text
SysY 源码
  -> Flex/Bison 语法分析
  -> 前端 AST dump
  -> Koopa IR 文本（parse.kp）
  -> 自定义 Koopa Lexer/Parser
  -> Program / FuncInfo / BBInfo / SymInfo
  -> used-by、活跃变量和活跃区间分析
  -> 线性扫描寄存器分配
  -> ARMv8 汇编
```

前端 AST 当前通过 `dump()` 直接输出 Koopa IR 文本，后端阶段再由 `src/ir` 重新解析该文本。因此 `parse.kp` 是两阶段之间的中间产物，默认写入当前工作目录。

## 构建与使用

构建脚本为 `build.sh`，依赖 Bash、Flex、Bison 和支持 C++17 的 Clang：

```bash
bash build.sh
```

编译器命令行形式：

```bash
test/compiler -S <input.sy> -o <output.s>
```

`-O*` 参数目前会被命令行解析但不会选择不同的优化等级。未指定 `-S` 时，程序只完成前端 IR 文件生成，不输出汇编。

## 后端测试边界

在没有 AArch64 汇编、链接和运行环境时，后端测试限定为静态测试：

- 验证生成过程是否成功完成，是否出现断言失败或非法状态；
- 检查汇编中的指令类别、寄存器类别、标签、栈偏移和数据段布局；
- 检查 Koopa 操作数到 ARMv8 操作数的映射；
- 检查整数/浮点参数寄存器、栈参数和返回寄存器的使用；
- 检查数组地址计算、分支跳转、函数序言/尾声以及保存恢复序列。

静态测试不能单独证明汇编在目标机器上的运行时语义。若需要验证运行结果，应在具备 AArch64 交叉汇编器、链接器或模拟器后增加动态测试阶段。

## 已知实现约束

- `parse.kp` 使用固定文件名，不适合多个编译任务并行运行；
- 优化器目前以分析和使用关系维护为主，具体优化 pass 的覆盖范围应以代码实际启用情况为准；
- 参数寄存器尚未完全纳入通用寄存器分配；
- spill、调用保护和栈帧分配仍包含待完善的实验性逻辑；
- 后端保存的寄存器集合和栈空间策略偏保守，生成的汇编可能包含冗余保存/恢复指令；
- 浮点、数组参数、聚合初始化和边界条件应通过独立回归用例持续验证；
- `build.sh` 面向类 Unix 环境，Windows 下需要 Bash、Flex、Bison 和 Clang 工具链兼容层。

## 全流程自动化测试

测试依赖新增pytest>=7.0。

当前按 `测试用例.md` 自动执行 46 条用例：TC01–TC15 覆盖前端和 Koopa IR/分析链路，TC16–TC30 覆盖 Koopa IR 到 ARMv8 汇编链路，TC31–TC46 补充短路、类型转换、调用、数据布局、控制流、错误处理和命令行场景。pytest 入口为 `tests/test_compiler.py`，公共执行器位于 `tests/run_tests.py`，新增用例及结构断言位于 `tests/extended_cases.py`。

详细的测试范围、单条用例流程、状态判定和完成标准见 [`tests/TEST_PROCESS.md`](tests/TEST_PROCESS.md)。

测试过程按以下步骤执行：

1. 为每条用例创建独立临时工作目录，并写入对应的 SysY 输入；
2. 调用编译器 `-S` 生成 `parse.kp` 和汇编文件；
3. 对前端用例检查 IR 文本，对后端用例检查 IR 与 ARMv8 汇编文本中的指令、寄存器、标签、数据段、访存和调用约定模式；
4. 记录每条用例的 `PASS`、`FAIL` 或 `BLOCKED`，输出 JSON、CSV 和 Markdown 汇总；
5. 不执行生成的汇编，不进行 AArch64 汇编、链接或运行，因此不要求本机具备 AArch64 环境。

默认运行全部 46 条用例（Linux 主机，汇编目标为 AArch64）：

```bash
python -m pytest -q tests/test_compiler.py --compiler test/compiler
```

仅运行新增 TC31–TC46（推荐保留诊断产物）：

```bash
python3 -m pytest -q tests/test_compiler.py -m extended --compiler ./test/compiler --keep-artifacts
```

仅运行原有前三十条使用 `-m baseline`。执行器自身回归测试使用 `python3 -m pytest -q tests/test_extended_harness.py`，其结果不代表被测编译器通过。

如果编译器尚未构建，可先执行 `bash build.sh`，然后重新运行测试。应在 Linux/WSL/AArch64 环境中提供可运行的 ELF 编译器：

```bash
python -m pytest -q tests/test_compiler.py --compiler ./test/compiler
```

报告默认写入 `test-results/`：

- `results.md`：适合人工查看的汇总表；
- `results.csv`：适合 Excel 或后续统计处理；
- `results.json`：供 CI 或其他脚本消费。

调试单条或多条用例：

```bash
python -m pytest -q tests/test_compiler.py --compiler ./test/compiler -k 'TC16 or TC30' --keep-artifacts
```

启用 `--keep-artifacts` 后，每条用例的 `input.sy`、`parse.kp`、`output.s` 和诊断信息会保存在 `test-results/artifacts/TCxx/`。编译器是唯一外部依赖，测试不 Mock 编译器本身；缺少编译器、无法启动或目标架构不匹配时，pytest 将用例标记为 `SKIPPED`，并在汇总中记为 `BLOCKED`。编译器异常退出、输出缺失或静态断言不满足时才记为失败。测试只检查文本，不执行 ARMv8 汇编。

### pytest 核心设计

- 参数化/数据驱动：`CASES` 是 46 条结构化 `Case` 数据，`@pytest.mark.parametrize` 自动生成 TC01–TC46；TC31–TC46 共用扩展执行器，支持多次调用和预期错误退出。
- 测试数据组织：每条记录包含阶段、源代码、正向正则断言、禁止模式、重要级别和测试方法备注；新增用例无需复制测试函数。
- 外部依赖处理：编译器路径通过 `--compiler` 注入并在启动临时目录前解析为绝对路径。不存在或无法启动时使用 pytest `skip` 表示环境阻塞，不伪造通过结果。
- 隔离与并发安全：每条用例使用独立临时目录，隔离固定生成的 `parse.kp`；不执行汇编、链接或 AArch64 程序。
- 结果汇总：pytest 会话结束时生成 `results.md`、`results.csv`、`results.json`，便于人工审阅和 CI 消费。

## 目录结构

```text
include/                 头文件
  front/                 SysY 前端、符号表和基本块管理
  ir/                    Koopa IR 数据结构和解析接口
  opt/                   数据流分析和优化接口
  back/                  ARMv8 汇编生成和寄存器分配接口
src/                     源文件
  front/                 Flex/Bison 及前端实现
  ir/                    Koopa IR 解析和构造
  opt/                   活跃变量、支配关系和优化实现
  back/                  ARMv8 后端实现
test/                    示例输入、参考 IR/汇编及编译器产物
build.sh                 构建脚本
```
