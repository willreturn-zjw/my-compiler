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

`src/opt` 还维护支配关系、符号使用关系和部分优化所需的数据结构。当前 `Optimizer::init_used_by` 负责重建指令的使用关系；优化测试应以实际启用的 pass 为准。

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

当前实现仍保留实验性寄存器和栈帧策略，调用约定、被调用者保存寄存器、spill 以及混合整数/浮点参数应作为重点测试对象。

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

前端 AST 当前通过 `dump()` 直接输出 Koopa IR 文本，后端阶段再由 `src/ir` 重新解析该文本。因此 `parse.kp` 是两阶段之间的中间产物，默认写入test文件所在工作目录。

## 构建与使用

构建脚本为 `build.sh`，依赖 Bash、Flex、Bison 和支持 C++17 的 Clang：

```bash
chmod +x build.sh
./build.sh
```

脚本会生成 build/ 下的目标文件，并将可执行文件链接为 test/compiler。如果编译器、Flex 或 Bison 不在 PATH 中，脚本会失败。

编译器命令行形式：

```bash
test/compiler -S <input.c> -o <output.s>
```

`-O*` 参数目前会被命令行解析但不会选择不同的优化等级。未指定 `-S` 时，程序只完成前端 IR 文件生成，不输出汇编。