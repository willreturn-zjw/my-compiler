#pragma once
#ifndef KOOPA_PRINT_HPP
#define KOOPA_PRINT_HPP

//该文件用来输出koopa中各种数据结构
#include <string>
#include <iostream>
#include <cassert>
#include <map>
#include <stack>
#include <iomanip> // for std::setw
#include <list>

#include "../ir/koopa.h"
#include "IRtoAsm.hpp"

//打印tag类型
std::string TagToString(koopa_raw_value_tag_t tag);
std::string type_TagToString(koopa_raw_type_tag_t tag);
//std::string precise_float(float number);
//将浮点数的高16位和低16位分别转化为无符号整形数
void float2uint(float num) ;
//将浮点数转化为32位无符号整形数
void float2uint32(float num);
//打印stack_map
void print_stack_map(const std::map<koopa_raw_value_t, int>& stack_map);
//请注意string若为空调用时用“”赋值而不是用null，int用0
bool isopimm(int64_t imm, bool is64bit);

bool islogicimm(uint64_t imm);

void add_print(std::string r0,std::string r1,int64_t imm,std::string r2,bool addimm, std::string cond);

void bit_cal_print(std::string r0,std::string r1,uint64_t imm,std::string r2,bool useimm,const koopa_raw_binary_op_t op);

void mov_print(std::string dstreg,int64_t imm,std::string srcreg,bool imm2reg,std::string cond);

void printStack(std::stack<std::string> reg_stack);

void str_print(std::string r0,std::string r1,int imm,std::string r2,int find_type);

void ldr_print(std::string r0,std::string r1,int imm,std::string r2,int find_type);

void cmp_set_print(std::string r0,std::string r1,uint64_t imm,std::string r2,bool cmpimm,const koopa_raw_binary_op_t op);

void cmp_print(std::string r1,int imm,std::string r2,bool cmpimm);

void gloabal_var_load(std::string r0,const char* name);

void fmov_print(std::string dstreg,float imm,std::string srcreg,bool imm2reg,std::string cond);

void fcvt_print(std::string r0,std::string r1,bool float2int);

void print_var_index_map();

void fcmp_set_print(std::string r0,std::string r1,float imm,std::string r2,bool cmpimm,const koopa_raw_binary_op_t op);

#endif  
