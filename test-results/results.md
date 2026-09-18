# 自动化测试结果

总用例数：46
通过：41，失败：5，阻塞：0

| ID | 阶段 | 标题 | 状态 | 耗时(ms) | 详情 |
|---:|---|---|---|---:|---|
| 1 | frontend | 关键字、符号与分隔符识别 | PASS | 10 | 所有静态断言通过 |
| 2 | frontend | 注释与空白处理 | PASS | 3 | 所有静态断言通过 |
| 3 | frontend | 全局变量、声明与函数定义解析 | PASS | 2 | 所有静态断言通过 |
| 4 | frontend | 整数运算优先级与常量折叠 | PASS | 2 | 所有静态断言通过 |
| 5 | frontend | 整数与浮点字面量 | PASS | 1 | 所有静态断言通过 |
| 6 | frontend | 常量、局部作用域与遮蔽 | PASS | 2 | 所有静态断言通过 |
| 7 | frontend | 非 void 函数定义与调用 | PASS | 1 | 所有静态断言通过 |
| 8 | frontend | if/else 控制流生成 | PASS | 1 | 所有静态断言通过 |
| 9 | frontend | 整数与浮点边界字面量 | PASS | 2 | 所有静态断言通过 |
| 10 | frontend | 长度为 1 的数组类型 | PASS | 1 | 所有静态断言通过 |
| 11 | frontend | 1x1 多维数组和下标 0 | PASS | 1 | 所有静态断言通过 |
| 12 | frontend | 多维数组部分初始化与补零 | PASS | 1 | 所有静态断言通过 |
| 13 | frontend | Koopa 指令链构造 | PASS | 2 | 所有静态断言通过 |
| 14 | frontend | while、continue、break 的 CFG | PASS | 2 | 所有静态断言通过 |
| 15 | frontend | 数组形参与 getptr/getelemptr | PASS | 2 | 所有静态断言通过 |
| 16 | backend | 整数常量返回 | PASS | 1 | 所有静态断言通过 |
| 17 | backend | 负数及 32 位边界常量 | PASS | 1 | 所有静态断言通过 |
| 18 | backend | 整数基本算术运算 | FAIL | 1 | 缺少: sdiv |
| 19 | backend | 有符号除法与取模 | PASS | 2 | 所有静态断言通过 |
| 20 | backend | 整数比较运算 | PASS | 1 | 所有静态断言通过 |
| 21 | backend | 浮点算术与比较 | PASS | 1 | 所有静态断言通过 |
| 22 | backend | 局部变量分配与读写 | PASS | 1 | 所有静态断言通过 |
| 23 | backend | 全局变量与全局数组 | PASS | 1 | 所有静态断言通过 |
| 24 | backend | 一维数组边界地址计算 | PASS | 2 | 所有静态断言通过 |
| 25 | backend | 多维数组地址计算 | PASS | 1 | 所有静态断言通过 |
| 26 | backend | 条件分支真假路径 | PASS | 1 | 所有静态断言通过 |
| 27 | backend | 循环回边与退出控制流 | PASS | 2 | 所有静态断言通过 |
| 28 | backend | 整数参数传递边界 | PASS | 2 | 所有静态断言通过 |
| 29 | backend | 浮点及混合参数传递 | PASS | 2 | 所有静态断言通过 |
| 30 | backend | 高寄存器压力与 spill/reload | PASS | 2 | 所有静态断言通过 |
| 31 | backend | 逻辑与、逻辑或和逻辑非的短路控制流 | PASS | 3 | 所有静态断言通过 |
| 32 | backend | 整数与浮点混合运算及比较转换 | FAIL | 2 | AssertionError('缺少模式：^\\s*fcmp\\w*\\s+') |
| 33 | backend | void 函数调用与隐式返回 | PASS | 2 | 所有静态断言通过 |
| 34 | backend | 整数与浮点 I/O 及计时函数调用 | PASS | 3 | 所有静态断言通过 |
| 35 | backend | 递归函数的返回值、分支与链接寄存器保护 | PASS | 2 | 所有静态断言通过 |
| 36 | backend | 嵌套多函数调用与中间返回值保存 | PASS | 2 | 所有静态断言通过 |
| 37 | backend | 全局常量、零初始化标量和零初始化数组 | FAIL | 1 | AssertionError('缺少全局对象 g') |
| 38 | backend | 大局部数组的首尾元素读写 | FAIL | 2 | AssertionError('缺少至少 4096 字节且 16 字节对齐的栈帧') |
| 39 | backend | 9 个整数与 9 个浮点混合参数溢出到栈 | PASS | 3 | 所有静态断言通过 |
| 40 | backend | 浮点函数返回值参与后续比较 | FAIL | 2 | AssertionError('缺少模式：^\\s*fcmp\\w*\\s+') |
| 41 | backend | 内层 break/continue 的最近循环目标 | PASS | 2 | 所有静态断言通过 |
| 42 | backend | 菱形 CFG 合流后的变量定义与使用 | PASS | 2 | 所有静态断言通过 |
| 43 | backend | 非法 SysY 语法的诊断与产物阻断 | PASS | 1 | 所有静态断言通过 |
| 44 | backend | 无 -S、-O 等级与指定输出路径 | PASS | 3 | 所有静态断言通过 |
| 45 | backend | 含空格的输入和汇编输出文件名 | PASS | 1 | 所有静态断言通过 |
| 46 | backend | 同目录连续编译的 IR 覆盖与状态重置 | PASS | 4 | 所有静态断言通过 |
