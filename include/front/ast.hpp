#pragma once
#ifndef AST_HPP
#define AST_HPP

#include <memory>
#include <string>
#include <vector>
#include<iostream>



enum op{ADD,SUB,MUL,DIV,MOD,GT,LT,LE,GE,EQ,NE,AND,OR,NOT,NONE};
enum Btype{BINT,BFLOAT,BCHAR,BDOUBLE,BVOID,BINVALID};
// 所有 AST 的基类
class BaseAST {
 public:
  virtual ~BaseAST() = default;
  virtual void dump() = 0;
  //计算表达式的int值
  virtual int calc() {return 0;};
  //计算表达式的float值
  virtual double fcalc() {return 0.0f;};
  virtual void up_calc(){;};
  bool calc_f = false; //是否可以计算值的标志flag
  Btype val_type;  //区分计算的值的类型：int float
};

// CompUnit 是 BaseAST
class CompUnitAST : public BaseAST {
 public:
  // 用智能指针管理对象
  std::unique_ptr<BaseAST> start;
  void dump();
};

//DefUnits 也是 BaseAST
class DefUnitsAST : public BaseAST {
 public:
  std::vector<std::unique_ptr<BaseAST>> unit_list;
  void dump();
};

//DefUnit 也是 BaseAST
class DefUnitAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> content;
  void dump();
};

// FuncDef 也是 BaseAST
class FuncDefAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> type;
  std::string ident;
  std::unique_ptr<BaseAST> params;
  std::unique_ptr<BaseAST> block;
  void dump();
};

class FuncFParamsAST:public BaseAST{
  public:
  std::vector<std::unique_ptr<BaseAST>> paramlist;
  void dump();
};

class FuncFParamAST:public BaseAST{
  public:
  Btype type;
  std::string ident;
  std::unique_ptr<BaseAST> arraydef;
  void dump();
  void array_dump();
  void variable_dump();
};

class FuncArrayAST:public BaseAST{
  public:
  std::string ident;
  std::vector<std::unique_ptr<BaseAST>> dimon_list;
  void dump(){};
};

class FuncCallAST: public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> params;
  void dump();
};

//函数调用相关的ast(R做标记，右值)
class FuncRParamsAST:public BaseAST{
  public:
  std::vector<std::unique_ptr<BaseAST>> paramlist;
  void dump();
};

// DeclarationType 也是 BaseAST
class DeclarationTypeAST: public BaseAST {
 public:
  Btype type;
  void dump();
};

// BasicType 也是 BaseAST

//Block 也是 BaseAST
class BlockAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> blockitems;
  void dump();
};

//Stmt 也是 BaseAST
class StmtAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> sent;
  void dump();
};

//Sents 也是 BaseAST
class BlockItemsAST : public BaseAST {
 public:
  std::vector<std::unique_ptr<BaseAST>> itemlist;
  void dump();
};

//Sent 也是 BaseAST
class BlockItemAST : public BaseAST {
 public:
  std::unique_ptr<BaseAST> content;
  void dump();
};
//AssignmentAST 也是 BaseAST
class AssignsAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> assigns;
  std::unique_ptr<BaseAST> assign;
  void dump();
};

class AssignAST : public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> arraydef;
  Btype exp_type;
  void dump();
  void array_dump();
  void variable_dump();
};

//if/else的AST
class IfElseAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> then_part;
  std::unique_ptr<BaseAST> else_part;
  void dump();
};

//while的AST
class WhileAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  std::unique_ptr<BaseAST> body;
  void dump();
};

class BreakAST: public BaseAST {
 public:
  void dump();
};

class ContinueAST: public BaseAST {
 public:
  void dump();
};

class ConstDeclListAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> constdefs;
  Btype type;
  void dump();
};

class VarDeclListAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> vardefs;
  Btype type;
  //TODO
  void dump();
};

class ConstDefsAST: public BaseAST {
 public:
 Btype type;
  std::vector<std::unique_ptr<BaseAST>> constdefs;
  //TODO
  void dump();
};

class ConstInitValsAST : public BaseAST{
  public:
  std::unique_ptr<BaseAST> constexp;
  std::vector<std::unique_ptr<BaseAST>> array_val_list;
  void dump() override;
  void up_calc() override;
  int calc() override;
  double fcalc() override;
};

class InitValsAST : public BaseAST{
  public:
  std::unique_ptr<BaseAST> exp;
  std::vector<std::unique_ptr<BaseAST>> array_val_list;
  bool is_undef=false;
  void dump(){};
};

class VarDefsAST: public BaseAST {
 public:
  Btype type;
  std::vector<std::unique_ptr<BaseAST>> vardefs;
  //TODO
  void dump();
};

class ConstDefAST: public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> arraydef;
  Btype type;
  std::unique_ptr<BaseAST> initval;
  int value;
  float fvalue;
  //TODO
  void dump() override;
  int calc() override;
  double fcalc() override;
}; 

class VarDefAST: public BaseAST {
 public:
  std::string ident;
  Btype type;
  std::unique_ptr<BaseAST> arraydef;
  std::unique_ptr<BaseAST> initval;
  //TODO
  void dump() override;
  void array_dump();
  void variable_dump();
};

class ArrayDefsAST: public BaseAST {
 public:
  std::string ident; 
  std::vector<std::unique_ptr<BaseAST>> dimon_list;
  void dump(){};
};

class ExpAST:public BaseAST{
  public:
    std::unique_ptr<BaseAST> exp;
    void dump() override;
    void up_calc() override;
    int calc() override;
    double fcalc() override;
};

class ConstExpAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void up_calc() override;
  void dump() override;
  int calc() override;
  double fcalc() override;
}; 

class ReturnAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> retNum;
  void dump();
};

//TODO:解决了么？
//二元操作的AST
class BinaryExpAST: public BaseAST {
 public:
  //第一个操作数
  std::unique_ptr<BaseAST> exp1;
  //操作符
  op op2;
  //第二个操作数
  std::unique_ptr<BaseAST> exp2;
  void dump() override;
  //TODO:直接运算 
  int calc() override;
  double fcalc() override;
  void up_calc() override;
};


class UnaryExpAST: public BaseAST {
 public:
  op op1;
  std::unique_ptr<BaseAST> exp1;
  void dump() override;
  //Todo:直接运算
  int calc() override;
  double fcalc() override;
  void up_calc() override;
};


//TODO:需要实现
class NumberAST: public BaseAST {
 public:
  int value;
  float fvalue;
  
  void dump() override;
  int calc() override{return value;}
  double fcalc() override{return fvalue;}
  void up_calc() override;
};

class LValAST: public BaseAST {
 public:
  std::string ident;
  std::unique_ptr<BaseAST> arraydef;
  void dump() override;
  int calc() override;
  double fcalc() override;
  void Array_dump();
  void Variable_dump();
  void up_calc() override;
};

class LArrayAST: public BaseAST {
 public:
  std::string ident;
  std::vector<std::unique_ptr<BaseAST>> dimon_list;
  void dump() {};
};

class OptionExpAST: public BaseAST {
 public:
  std::unique_ptr<BaseAST> exp;
  void dump() override;
  void up_calc() override;
  int calc() override;
  double fcalc() override;
};

std::string floatToBinary(float num);

#endif  // AST_HPP