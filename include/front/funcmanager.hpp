#pragma once
#ifndef FUNCMANAGER_HPP
#define FUNCMANAGER_HPP
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

#include "ast.hpp"

using namespace std;

// 函数管理工具
class FuncManager{
public:
    //decl_func_table定位函数及类型
    unordered_map<string, Btype> decl_func_table;
    //函数栈保存函数名 , 打印函数调用时使用
    vector<string> func_stack;
    //函数参数标志   T or F
    vector<bool> param_flag; 

    FuncManager(){
        param_flag.push_back(false);
    }
    //添加map(name,type)到decl_func_table
    void add_Func(string name, Btype type);
    Btype get_Functype(string name);
    //打印函数调用时使用  ,  将name加入func_stack
    void EnterStack(string name);
    //打印函数调用时使用  ,  从func_stack末尾弹出 num个函数名并打印
    void PopStack(int num);
    void printSysyDeclFunc();
    void startCall(){
        param_flag.push_back(true);
    }
    void endCall(){
        param_flag.pop_back();
    }

    bool isParam(){
        return param_flag.back();
    }
    
};


#endif  // FUNCMANAGER_HPP