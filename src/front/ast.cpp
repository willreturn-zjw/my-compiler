#include <iostream>
#include <iomanip> 
#include <bitset>
#include <cstring>
#include <vector>
#include <algorithm> // 包含 std::copy 函数
#include <iterator>  // 包含 std::ostream_iterator
#include <cstdint>  

#include "../../include/front/ast.hpp"
#include"../../include/front/variable.hpp"
#include"../../include/front/basicblock.hpp"
#include"../../include/front/funcmanager.hpp"
#include "../../include/back/koopa_print.hpp"

using namespace std;

extern int now;
extern Btype var_type;
extern Btype func_ret_type;
extern Val_Table val_table;
extern BasicBlockManager bbm;
extern FuncManager funcm;
extern ArrayManager arraym;

int VoidHasRet=0;

void CompUnitAST::dump() {
    start->dump();
}

void DefUnitsAST::dump() {
    for(auto &unit:unit_list)
    {   
        if(unit!=nullptr)
            unit->dump();
    }
}

void DefUnitAST::dump() {
    content->dump();
}

//******************函数定义*****************************************************************************************************************************
//函数定义的顶层 AST
void FuncDefAST::dump() {
    VoidHasRet=0;
    std::cout<<"fun @"<<ident<<"(";
    if(params)
        params->dump();
    std::cout<<")";
    Btype btype=dynamic_cast<DeclarationTypeAST*>(type.get())->type;
    func_ret_type = btype;
    if(btype!=Btype::BVOID){
        std::cout<<" : ";
        printType(btype);
    }
    funcm.add_Func(ident,btype); //添加map(name,type)到decl_func_table
    std::cout<<"{ "<<std::endl;

    bbm.CreateBasicBlock("entry");
    block->dump();    // BlockAST::dunm();
    if(btype==Btype::BVOID  && VoidHasRet==0){
        std::cout<<"ret"<<std::endl;
        bbm.generateRetOrJump();
    }
    std::cout<<" }"<<std::endl;
}

//函数参数打印的顶层AST函数，逐个调用vector里的每个参数
void FuncFParamsAST::dump() {
    auto it=paramlist.begin();
    for(;it!=paramlist.end();it++)
    {   
        if(it!=paramlist.begin())
            std::cout<<", ";
        (*it)->dump();
    }
}

//具体一个函数参数的打印
void FuncFParamAST::dump() { //根据arraydef判断是`数组`参数还是`变量`参数
    if(arraydef==nullptr)
        variable_dump();
    else
        array_dump();
}

void FuncFParamAST::variable_dump() {
    std::cout<<"@"<<ident<<": ";
    printType(type);   
    val_table.add_Param(ident,type);// 向val_table添加参数
}

//函数参数是数组时的处理
void FuncFParamAST::array_dump() {
    arraym.InitializeManager(type);
    arraym.addParamArray(arraydef);
    std::cout<<"@"<<arraym.get_name()<<": *"<<arraym.generateArrayType(0);
    arraym.KillManager();
}

//函数调用时使用
void FuncCallAST::dump() {
    int num=0;
    if(params){
        funcm.startCall();
        FuncRParamsAST* rparams=dynamic_cast<FuncRParamsAST*>(params.get());
        num=rparams->paramlist.size();
        params->dump();  // params 是FuncRParamsAST类型
        funcm.endCall();
    }    
    if(funcm.get_Functype(ident)==Btype::BVOID)
        std::cout<<"call @"<<ident<<"(";
    else{
        std::cout<<"%"<<now<<"= "<<"call @"<<ident<<"(";
        ++now;
    }
    if(params)
        funcm.PopStack(num);
    std::cout<<")"<<std::endl;
}

void FuncRParamsAST::dump() {
    for(auto &para:paramlist)
    {   
        para->up_calc();  //para是一个exp
        if(para->calc_f){
            //注：可能有问题
            if(para->val_type==BINT)std::cout<<"%"<<now<<"= "<<"add 0, "<<para->calc()<<std::endl;
            if(para->val_type==BFLOAT)std::cout<<"%"<<now<<"= "<<"add 0, "<<floatToBinary(para->fcalc())<<std::endl;
            now++;
        }
        else
            para->dump();
        funcm.EnterStack("%"+to_string(now-1));
    }
}



void DeclarationTypeAST::dump() {
    switch(type){
        case Btype::BINT:
            std::cout<<"i32 ";
            break;
        case Btype::BVOID:
            std::cout<<"void ";
            break;
        case Btype::BFLOAT:
            std::cout<<"f32 ";
            break;
        default:
            break;
    }
}

void BlockAST::dump() {
    val_table.EnterBlock();
    val_table.params2block(); //将函数参数'@name'新建一份 '%name'，并store进去
    if(blockitems)  //逐个打印每个item的内容 （注意这里的block不是基本块）
        blockitems->dump();
    val_table.ExitBlock();
}

void StmtAST::dump() {
    sent->dump();
}

void BlockItemsAST::dump() {
    for(auto &item:itemlist)
    {
        item->dump();  //BlockItemAST::dump();  or  StmtAST::dump();
    }
}

void ConstInitValsAST::dump(){
    constexp->dump();
}

void BlockItemAST::dump() {
    if(!bbm.CheckEndBl())
        content->dump();  //stmt or De
    // else{
    //     //debug info
    //     std::cout<<"------好像不对呢------"<<std::endl;
    //     content->dump();
    //     std::cout<<"------------------------"<<std::endl;
    // }
}


//***********************常量&变量声明*************************************************************************************************
//TODO:完成符号表后
void ConstDeclListAST::dump(){
    ConstDefsAST* defs=dynamic_cast<ConstDefsAST*>(constdefs.get());
    if(defs==nullptr){
        std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
    }
    defs->type=this->type;
    constdefs->dump();
}

void VarDeclListAST::dump(){
    //std::cout << "var_type before assign: " << var_type << std::endl;
    var_type=this->type;
    //std::cout << "var_type after assign: " << var_type << std::endl;
    vardefs->dump();
}
//TODO:完成符号表后
void ConstDefsAST::dump(){
    for(auto &constdef:constdefs)
    {
        ConstDefAST* def=dynamic_cast<ConstDefAST*>(constdef.get());
        if(def==nullptr){
            std::cerr << "Exception: " << "dynamic_cast failed"<< std::endl;
        }
        def->type=this->type;
        constdef->dump();
    }
}

void VarDefsAST::dump(){
    for(auto &vardef:vardefs){
        vardef->dump();
    }
}

//TODO:完成符号表后
void ConstDefAST::dump(){
    if(arraydef==nullptr){
        //std::cout<<"const:"<<this->ident<<"  "<<this->type<<std::endl;
        initval->up_calc();
        if(this->type==BINT)calc();
        if(this->type==BFLOAT)fcalc();
    }else{
        arraym.InitializeManager(type,true);
        arraym.addArray(arraydef);
        std::vector<std::string> arrayinitval=arraym.ConstParseArray(initval,0);

        if(val_table.get_field_idx()==0){
            std::cout<<"global ";
            std::cout<<arraym.generateAllocStatement(val_table.Get_Name(arraym.get_name()))<<", ";
            arraym.printArray(arrayinitval);
        }else{
            std::cout<<arraym.generateAllocStatement(val_table.Get_Name(arraym.get_name()))<<std::endl;
            std::cout<<"store ";
            arraym.printArray(arrayinitval);
            std::cout<<", "<<"@"<<val_table.Get_Name(arraym.get_name());
        }
        std::cout<<std::endl;     
        arraym.KillManager();  
    }
} 

//变量定义打印
void VarDefAST::dump(){
    if(arraydef==nullptr)
        variable_dump();
    else
        array_dump();
}

void VarDefAST::variable_dump(){
    type=var_type;
    //(f)value为0 表明其是一个变量
    if(type==BINT)val_table.Record(ident,0,false,type);
    if(type==BFLOAT)val_table.Record(ident,0.0f,false,type);
    //std::cout<<ident<<"  type  is  "<<var_type<<std::endl;
    //获取该变量的作用域编号，0表示在全局作用域
    if(val_table.get_field_idx()!=0){ //局部作用域
        std::cout<<"@"<<val_table.Get_Name(ident)<<" = "<<"alloc ";
        printType(type);
        std::cout<<std::endl;
        if(initval!=nullptr){
            initval->up_calc();
            if(initval->calc_f){
                if(type==BINT)
                std::cout<<"store "<<initval->calc()<<", "<<"@"<<val_table.Get_Name(ident)<<std::endl;
                if(type==BFLOAT)
                std::cout<<"store "<<floatToBinary(initval->fcalc())<<", "<<"@"<<val_table.Get_Name(ident)<<std::endl;
            }
            else{
                initval->dump();
                std::cout<<"store "<<"%"<<now-1<<", "<<"@"<<val_table.Get_Name(ident)<<std::endl;
            }
        }
    }else{ //全局作用域的变量
        std::cout<<"global "<<"@"<<val_table.Get_Name(ident)<<" = "<<"alloc ";
        type=var_type;
        printType(type);
        std::cout<<", ";
        if(initval!=nullptr){
            initval->up_calc();
            if(initval->calc_f){
                if(type==BINT)std::cout<<initval->calc()<<std::endl;
                if(type==BFLOAT)std::cout<<floatToBinary(initval->fcalc())<<std::endl;
            }
            else{
                initval->dump();
                std::cout<<"%"<<now-1<<std::endl;
            }
        }else{
            std::cout<<"zeroinit";
        }
    }
    std::cout<<std::endl;
}

void VarDefAST::array_dump(){
    arraym.InitializeManager(var_type);
    arraym.addArray(arraydef); //数组定义的 = 左边  arraydef
    InitValsAST* initvalue_ast=dynamic_cast<InitValsAST*>(initval.get()); //数组定义的 = 右边初值  initval
    std::vector<std::string> arrayinitval;
    
    //全局数组变量
    if(val_table.get_field_idx()==0){
        if(!initvalue_ast->array_val_list.empty())
            arrayinitval=arraym.VarParseArray(initval,0);
        std::cout<<"global ";
        std::cout<<arraym.generateAllocStatement(val_table.Get_Name(arraym.get_name()))<<", ";
        if(initvalue_ast->array_val_list.empty())
            std::cout<<"zeroinit";
        else
            arraym.printArray(arrayinitval);
    }else{ //局部数组变量
        if(!initvalue_ast->array_val_list.empty())
            arrayinitval=arraym.VarParseArray(initval,0);
        std::cout<<arraym.generateAllocStatement(val_table.Get_Name(arraym.get_name()))<<std::endl;
        std::cout<<"store ";
        if(initvalue_ast->array_val_list.empty()){
            if(initvalue_ast->is_undef)std::cout << "undef";
            else std::cout << "zeroinit";
            // std::cout<<"zeroinit";
        }  
        else
            {
            //std::copy(arrayinitval.begin(), arrayinitval.end(), std::ostream_iterator<std::string>(std::cout, " "));
            arraym.printArray(arrayinitval);
            }
        std::cout<<", "<<"@"<<val_table.Get_Name(arraym.get_name());
    }
    std::cout<<std::endl;
    arraym.KillManager();
}

//*****************************语句块函数if、while、continue*************************************************************** 
//ifelse块的打印
void IfElseAST::dump(){
    exp->up_calc();
    ++bbm.ifCounter;
    if(bbm.CheckEndBl()) return;
    //给br语句用的then的lable,ifCounter是避免重复命名用的
    std::string thenLable="then"+to_string(bbm.ifCounter);
    std::string elseLable="else"+to_string(bbm.ifCounter);
    std::string mergeLable="merge"+to_string(bbm.ifCounter);
    if(!else_part) elseLable=mergeLable; 
    //打印br语句
    if(exp->calc_f)
        bbm.BlockBranch(exp->calc(),thenLable,elseLable);
    else{
        exp->dump();
        bbm.BlockBranch("%"+to_string(now-1),thenLable,elseLable);
    }
    
    std::cout<<std::endl;
    //打印thenpart
    bbm.CreateBasicBlock(thenLable);
    then_part->dump();
    bbm.BlockJump(mergeLable);

    //打印elsepart
    if(else_part){
        bbm.CreateBasicBlock(elseLable);
        else_part->dump();
        bbm.BlockJump(mergeLable); 
    }

    //打印merge部分
    bbm.CreateBasicBlock(mergeLable);

}

void WhileAST::dump(){
    exp->up_calc();
    ++bbm.whileCounter;
    if(bbm.CheckEndBl()) return;
    //给br语句用的then的lable,ifCounter是避免重复命名用的
    std::string whileLable="while_entry"+to_string(bbm.whileCounter);
    std::string bodyLable="while_body"+to_string(bbm.whileCounter);
    std::string mergeLable="while_end"+to_string(bbm.whileCounter);
    //打印br语句
    bbm.BlockJump(whileLable);
    bbm.CreateBasicBlock(whileLable);
    bbm.while_stack.push_back(bbm.whileCounter);
    if(exp->calc_f)
        bbm.BlockBranch(exp->calc(),bodyLable,mergeLable);
    else{
        exp->dump();
        bbm.BlockBranch("%"+to_string(now-1),bodyLable,mergeLable);
    }
    std::cout<<std::endl;
    //打印body部分
    bbm.CreateBasicBlock(bodyLable);
    body->dump();
    bbm.BlockJump(whileLable);
    //打印merge部分
    bbm.CreateBasicBlock(mergeLable);
    bbm.while_stack.pop_back();
}

void BreakAST::dump(){
    bbm.BlockJump("while_end"+to_string(bbm.while_stack.back()));
}

void ContinueAST::dump(){
    bbm.BlockJump("while_entry"+to_string(bbm.while_stack.back()));
}

void ReturnAST::dump() {
    //此处需要更新
    if(retNum){
        retNum->up_calc();
        if(retNum->calc_f)
        //注：需要区分float,可以引入functype
            if(func_ret_type==BFLOAT)std::cout<<"ret "<<floatToBinary(retNum->fcalc())<<std::endl;
            else std::cout<<"ret "<<retNum->calc()<<std::endl;
        else{
           retNum->dump();
            std::cout<<"ret "<<"%"<<now-1<<std::endl;
        }
    }
    else{
        VoidHasRet=1;
        std::cout<<"ret"<<std::endl;
    }
        
    bbm.generateRetOrJump();
}

//*****************************赋值assign语句************************************************************************************************************* 
void AssignsAST::dump() { //这个没用，可以后续删除得

}

void::AssignAST::dump() {
    if(arraydef)
        array_dump();
    else
        variable_dump();
}

void AssignAST::variable_dump() {
    if(val_table.get(ident).const_var){
        cerr<<"this is a const,can't be assigned"<<std::endl;
    }else{
        std::string val_name;
        if(val_table.get(ident).value==0 || val_table.get(ident).fvalue==0) //变量
            val_name="@"+val_table.Get_Name(ident);
        else if(val_table.get(ident).value==1|| val_table.get(ident).fvalue==1){ //数组函数参数
            //TODO:这里后面要改,目前只有int，先这样了
            if(val_table.get(ident).type==BINT){
                //std::cout<<"b type is "<<val_table.get(ident).type<<std::endl<<std::endl<<std::endl;
                std::cout<<"%"<<ident<<"= "<<"alloc "<<"i32"<<std::endl;
            }
            if(val_table.get(ident).type==BFLOAT)
                std::cout<<"%"<<ident<<"= "<<"alloc "<<"f32"<<std::endl;
            std::cout<<"store "<<"@"<<val_table.Get_Name(ident)<<", "<<"%"<<ident<<std::endl;
            //std::cerr << val_table.Get_Name(ident) <<endl;
            val_name="%"+ident;
            val_table.valuePlus1(ident);
        }
        else{
            val_name="%"+ident;
        }
        exp->up_calc();
                // std::cout << std::fixed << std::setprecision(6);
                // std::cout<<"store "<<float(exp->calc())<<", "<<val_name<<std::endl;
                // std::cout.unsetf(std::ios::fixed);  // 取消fixed格式
        if(exp->calc_f){
            //std::cout<<"assigast:"<<std::endl;
            if(val_table.get(ident).type==BINT){
                if(exp->calc()!=-1)std::cout<<"store "<<exp->calc()<<", "<<val_name<<std::endl;
                else if(exp->fcalc()!=-1)std::cout<<"store "<<int(exp->fcalc())<<", "<<val_name<<std::endl;
                else std::cout<<"store "<<"-1"<<", "<<val_name<<std::endl;//注：
            }
            if(val_table.get(ident).type==BFLOAT){
                if(exp->fcalc()!=-1)std::cout<<"store "<<floatToBinary(exp->fcalc())<<", "<<val_name<<std::endl;
                else if(exp->calc()!=-1)std::cout<<"store "<<floatToBinary(float(exp->calc()))<<", "<<val_name<<std::endl;
                else std::cout<<"store "<<"-1.0"<<", "<<val_name<<std::endl;//注：
            }
        }
        else{
            exp->dump();
            std::cout<<"store "<<"%"<<now-1<<", "<<val_name<<std::endl;
        }
    }
}

void AssignAST::array_dump(){
    arraym.InitializeManager(var_type);
    arraym.addLArray(arraydef); //数组赋值 = 左边arraydef
    if(val_table.get(arraym.get_name()).array_const){
        cerr<<"this is a const,can't be assigned"<<std::endl;
    }
    else{ //赋值=左边
        std::string val_name;
        ///正常数组
        if(val_table.get(arraym.get_name()).value==0 || val_table.get(arraym.get_name()).fvalue==0){  //数组变量
            arraym.LArrayLoad();  //打印数组下标的那几个get_element_ptr
            val_name=arraym.get_current_ptr(); 
        }
        //函数参数数组
        else if(val_table.get(arraym.get_name()).value==1||val_table.get(arraym.get_name()).fvalue==1){ //函数数组参数
            
            arraym.ParamArrayLoad();
            val_name=arraym.get_current_ptr();
        }
        //赋值等号右边
        exp->up_calc();
        if(exp->calc_f){ //表达式的值时可以计算的
            if(exp->calc()) //计算表达式的值int
            {
                //注：可能有问题
            if(arraym.get_type()==BINT)std::cout<<"store "<<exp->calc()<<", "<<val_name<<std::endl;
            if(arraym.get_type()==BFLOAT)std::cout<<"store "<<floatToBinary(float(exp->calc()))<<", "<<val_name<<std::endl;
            }
            else 
            {
            if(arraym.get_type()==BINT)std::cout<<"store "<<int(exp->fcalc())<<", "<<val_name<<std::endl;
            if(arraym.get_type()==BFLOAT)std::cout<<"store "<<floatToBinary(exp->fcalc())<<", "<<val_name<<std::endl;
            }
        }
        else{
            exp->dump();
            std::cout<<"store "<<"%"<<now-1<<", "<<val_name<<std::endl;
        }
    }
    arraym.KillManager();
}

//***********************************************表达式****************************************************************************************************
void ExpAST::dump() {
    exp->dump();
}

void ConstExpAST::dump() {
    exp->dump();
}

//TODO:解决了么   这里的float是个大麻烦
void BinaryExpAST::dump() {
    std::string lable1="";
    std::string lable2="";
    int orCounter=0;
    int andCounter=0;
    float res1=1;
    int now1=0,now2=0;
    // exp1不可计算  并且  运算符不是and或or(这俩要短路求值)
    if((!exp1->calc_f)&&(op2!=op::AND&&op2!=op::OR)){
        exp1->dump();
        now1=now-1;
    }

    if((!exp2->calc_f)&&(op2!=op::AND&&op2!=op::OR)){
        exp2->dump();
        now2=now-1;
    }

    //执行到这里的，必是一个koopa层面的二元运算了
    // [%(i+1)] = op [%(i-1) or num] , [%i or num]
    switch(op2){
        case op::ADD:
            std::cout<<"%"<<now<<"= "<<"add ";
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<floatToBinary( exp1->fcalc());
                else  std::cout<<exp1->calc();
            }else    std::cout<<"%"<<now1;
            std::cout<<", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::SUB:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"sub "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"sub "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"sub "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::MUL:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"mul "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"mul "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"mul "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::DIV:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"div "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"div "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"div "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::MOD:
            if(exp1->calc_f)
                std::cout<<"%"<<now<<"= "<<"mod "<<exp1->calc();
            else
                std::cout<<"%"<<now<<"= "<<"mod "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f)
                std::cout<<exp2->calc();
            else
                std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::GT:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"gt "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"gt "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"gt "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::LT:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"lt "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"lt "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"lt "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::GE:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"ge "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"ge "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"ge "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::LE:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"le "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"le "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"le "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::EQ:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"eq "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"eq "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"eq "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::NE:
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"ne "<<floatToBinary( exp1->fcalc());
                else std::cout<<"%"<<now<<"= "<<"ne "<<exp1->calc();
            }else    std::cout<<"%"<<now<<"= "<<"ne "<<"%"<<now1;
            cout << ", ";
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<floatToBinary( exp2->fcalc());
                else std::cout<<exp2->calc();
            }else    std::cout<<"%"<<now2;
            std::cout<<std::endl;
            ++now;
            break;
        case op::AND:{ //短路求值啊
            // lhs: exp1
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT){
                    res1=exp1->fcalc();
                    std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<floatToBinary(res1)<<std::endl; 
                }else{
                    res1=exp1->calc();
                    std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<(int)res1<<std::endl; 
                }
                now1=now++;
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<",%"<<now1<<std::endl;
                if(!res1){
                    now++;
                    return;
                }
                now1=now++;            
            }
            else{
                exp1->dump();
                now1=now-1;

                bbm.AndBranch();
                //短路求值用这个做标记
                andCounter=bbm.andCounter;
                lable1="and1part"+to_string(andCounter);
                lable2="and2part"+to_string(andCounter);
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now1<<std::endl;
                now1=now++;
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now1<<std::endl;
                now1=now++;
                std::cout<<"%"<<"and"<<andCounter<<" = "<<"alloc i32"<<std::endl;
                std::cout<<"store"<<" %"<<now1<<", "<<"%"<<"and"<<andCounter<<std::endl;
                bbm.BlockBranch("%"+to_string(now1),lable1,lable2);
                bbm.CreateBasicBlock(lable1);
            }
            //rhs:exp2
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT)std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<floatToBinary(exp2->fcalc())<<std::endl;
                else std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<exp2->calc()<<std::endl;
            }else{
                exp2->dump();
                now2=now-1;
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now2<<std::endl;
            }
            now2=now++;
            std::cout<<"%"<<now<<"= "<<"eq "<<0<<",%"<<now2<<std::endl;
            now2=now++;
            std::cout<<"%"<<now<<"= "<<"and "<<"%"<<now1<<",%"<<now2<<std::endl;
            if(!exp1->calc_f){
                std::cout<<"store"<<" %"<<now<<", "<<"%"<<"and"<<andCounter<<std::endl;           
            }
            now++;
            if(!exp1->calc_f){
                bbm.BlockJump(lable2);
                bbm.CreateBasicBlock(lable2);
                std::cout<<"%"<<now<<"= "<<"load "<<"%"<<"and"<<andCounter<<std::endl;
                now++;
            }
            break;
        }
        case op::OR:{
            // lhs: exp1
            if(exp1->calc_f){
                if(exp1->val_type==BFLOAT){
                    res1=exp1->fcalc();
                    std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<floatToBinary(res1)<<std::endl;
                }else {
                    res1=exp1->calc();
                    std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<res1<<std::endl;
                }
                now1=now++;
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now1<<std::endl;
                if(res1){
                    now++;
                    return;
                }
                now1=now++;
            }else{
                exp1->dump();
                now1=now-1;

                bbm.OrBranch();
                orCounter=bbm.orCounter;
                //短路求值用这个做标记
                lable1="or1part"+to_string(orCounter);
                lable2="or2part"+to_string(orCounter);
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now1<<std::endl;
                now1=now++;
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now1<<std::endl;
                now1=now++;

                std::cout<<"%"<<"or"<<orCounter<<" = "<<"alloc i32"<<std::endl;
                std::cout<<"store"<<" %"<<now1<<", "<<"%"<<"or"<<orCounter<<std::endl;

                bbm.BlockBranch("%"+to_string(now1),lable1,lable2);
                bbm.CreateBasicBlock(lable2);
            }
            // rhs: exp2
            if(exp2->calc_f){
                if(exp2->val_type==BFLOAT) std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<floatToBinary(exp2->fcalc())<<std::endl;
                else std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<exp2->calc()<<std::endl;
            }
            else{
                exp2->dump();
                now2=now-1;
                std::cout<<"%"<<now<<"= "<<"eq "<<0<<", "<<"%"<<now2<<std::endl;
            }
            now2=now++;
            std::cout<<"%"<<now<<"= "<<"eq "<<0<<",%"<<now2<<std::endl;
            now2=now++;
            std::cout<<"store"<<" %"<<now-1<<", "<<"%"<<"or"<<orCounter<<std::endl;
            std::cout<<"%"<<now<<"= "<<"or "<<"%"<<now1<<",%"<<now2<<std::endl;
            if(!exp1->calc_f){
                std::cout<<"store"<<" %"<<now<<", "<<"%"<<"or"<<orCounter<<std::endl;           
            }
            now++;
            if(!exp1->calc_f){
                bbm.BlockJump(lable1);
                bbm.CreateBasicBlock(lable1);
                std::cout<<"%"<<now<<"= "<<"load "<<"%"<<"or"<<orCounter<<std::endl;
                now++;
            }
            break;
        }
        default:
            //后面看清况再说
            break;
    }
}

//TODO:解决了么？
void UnaryExpAST::dump() {
    exp1->dump();
    if(op1==op::SUB){
        std::cout<<"%"<<now<<"= "<<"sub 0,"<<"%"<<now-1<<std::endl;
        ++now;
    }
    else if(op1==op::NOT){
        std::cout<<"%"<<now<<"= "<<"eq "<<"%"<<now-1<<",0"<<std::endl;
        ++now;
    }
}

void LValAST::dump() {
    if(ident=="")
        Array_dump();
    else
        Variable_dump();
}

void LValAST::Variable_dump() {
    //value是int类型的   fvalue是float类型的   
    if(calc_f){//若左值可以计算，(f)value是其对应的值
        if(val_table.get(ident).type==BFLOAT)std::cout<<"%"<<now<<"= "<<"add 0, "<<floatToBinary(val_table.get(ident).fvalue)<<std::endl;
        else std::cout<<"%"<<now<<"= "<<"add 0, "<<val_table.get(ident).value<<std::endl;
    }
    else{//若不可计算，那么(f)value用来区分是变量还是函数参数  0时为变量  1时为数组参数
        int tmp=val_table.get(ident).value;
        float tmpf =val_table.get(ident).fvalue;
        if(tmp==0 || tmpf==0){  //变量 (int or float)
            if(val_table.get(ident).array_size==0)
                std::cout<<"%"<<now<<"= "<<"load "<<"@"<<val_table.Get_Name(ident)<<std::endl;
            else if(val_table.get(ident).array_size>0){
                std::cout<<"%"<<now<<"= "<<"getelemptr "<<"@"<<val_table.Get_Name(ident)<<", "<<0<<std::endl;
            }
        }
        else if(tmp==1 || tmpf==1){  //函数数组参数 int or float
            if(val_table.get(ident).array_size==0)
                std::cout<<"%"<<now<<"= "<<"add 0,"<<"@"<<val_table.Get_Name(ident)<<std::endl;
            else if(val_table.get(ident).array_size>0)
                std::cout<<"%"<<now<<"= "<<"getptr "<<"@"<<val_table.Get_Name(ident)<<", "<<0<<std::endl;
        }
        else{ //这里是函数参数啦
            std::cout<<"%"<<now<<"= "<<"load "<<"%"<<val_table.Get_Name(ident)<<std::endl;
        }
    } 
    ++now; 
}

void LValAST::Array_dump() {
    arraym.InitializeManager(var_type);
    arraym.addLArray(arraydef);   //arraydef.type=LArrayAST
    if(val_table.get(arraym.get_name()).value==0 || val_table.get(arraym.get_name()).fvalue==0) 
        arraym.LArrayLoad();
    else
        arraym.ParamArrayLoad();
    
    if(val_table.get(arraym.get_name()).array_size>arraym.get_ldimon().size()){
        std::cout<<"%"<<now<<" = "<<"getelemptr "<<arraym.get_current_ptr()<<", "<<0<<std::endl;
        now++;
    }else if(val_table.get(arraym.get_name()).array_size==arraym.get_ldimon().size()){
        std::cout<<"%"<<now<<" = load "<<arraym.get_current_ptr()<<std::endl;
	    now++;
    }
    arraym.KillManager();
};


void NumberAST::dump() {
    if(val_type==BFLOAT)std::cout<<"%"<<now<<"="<<"add 0, "<<floatToBinary(fvalue)<<std::endl;
    else std::cout<<"%"<<now<<"="<<"add 0, "<<value<<std::endl;
    ++now;
}

void OptionExpAST::dump() {
    if(exp){
        exp->up_calc();
        if(!exp->calc_f)
            exp->dump();
    }
}

//***********************************************辅助函数****************************************************************************************************
void set_var_type(Btype t)
{
    var_type=t;
}

//根据type输出对应类型
void printType(Btype type){
    switch(type){
        case Btype::BINT:
            std::cout<<"i32 ";
            break;
        case Btype::BVOID:
            std::cout<<"void ";
            break;
        case Btype::BFLOAT:
            std::cout<<"f32 ";
            break;
        default:
            break;
    }
}
//输出单精度浮点数前9位非0小数
std::string precise_float(float number) {
    std::ostringstream out;
    out << std::fixed << std::setprecision(8) << number; // 先设置一个足够大的精度
    std::string result = out.str();

    // 移除多余的零
    result.erase(result.find_last_not_of('0') + 1, std::string::npos);

    // 移除多余的小数点
    if (result.back() == '.') {
        result.pop_back();
    }
    // 确保最少输出一位小数
    if (result.find('.') == std::string::npos) {
        result += ".0";
    } else if (result.find('.') == result.size() - 2) {
        result += "0";
    }

    return result;
}


//浮点数在koopa ir中以2进制形式输出
std::string floatToBinary(float num) {
    uint32_t asInt;
    memcpy(&asInt, &num, sizeof(num));
    std::bitset<32> binary(asInt);
    return '#'+binary.to_string();
}