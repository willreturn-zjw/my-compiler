#pragma once
#include <string>
#include <iostream>
#include <cassert>
#include <map>
#include <stack>
#include <iomanip> // for std::setw
#include <list>
#include <queue> // For std::priority_queue
#include "../ir/koopa.h"
#include "../opt/dfa.hpp"

#define w_regs_num 29
// w16 w17 w18 w19 w29 w30 w31不用
#define zero_w_reg_num 4
#define w_regs_reserved 8
#define w_regs_for_arg 8

#define s_regs_num 32
#define zero_s_reg_num 1
#define s_regs_reserved 8
#define s_regs_for_arg 8

#define USE_INT_REG 1
#define USE_FLOAT_REG 2




// 定义一个别名，用于表示 low 和 high 配对
using Pair = std::pair<int, int>;


// 分配结果表
extern std::unordered_map<koopa_raw_value_t, std::string> reg_alloc_map;

void eviction(int l);
void eviction_for_one(std::priority_queue<LiveRange *, std::vector<LiveRange *>> &occupied_intervals, int l);
void try_spill(LiveRange *lr, std::priority_queue<LiveRange *, std::vector<LiveRange *> > &occupied_intervals);
std::string get_reg_of_value(const koopa_raw_value_t &value);
void reg_alloc(FuncInfo *func);
void arrange_win_seq();
void update_win(std::string &reg, LiveRange *lr);
void print_win_seq(FuncInfo *func);

