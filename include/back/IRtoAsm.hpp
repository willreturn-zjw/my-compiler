#pragma once

#ifndef IRTOASM_HPP
#define IRTOASM_HPP

#include <string>
#include <iostream>
#include <cassert>
#include <map>
#include <stack>
#include <iomanip> // for std::setw
#include <list>
#include "../ir/koopa.h"
#include "../ir/build.hpp"
#include "../opt/dfa.hpp"
#include "../opt/optimizer.hpp"
#include "../back/regalloc.hpp"
extern std::unique_ptr<Program> program;
void Visit_top_program(const koopa_raw_program_t &program);
void Visit_raw_slice(const koopa_raw_slice_t &slice);
void Visit_func(const koopa_raw_function_t &func);
void Visit_bb(const koopa_raw_basic_block_t &bb);
void Visit_inst(const koopa_raw_value_t &value);
void Visit_ret(const koopa_raw_return_t &ret , const koopa_raw_value_t & value);
std::string Visit_const_int(const koopa_raw_integer_t &integer); 
std::string Visit_const_float(const koopa_raw_floatnum_t &floatnum);
void Visit_binary(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value, const uint32_t use_count);
void Visit_alloc(const koopa_raw_value_t &value);
void Visit_store(const koopa_raw_store_t &store);
void Visit_load(const koopa_raw_load_t &load, const koopa_raw_value_t &value);
void Visit_branch(const koopa_raw_branch_t &branch);
void Visit_jump(const koopa_raw_jump_t &jump);
void Visit_call(const koopa_raw_call_t &call, const koopa_raw_value_t &value);
void Visit_global_alloc(const koopa_raw_global_alloc_t &global_alloc, const koopa_raw_value_t &value);
void Visit_elem_ptr(const koopa_raw_get_elem_ptr_t &get_elem_ptr, const koopa_raw_value_t &value);
void Visit_ptr(const koopa_raw_get_ptr_t &get_ptr, const koopa_raw_value_t &value);
int cal_array_length(const koopa_raw_type_kind &value);
void analysis_aggregate(const koopa_raw_aggregate_t &aggregate, std::list<int32_t> &init_list );
void analysis_float_aggregate(const koopa_raw_aggregate_t &aggregate, std::list<float> &init_list);
void store_array(const koopa_raw_value_t &value, const koopa_raw_value_t &dest);
int IRanalysis(std::string inputfile, std::string outputfile);
int get_value_type(const koopa_raw_type_t &type);
int get_param_type(const koopa_raw_type_t &type);
void regstack_push(std::string r);
std::string regstack_pop(int flag);
void value_to_reg(std::string dest_reg, const koopa_raw_value_t &src);
std::string ty2str(const koopa_raw_type_t &ty);
int getPowerOfTwo(int n);


void imm2reg(const koopa_raw_value_t &value, std::string &dest_reg);
std::string prep_operand(const koopa_raw_value_t &operand, std::string dest_reg, const koopa_raw_value_t &value);
std::string int2float_reg(int type, bool is_imm, float float_num, std::string &i_reg);
void store_regs();
void restore_regs();
bool is_temp_reg(const std::string &reg);
void update_reg_value_map(const koopa_raw_value_t &value, std::string reg);
void mov_between_any_regs(std::string dest_reg, std::string src_reg);
void str_print(int dest_type, int src_type, std::string dest_reg, std::string reg);
void ldr_print(int value_type, int mem_type, std::string src_reg, std::string dest_reg);


#endif  // #endif  // BLOCK_HPP

