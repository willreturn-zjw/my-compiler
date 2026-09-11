# my-compiler

这是一个使用 C++ 编写的 SysY 编译器实验项目。当前代码链路覆盖：SysY 源程序的词法/语法分析、AST 构建、Koopa 风格 IR 生成与解析、部分数据流分析，以及 RISC-V 汇编生成。

## 整体流程

```mermaid
flowchart LR
    A[SysY 源文件 .c] --> B[src/front\nFlex/Bison + AST]
    B --> C[parse.kp\nKoopa 风格文本 IR]
    C --> D[src/ir\nLexer + Parser + Builder]
    D --> E[Program / FuncInfo / BBInfo / SymInfo]
    E --> F[src/opt\n活跃变量、区间、used-by 分析]
    F --> G[src/back\nIR 到 RISC-V 汇编]
    G --> H[输出 .s 文件]
```

程序入口是 `src/main.cpp`。默认流程只进行前端分析并写出 `parse.kp`；指定 `-S` 后，会继续执行 IR 解析、优化分析和汇编生成。

## 目录说明

| 目录/文件 | 作用 |
| --- | --- |
| `src/front/sysy.l` | SysY 词法规则；修改后需要重新运行 Flex |
| `src/front/sysy.y` | SysY Bison 语法规则和 AST 节点构造 |
| `src/front/ast.cpp` | AST dump、符号使用、前端 IR 文本生成 |
| `src/front/variable.cpp` | 作用域符号表、常量/变量、数组初始化和数组寻址 |
| `src/front/funcManager.cpp` | 函数声明、函数调用栈和 SysY 库函数声明 |
| `src/front/bbmanager_air.cpp` | 基本块、跳转、分支、返回指令生成 |
| `src/ir/lexer.cpp` | Koopa 风格 IR 词法分析 |
| `src/ir/parser.cpp` | Koopa 风格 IR 语法分析 |
| `src/ir/koopaAST.cpp` | IR AST 和 `Program`/函数/基本块/符号表结构 |
| `src/ir/build.cpp` | 把 IR AST 构建为 `koopa_raw_program_t` |
| `src/opt/dfa.cpp` | 控制流连接、活跃变量分析、Tarjan 环检测、活跃区间 |
| `src/opt/optimizer.cpp` | 建立 value/block 的 `used_by` 和相关 use-def 信息 |
| `src/back/IRtoAsm.cpp`、`visit.cpp` | 遍历 IR 并生成 RISC-V 汇编 |
| `build.sh` | 使用 Flex、Bison 和 clang++ 编译项目 |
| `test/` | 当前示例输入、生成的测试输出和已构建的本地二进制 |

`src/front/sysy.tab.cpp`、`src/front/sysy.tab.hpp`、`src/front/sysy.lex.cpp` 是生成文件。日常修改应优先针对 `sysy.y` 和 `sysy.l`，再通过构建脚本重新生成。

## 构建

构建脚本是 Unix shell 脚本，建议在 Linux 或 WSL 环境执行。需要安装：

- C++17 编译器，项目脚本默认使用 `clang++`；
- Flex；
- Bison。

在项目根目录执行：

```bash
chmod +x build.sh
./build.sh
```

脚本会生成 `build/` 下的目标文件，并将可执行文件链接为 `test/compiler`。如果编译器、Flex 或 Bison 不在 PATH 中，脚本会失败。

## 命令行用法

```bash
# 只运行前端，结果写入当前工作目录的 parse.kp
./test/compiler test/test.c

# 生成 RISC-V 汇编
./test/compiler -S test/test.c -o test/test.s
```

当前入口支持以下参数行为：

| 参数 | 当前行为 |
| --- | --- |
| `-S` | 继续执行 IR 解析和后端汇编生成 |
| `-o <file>` | 指定汇编输出文件；通常与 `-S` 一起使用 |
| `-O...` | 参数会被跳过，但当前 `main` 没有按级别切换优化 |
| 输入文件 | 取最后一个非选项参数作为 SysY 输入文件 |

注意：`parse.kp` 是固定文件名，并且写到进程当前工作目录；并行运行多个测试时应为每个用例准备独立工作目录，防止相互覆盖。

## 前端支持概览

当前语法文件包含 `int`、`float`、`void`，常量和变量声明，多维数组，函数定义和调用，赋值、返回、`if/else`、`while`、`break`、`continue`，以及一元、算术、关系、相等、逻辑与/或表达式。前端会对可确定的常量表达式进行折叠，对需要运行时求值的表达式生成 IR 指令。

前端维护全局可变状态，包括符号表 `val_table`、基本块管理器 `bbm`、函数管理器 `funcm` 和数组管理器 `arraym`。因此端到端测试建议每个用例启动一个新的编译器进程。

## IR 与优化测试边界

`src/ir` 目前解析的重点包括全局定义、函数声明/定义、基本块、`alloc`、`load`、`store`、`getptr`、`getelemptr`、二元表达式、函数调用、分支、跳转和返回，以及整数、浮点数、`undef`、`zeroinit`、聚合初始化值。

`src/opt/dfa.cpp` 的主要可测行为是：建立基本块前驱/后继关系，收集每条指令的 def/use，迭代计算 live-in/live-out，使用 Tarjan 算法识别循环，并据此计算活跃区间。`src/opt/dom.cpp` 当前没有生效的实现；其中的支配树/SSA 代码全部处于注释状态，不应作为当前版本的通过标准。

