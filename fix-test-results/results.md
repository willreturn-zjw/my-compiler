# 自动化测试结果

总用例数：30
通过：27，失败：3，阻塞：0

| ID | 阶段 | 标题 | 状态 | 耗时(ms) | 详情 |
|---:|---|---|---|---:|---|
| 1 | frontend | 关键字、符号与分隔符识别 | PASS | 20 | 所有静态断言通过 |
| 2 | frontend | 注释与空白处理 | PASS | 1 | 所有静态断言通过 |
| 3 | frontend | 全局变量、声明与函数定义解析 | PASS | 1 | 所有静态断言通过 |
| 4 | frontend | 整数运算优先级与常量折叠 | PASS | 1 | 所有静态断言通过 |
| 5 | frontend | 整数与浮点字面量 | PASS | 1 | 所有静态断言通过 |
| 6 | frontend | 常量、局部作用域与遮蔽 | PASS | 1 | 所有静态断言通过 |
| 7 | frontend | 非 void 函数定义与调用 | PASS | 1 | 所有静态断言通过 |
| 8 | frontend | if/else 控制流生成 | PASS | 1 | 所有静态断言通过 |
| 9 | frontend | 整数与浮点边界字面量 | PASS | 1 | 所有静态断言通过 |
| 10 | frontend | 长度为 1 的数组类型 | PASS | 1 | 所有静态断言通过 |
| 11 | frontend | 1x1 多维数组和下标 0 | PASS | 1 | 所有静态断言通过 |
| 12 | frontend | 多维数组部分初始化与补零 | PASS | 1 | 所有静态断言通过 |
| 13 | frontend | Koopa 指令链构造 | PASS | 1 | 所有静态断言通过 |
| 14 | frontend | while、continue、break 的 CFG | PASS | 1 | 所有静态断言通过 |
| 15 | frontend | 数组形参与 getptr/getelemptr | PASS | 2 | 所有静态断言通过 |
| 16 | backend | 整数常量返回 | PASS | 1 | 所有静态断言通过 |
| 17 | backend | 负数及 32 位边界常量 | PASS | 1 | 所有静态断言通过 |
| 18 | backend | 整数基本算术运算 | PASS | 1 | 所有静态断言通过 |
| 19 | backend | 有符号除法与取模 | PASS | 2 | 所有静态断言通过 |
| 20 | backend | 整数比较运算 | PASS | 2 | 所有静态断言通过 |
| 21 | backend | 浮点算术与比较 | PASS | 2 | 所有静态断言通过 |
| 22 | backend | 局部变量分配与读写 | PASS | 1 | 所有静态断言通过 |
| 23 | backend | 全局变量与全局数组 | PASS | 1 | 所有静态断言通过 |
| 24 | backend | 一维数组边界地址计算 | PASS | 1 | 所有静态断言通过 |
| 25 | backend | 多维数组地址计算 | PASS | 1 | 所有静态断言通过 |
| 26 | backend | 条件分支真假路径 | PASS | 1 | 所有静态断言通过 |
| 27 | backend | 循环回边与退出控制流 | PASS | 1 | 所有静态断言通过 |
| 28 | backend | 整数参数传递边界 | PASS | 1 | 所有静态断言通过 |
| 29 | backend | 浮点及混合参数传递 | PASS | 2 | 所有静态断言通过 |
| 30 | backend | 高寄存器压力与 spill/reload | PASS | 2 | 所有静态断言通过 |