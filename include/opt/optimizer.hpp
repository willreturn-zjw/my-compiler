#pragma once
#ifndef OPTIMIZER_HPP
#define OPTIMIZER_HPP

#include "../ir/koopa.h"
#include "../ir/koopaAST.hpp"

#include <iostream>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <tuple>
#include <utility>
#include <cstring>
#include <unordered_set>
#include <algorithm>


// 临时变量映射局部变量  zy_use   
extern std::unordered_map<SymInfo * , SymInfo * >  temp2local;  //局部变量包括 纯局部变量和基本块参数
// 局部变量映射临时变量列表  zyc_use  
extern std::unordered_map<SymInfo * , std::vector< std::pair< SymInfo *, BBInfo * > >  >  local2temp; 

extern std::unordered_map <koopa_raw_value_t, std::unordered_map<koopa_raw_value_t ,
                         std::vector <std::pair<BBInfo *, SymInfo * > > >> ptr_exp;


class Optimizer {
 public:
    int round = 0 ; //当前循环次数
    int Max_round = 4; //最大轮次
    Optimizer(){}

    void init_used_by(Program * program);

};





#endif  //OPTIMIZER_HPP