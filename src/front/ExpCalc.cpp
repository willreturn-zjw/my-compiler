#pragma once
#include <iostream>

#include "../../include/front/ast.hpp"
#include "../../include/front/variable.hpp"

extern int now;
extern Val_Table val_table;
extern Btype var_type;

int ConstDefAST::calc(){
    val_table.Record(ident,initval->calc(),true,type);
    //val_table.Record(ident,initval->fcalc(),true,type);
    return val_table.get(ident).value;
}

double ConstDefAST::fcalc(){
    val_table.Record(ident,float(initval->fcalc()),true,type);
    //val_table.Record(ident,initval->calc(),true,type);
    return val_table.get(ident).fvalue;
}

//*********下面几个是高层节点，只调用子表达式的(f)calc计算***************************
int ConstExpAST::calc(){
  
    return exp->calc();
}

double ConstExpAST::fcalc() {
    return exp->fcalc();
}

int ExpAST::calc(){
    return exp->calc();
}

double ExpAST::fcalc() {
    return exp->fcalc();
}


int BinaryExpAST::calc(){
    Btype tytmp1 = exp1->val_type;
    Btype tytmp2 = exp2->val_type;
    double tmp1,tmp2;
    tmp1 = (tytmp1 == Btype::BINT)? exp1->calc() : exp1->fcalc();
    tmp2 = (tytmp2 == Btype::BINT)? exp2->calc() : exp2->fcalc();
    switch(op2){
        case op::ADD:
            //std::cout<<"//"<<tmp1<<" + "<<tmp2<<std::endl;  //debug
            return tmp1+tmp2;
        case op::SUB:
            return tmp1-tmp2;
        case op::MUL:
            return tmp1*tmp2;
        case op::DIV:
            return tmp1/tmp2;
        case op::MOD:
            return (int)tmp1% (int)tmp2;
        case op::GT:
            return tmp1>tmp2;
        case op::LT:
            return tmp1<tmp2;
        case op::GE:
            return tmp1>=tmp2;
        case op::LE:
            return tmp1<=tmp2;
        case op::EQ:
            return tmp1==tmp2;
        case op::NE:
            return tmp1!=tmp2;
        case op::AND:
            return tmp1&& tmp2;
        case op::OR:
            return tmp1|| tmp2;
        default:
            //后面看清况再说
            std::cout<<"not defiend calc"<<std::endl;
            return -1;
            break;
    }
}

double BinaryExpAST::fcalc() {
    Btype tytmp1 = exp1->val_type;
    Btype tytmp2 = exp2->val_type;
    double tmp1,tmp2;
    tmp1 = (tytmp1 == Btype::BINT)? exp1->calc() : exp1->fcalc();
    tmp2 = (tytmp2 == Btype::BINT)? exp2->calc() : exp2->fcalc();
    switch (op2) {
        case op::ADD:
            return tmp1 + tmp2;
        case op::SUB:
            return tmp1 - tmp2;
        case op::MUL:
            return tmp1 * tmp2;
        case op::DIV:
            return tmp1 / tmp2;
        case op::GT:
            return tmp1 > tmp2;
        case op::LT:
            return tmp1 < tmp2;
        case op::GE:
            return tmp1 >= tmp2;
        case op::LE:
            return tmp1 <= tmp2;
        case op::EQ:
            return tmp1 == tmp2;
        case op::NE:
            return tmp1 != tmp2;
        case op::AND:
            return tmp1 && tmp2;
        case op::OR:
            return tmp1 || tmp2;
        default:
            std::cout << "not defined fcalc" << std::endl;
            return -1.0f;
    }
}

//一元运算 直接计算
int UnaryExpAST::calc(){
    int tmp=exp1->calc();
    if(op1==op::SUB){
        return -tmp;
    }
    else if(op1==op::NOT){
        return !tmp;
    }
    return tmp;
}
double UnaryExpAST::fcalc() {
    double tmp = exp1->fcalc();
    if (op1 == op::SUB) {
        return -tmp;
    } else if (op1 == op::NOT) {
        return !tmp;
    }
    return tmp;
}

//只有常量时才能调用
int LValAST::calc(){
    if(calc_f==0)
    {
        std::cout<<val_table.Get_Name(this->ident)<<std::endl;
        std::cout<<val_table.get(ident).const_var<<std::endl;
        std::cout<<"error: can't be directly calculated"<<std::endl;
        throw std::runtime_error("error: can't be directly calculated");
    }
    const auto& info=val_table.get(ident);
    return info.const_var ? info.value : info.known_value;
}

double LValAST::fcalc() {
    if (calc_f == 0) {
        std::cout << val_table.Get_Name(this->ident) << std::endl;
        std::cout << val_table.get(ident).const_var << std::endl;
        std::cout << "error: can't be directly calculated" << std::endl;
        throw std::runtime_error("error: can't be directly calculated");
    }
    const auto& info=val_table.get(ident);
    return info.const_var ? info.fvalue : info.known_fvalue;
}

int OptionExpAST::calc(){
    if(exp){
        return exp->calc();
    }
    return 0;
}

double OptionExpAST::fcalc() {
    if (exp) {
        return exp->fcalc();
    }
    return 0.0f;
}


//***************************************************************************************************
//***********更新是否可以计算的符号位*************************************************************
//***************************************************************************************************
void ConstExpAST::up_calc(){
    exp->up_calc();
    calc_f  = exp->calc_f;
    val_type= exp->val_type;
}

void ExpAST::up_calc(){
    exp->up_calc();
    calc_f  = exp->calc_f;
    val_type= exp->val_type;
}

void BinaryExpAST::up_calc(){
    exp1->up_calc();
    exp2->up_calc();
    calc_f=exp1->calc_f&&exp2->calc_f;
    switch (op2){ //根据运算符更新该式子的计算值类型
    case ADD:
    case SUB:
    case MUL:
    case DIV:
        val_type = (exp1->val_type==BFLOAT || exp2->val_type==BFLOAT)? val_type=BFLOAT : val_type=BINT;
        break;
    case MOD: // 取余运算都是整数
    case GT: 
    case LT:
    case LE:
    case GE:
    case EQ:
    case NE:
    case AND:
    case OR:
    case NOT:
        val_type = BINT;
        break;
    default:
        val_type = BVOID;
        break;
    }
    
}
void UnaryExpAST::up_calc(){
    exp1->up_calc();
    calc_f=exp1->calc_f;
    if(op1==op::NOT) val_type=BINT;
    else  val_type= exp1 ->val_type;
}
void NumberAST::up_calc(){
    calc_f=true;
}
void LValAST::up_calc(){
    if(ident!=""){
        const auto& info=val_table.get(ident);
        calc_f=info.const_var || info.known_from_initializer;
        val_type=info.type;
    }else{
        calc_f=false;
        val_type = BVOID;
    }
}

void OptionExpAST::up_calc(){
    if(exp){
        exp->up_calc();
        calc_f=exp->calc_f;
        val_type= exp->val_type;
    }
    else{
        calc_f=false;
        val_type = BVOID;
    }
}


///****常量初始化值计算*********************** 
//递归调用确定该量是否可计算值
void ConstInitValsAST::up_calc(){
    if(constexp){
        constexp->up_calc();
        calc_f=constexp->calc_f;
        val_type= constexp->val_type;
    }
    else{
        calc_f=false;
        val_type = BVOID;
    }
}

int ConstInitValsAST::calc(){
    if(calc_f)
        return constexp->calc();
    else{
        throw std::runtime_error("error: can't be directly calculated");
    }
}

double ConstInitValsAST::fcalc(){
    if(calc_f)
        return constexp->fcalc();
    else{
        throw std::runtime_error("error: can't be directly calculated");
    }
}

