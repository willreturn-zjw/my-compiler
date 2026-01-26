#pragma once
#ifndef DFA
#define DFA

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unordered_set>
#include "../ir/datadef.hpp"
#include "../ir/koopa.h"
#include "../ir/koopaAST.hpp"
#define MAX_SIZE 1000

#define DEF 1   //定义、常量
#define NAC 2   //非常量
#define UNDEF 0

// 每条指令的数据流分析结果
class ValueInfo
{
public:
    koopa_raw_value_data *value;
    int8_t *in_vector;
    int8_t *out_vector;
    std::unordered_set<ValueInfo *> pred;
    std::unordered_set<ValueInfo *> succ;
    std::unordered_set<int> def_set;
    std::unordered_set<int> use_set;
    // 指令的行号
    int index = -1;
    ValueInfo(koopa_raw_value_data *v, int var_num);
};

// 循环判断
struct scc
{
    int l = INT32_MAX;
    int h = -1;
    int size = 0;
};

// 活变量分析
void set_all_prev_next(Program *program);
void set_var_index();
void Live_Var(Program *program);
void Union_for_LV(int8_t *arr1, int8_t *arr2, int n, int control_bit);
bool LV_for_one_inst(koopa_raw_value_data *inst, bool visited);
bool LV_for_one_BB(BBInfo &BB);
void Live_for_func(FuncInfo *func);

/// 获取相关成员的函数
int8_t *get_inst_vector(koopa_raw_value_data &inst, bool in);
int8_t *get_inst_vector(koopa_raw_value_t &inst, bool in);
int get_var_index(const koopa_raw_value_t &value);

std::string ty2str(const koopa_raw_type_t &ty);

// 活跃区间分析
void tarjan(BBInfo *b);
void interval_analysis(FuncInfo *func);
void calculate_intervals(FuncInfo *func);
void number();
void linearize(FuncInfo *func);
void topo_sort(BBInfo *BB);
#endif