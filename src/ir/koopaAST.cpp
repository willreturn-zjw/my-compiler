#include "../../include/ir/koopaAST.hpp"

std::string cur_func;
std::string cur_bb;

std::unique_ptr<Program> program =std::make_unique<Program>();

koopa_raw_program_t * Program::buildon() {  //建立内存koopa.h 
    koopa_raw_program_t * raw=new koopa_raw_program_t();
    //将raw的values和funcs两个slice切片初始化好
    raw->values.kind = KOOPA_RSIK_VALUE;
    raw->funcs.kind = KOOPA_RSIK_FUNCTION;
    raw->values.len = this->global_syms.size();
    raw->funcs.len = this->functable.size();
    // 分配给buffer 空间，并将vector的转移到buffer
    raw->values.buffer = new const void*[raw->values.len ];
    raw->funcs.buffer  = new const void*[raw->funcs.len ];
    //全局变量
    for(size_t i=0; i<raw->values.len;i++){
        //补齐 inst-used_by
        koopa_raw_value_data * inst =  this->global_syms[i]->sym_addr;
        SymInfo *sym = this->global_syms[i];
        if(sym->used_by.size()>0){
                inst->used_by.kind = KOOPA_RSIK_VALUE;
                inst->used_by.len =sym->used_by.size();
                inst->used_by.buffer = new const void* [inst->used_by.len];
                for(size_t k=0; k<sym->used_by.size(); k++){
                    inst->used_by.buffer[k] = static_cast<const void*>(sym->used_by[k]);
                }
            
        }else{
            inst->used_by.kind = KOOPA_RSIK_VALUE;
        inst->used_by.len =0;
        inst->used_by.buffer = nullptr;

        }

        
        raw->values.buffer[i] = static_cast<const void*>(inst);
    }
    //指令
    for(size_t i=0; i<raw->funcs.len;i++){
        cur_func= this->functable[i]->func_name;
        //raw_func是当前函数（koopa.h）
        koopa_raw_function_data_t * raw_func = this->functable[i]->func_addr;
        raw->funcs.buffer[i] =  static_cast<const void*>(raw_func);
        // functable[i]->func_addr->bbs.buffer
        raw_func->bbs.len = this->functable[i]->bb_list.size();
        raw_func->bbs.buffer = new const void*[raw_func->bbs.len];
        for(size_t j=0; j<this->functable[i]->bb_list.size() ; j++){
            //对函数内的基本块进行操作
            cur_bb = this->functable[i]->bb_list[j]->bb_name;
            //raw_bb 是当前basic block（koopa.h）
            koopa_raw_basic_block_data_t * raw_bb = this->functable[i]->bb_list[j]->bb_addr;
            BBInfo * bbinfo_tmp = this->functable[i]->bb_list[j];
            raw_func->bbs.buffer[j] = static_cast<const void*>( raw_bb ); //补足koopa.h的bb_slice.buffer
            //补齐inst
            // raw_bb->insts
            raw_bb->insts.len = bbinfo_tmp->insts.size() ;
            raw_bb->insts.buffer = new const void*[raw_bb->insts.len];
            int idx = 0 ;
            for(koopa_raw_value_data * inst :bbinfo_tmp->insts ){
                inst->used_by.kind = KOOPA_RSIK_VALUE;
                inst->used_by.len =0 ;
                inst->used_by.buffer = nullptr ;
                //补齐 inst-used_by
                if(inst->name){
                    //这是一个变量，要建立use_by
                    SymInfo *sym = find_sym(inst->name);
                    if(sym->used_by.size()>0){
                        inst->used_by.kind = KOOPA_RSIK_VALUE;
                        inst->used_by.len =sym->used_by.size();
                        inst->used_by.buffer = new const void* [inst->used_by.len];
                        for(size_t k=0; k<sym->used_by.size(); k++){
                            inst->used_by.buffer[k] = static_cast<const void*>(sym->used_by[k]);
                        }
                    }
                }

                raw_bb->insts.buffer[idx++] = static_cast<const void*>(inst);
            }
            
        }
    }


    return raw;
}


/// opt的时候用，判断当前sym是不是全局变量
bool SymInfo::isGlobal(){
    if(this->sym_addr->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)return true;
    // if(this->sym_name.substr(0,11)=="@__filed0__") return true;
    else return false;   
}

// 用于opt ，清空当前sym的sym_addr的内容，释放内存
void SymInfo:: clear_value(){
    delete this->sym_addr->used_by.buffer;
    
    delete this->sym_addr->name;

    delete this->sym_addr;
}

/// opt的时候用，当前sym值是一个常量值时，生成一个常量value返回
koopa_raw_value_data * SymInfo::mk_const_value(){
    koopa_raw_value_data *  value = new koopa_raw_value_data();
    value->used_by.len =0;
    value->used_by.buffer = nullptr;
    value->used_by.kind = KOOPA_RSIK_VALUE;
    value->name = nullptr;
    koopa_raw_type_kind_t *  type =   new koopa_raw_type_kind_t ;
    if(this->val_tag==SymValTag::INT32 ){
        type->tag = KOOPA_RTT_INT32;
        value->ty =  type;
        value->kind.tag = KOOPA_RVT_INTEGER;
        value->kind.data.integer.value = this->def_val.integer;
    }else{
        type->tag = KOOPA_RTT_FLOAT32;
        value->ty =  type;
        value->kind.tag = KOOPA_RVT_FLOATNUM;
        value->kind.data.floatnum.value = this->def_val.float_val;
    }
    return value;
}
//删除无用分支的时候用，获取当前bb的前驱传来的基本块参数 （仅当前驱唯一时可以调用）
std::vector<SymInfo * > BBInfo::get_bb_arg_val(){
    std::vector<SymInfo * > ret ;
    //首先判断前驱是否唯一
    if(this->prev.size()!=1)return ret;
    BBInfo * prev_bb = this->prev[0];
    koopa_raw_value_data * prev_last_inst = prev_bb->insts.back();

    const void ** buffer=nullptr;
    int buf_len = -1;

    switch(prev_last_inst->kind.tag){
    case KOOPA_RVT_BRANCH:{
        //要判断当前bb是prev_bb的true还是false
        bool br_flag = false;
        if((prev_last_inst->kind.data.branch.true_bb == this->bb_addr)&&
                        (std::string(prev_last_inst->kind.data.branch.true_bb->name) ==this->bb_name)){
            br_flag = true;
            buffer = prev_last_inst->kind.data.branch.true_args.buffer;
            buf_len = prev_last_inst->kind.data.branch.true_args.len;
        }   
        else if((prev_last_inst->kind.data.branch.false_bb == this->bb_addr)&&
                        (std::string(prev_last_inst->kind.data.branch.false_bb->name) ==this->bb_name)){
            br_flag = false;
            buffer = prev_last_inst->kind.data.branch.false_args.buffer;
            buf_len = prev_last_inst->kind.data.branch.false_args.len;
        }else return ret;
        break;
    }case KOOPA_RVT_JUMP:{
        buffer = prev_last_inst->kind.data.jump.args.buffer;
        buf_len =  prev_last_inst->kind.data.jump.args.len;
        break;
    }default:{
        break;
    }}
    //基本块参数定值
    for(int i = 0 ; i < buf_len ; i++ ){
        koopa_raw_value_t tmp =reinterpret_cast<koopa_raw_value_t>( buffer[i]);
        if(tmp->name==nullptr){ //这个参数是个常量
            SymInfo * tmp_sym = new SymInfo();
            tmp_sym->cal_f = 1;
            if(tmp->kind.tag == KOOPA_RVT_INTEGER){
                tmp_sym->val_tag = SymValTag :: INT32;
                tmp_sym->def_val.integer = tmp->kind.data.integer.value;
            }else{
                tmp_sym->val_tag = SymValTag::FLOAT32;
                tmp_sym->def_val.float_val = tmp->kind.data.floatnum.value;
            }
            ret.push_back(tmp_sym);            
        }else   ret.push_back(find_sym(tmp->name));  //这个实参得是个Symdef
    }  
    return ret;
}

// // 删除当前bb，以及后续的各种关联
// void BBInfo::del_bb(){
//     //// 处理当前的前驱
//     //删除前驱对他的后继

//     //清空当前bb的prev-vector

//     // live置false

//     //// 处理当前的后继
//     //遍历next-vector，对于某一后继a ，如果a只有当前bb一个前驱，那么调用a->del_bb() ; 否则（a有多个前驱，不能删除a），删除当前bb和a之间的关系即可
    

//     //// 上述处理好了前驱后继关系 讨论基本块参数的处理
// }



// //删除branch的某一不会跳转的bb
// void BBInfo::del_br_NextBB(bool cond_val){
    
//     koopa_raw_value_data * last_inst = this->insts.back();
//     if(last_inst->kind.tag != KOOPA_RVT_BRANCH)return ;
//     koopa_raw_basic_block_data_t * del_bb = nullptr;
//     if(cond_val)del_bb = const_cast<koopa_raw_basic_block_data_t*>(last_inst->kind.data.branch.false_bb);
//     else del_bb =  const_cast<koopa_raw_basic_block_data_t*>(last_inst->kind.data.branch.true_bb);
   
//     // 要删除的bb_addr  last_inst->kind.data.branch.true_bb
//     // 遍历next后继vector，找到对应的bb
//     BBInfo * del_bbinfo = nullptr;
//     for (auto it = this->next.begin(); it != this->next.end(); ) {
//         del_bbinfo = *it;
//         if ( del_bbinfo->bb_name == std::string(del_bb->name) && del_bbinfo->bb_addr == del_bb  ) { //找到要删除的基本块了
//             // it = this->next.erase(it);  // erase返回指向下一个元素的迭代器
//             break; //找到了就不继续迭代了
//         } else  ++it;  // 仅当未删除元素时，才递增迭代器
//     }
 
//     //删除 del_bbinfo
//     del_bbinfo->del_bb();
// }

//函数名与函数地址的映射关系  以及重载std::cout<<运算
std::unordered_map<std::string, std::unique_ptr< FuncInfo> > functable ;
std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string,std::unique_ptr< FuncInfo>>& unmap) {
    for(auto & it : unmap) {
        std::cout <<"{ " <<it.first << " , "<<(void *)(it.second.get())<<" }"<<std::endl;
    }    
    return os;
}

//函数-bbname与bbname对应的bb地址映射关系    以及重载std::cout<<运算
std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<BBInfo >  > >bbtable;
std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, std::unique_ptr<BBInfo>  >& unmap) {
    for(auto & it : unmap) {
        std::cout <<"{ " <<it.first << " , "<<(void *)(it.second.get())<<" }"<<std::endl;
    }    
    return os;
}
std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, 
                        std::unordered_map<std::string, std::unique_ptr<BBInfo>  > >& unmap) {
    for(auto & it : unmap) {
        std::cout <<"{ " <<it.first <<':'<<std::endl;
        for(auto & item : it.second) {
            std::cout <<"\t{ " <<item.first << " , "<<(void *)(item.second.get())<<" }"<<std::endl;
        }    
        std::cout<<"}"<<std::endl;
    }    
    return os;
}

//符号表     函数名-变量名-value地址映射   以及重载std::cout<<运算
//全局变量时 函数名为“global”  ，其他函数名以@开头，因此可以区分
std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr< SymInfo> > >symtable;

std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string, std::unique_ptr<SymInfo> >& unmap) {
    for(const auto&  it : unmap) {
        std::cout <<"{ " <<it.first << " , "<<(void *)(it.second.get())<<" }"<<std::endl;
    }    
    return os;
}
std::ostream& operator<<(std::ostream& os, const std::unordered_map<std::string,
                        std::unordered_map<std::string, std::unique_ptr<SymInfo> > >& unmap) {
    for(const auto&  it : unmap) {
        std::cout <<"{ " <<it.first <<','<<std::endl;
        for(const auto&  item : it.second) {
            std::cout <<"\tunordered_map{ " <<item.first << " , "<<(void *)(item.second.get())<<" }"<<std::endl;
        }    
        std::cout<<"}"<<std::endl;
    }    
    return os;
}

//通过cur_func全局变量和sym找对应的符号
SymInfo * find_sym(std::string sym){
    if(symtable.find(cur_func)!= symtable.end() ){ //当前函数以及存储
        if(symtable[cur_func].find(sym)!= symtable[cur_func].end()){
            return symtable[cur_func][sym].get();
        }
    }
    if(symtable.find("Global")!= symtable.end()){ //全局变量
        if(symtable["Global"].find(sym)!= symtable["Global"].end()){
            return symtable["Global"][sym].get();
        }else return nullptr;
    }else return nullptr; //未找到
}






//**构造函数定义*************************************************************************************************
//**********************************************************************************************************
//*****      *******       ********     *******    *****   *****  *****************************************
//*****  ****  *****  ***********  *************  ******  *  ***  ****************************************
//*****      *******      ******  **************  ******  **  **  ***************************************
//*****  ****  *****  **********  ******  ******  ******  ***  *  ****************************************
//*****      *******       *******     ** *****    *****  *****   *****************************************
//*********************************************************************************************************
AstBase::AstBase(AstKind kind): kind(kind) {}





//对parser_type返回的ast进行buildon, 
koopa_raw_type_kind_t *  AstBuildHelper::type_buildon(std::unique_ptr<AstBase> & ast){
    koopa_raw_type_kind_t * ret_type=new koopa_raw_type_kind_t;

    switch(ast->kind){
    //根据ast的类型来返回type，有IntType , FloatType , ArrayType , PointerType , Funtype 几种
    case AstKind::IntType:{
        ret_type->tag=KOOPA_RTT_INT32; //union.data无意义
        break;
    }case AstKind::FloatType:{
        ret_type->tag=KOOPA_RTT_FLOAT32; //union.data无意义
        break;
    }case AstKind::ArrayType:{
        ArrayType * array = dynamic_cast<ArrayType *>(ast.get());
        ret_type->tag=KOOPA_RTT_ARRAY;
        //// ret_type->data.array
        //len
        ret_type->data.array.len=array->len;
        ret_type->data.array.base = type_buildon(array->base);
        break;
    }case AstKind::PointerType:{
        PointerType * ptr = dynamic_cast<PointerType *>(ast.get());
        ret_type->tag=KOOPA_RTT_POINTER;
        ret_type->data.pointer.base = type_buildon(ptr->base);
        break;
    }case AstKind::FunType:{
        FunType * fun = dynamic_cast<FunType *>(ast.get());
        ret_type->tag=KOOPA_RTT_FUNCTION;
        //// ret_type->data.function
        // params (koopa_raw_slice_t)
        int length=fun->params.size();
        if(length>0){
            ret_type->data.function.params.len = length;
            ret_type->data.function.params.kind = KOOPA_RSIK_TYPE;
            ret_type->data.function.params.buffer = new const void *[length];
            for (size_t i =0; i<length ; i++){
                ret_type->data.function.params.buffer[i] = static_cast<const void*>(
                    type_buildon(fun->params[i]));
            }
        }else {
            ret_type->data.function.params.kind=KOOPA_RSIK_TYPE;
            ret_type->data.function.params.len = 0;
            ret_type->data.function.params.buffer=nullptr;
        }
        // ret   yes or void
        if(fun->ret == nullptr){
            //函数返回值应该为空
            koopa_raw_type_kind_t *funret=new koopa_raw_type_kind_t;
            funret->tag = KOOPA_RTT_UNIT;   //data无意义
            ret_type->data.function.ret = funret;
        }else ret_type->data.function.ret = type_buildon(fun->ret);
        break;
    } 
    default:{
        std::cout<<"parse_type ast kind error!"<<std::endl;
        break;
    }}
    return ret_type;
}
 
//对parser_value返回的ast进行buildon
koopa_raw_value_data_t * AstBuildHelper::value_buildon(std::unique_ptr<AstBase> & ast , koopa_raw_value_data_t * inst){
    koopa_raw_value_data_t* retvalue =new koopa_raw_value_data_t;
    retvalue->used_by.len=0;
    retvalue->used_by.kind = KOOPA_RSIK_VALUE;
    retvalue->used_by.buffer=nullptr;
    switch(ast->kind){
    case AstKind::SymbolRef:{ //SymbolRef ast的buildon不需要了
        SymbolRef * symtemp = dynamic_cast<SymbolRef *>(ast.get());

        if(symtable[cur_func].find(symtemp->symbol)!=symtable[cur_func].end()){
            //添加该指令对symtemp->symbol的used_by
            symtable[cur_func][symtemp->symbol]->used_by.push_back(inst);
            return symtable[cur_func][symtemp->symbol]->sym_addr;//在局部表
        }else{
            //添加该指令对symtemp->symbol的used_by
            symtable["Global"][symtemp->symbol]->used_by.push_back(inst);
            //补充函数使用的全局变量
            functable[cur_func]->use_global.insert(symtable["Global"][symtemp->symbol].get());
            return symtable["Global"][symtemp->symbol]->sym_addr;    
        } 
        break;
    }case AstKind::IntVal:{ //IntVal ast的buildon不需要了
        IntVal * inttemp = dynamic_cast<IntVal *>(ast.get());
        
        ////****ty
        koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
        type->tag=KOOPA_RTT_INT32;
        retvalue->ty=type;
        ////****name
        retvalue->name=nullptr;
        ////****kind
        retvalue->kind.tag=KOOPA_RVT_INTEGER;
        retvalue->kind.data.integer.value = inttemp->value;
        return retvalue;    
        break;
    }case AstKind::FloatVal:{
        FloatVal * floattemp = dynamic_cast<FloatVal *>(ast.get());
    
        ////****ty
        koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
        type->tag=KOOPA_RTT_FLOAT32;
        retvalue->ty=type;
        ////****name
        retvalue->name=nullptr;
        ////****kind
        retvalue->kind.tag=KOOPA_RVT_FLOATNUM;
        retvalue->kind.data.floatnum.value = floattemp->value;
        return retvalue;  
        break;
    }case AstKind::UndefVal:{
        return nullptr; break;
    }default:{
        std::cout<<"parser_value ast kind error"<<std::endl;
        return nullptr;break;
    }}
}

//对parser_init返回的ast进行buildon
koopa_raw_value_data_t * AstBuildHelper::init_buildon(std::unique_ptr<AstBase> & ast , koopa_raw_value_data_t * inst){
    koopa_raw_value_data_t* retvalue =new koopa_raw_value_data_t;
    retvalue->used_by.len=0;
    retvalue->used_by.kind = KOOPA_RSIK_VALUE;
    retvalue->used_by.buffer=nullptr;
    switch(ast->kind){
    //根据ast的类型来返回value，有IntVal、FloatVal、UndefVal、ZeroInit、Aggregate几种
    case AstKind::SymbolRef:{
        SymbolRef * symtemp = dynamic_cast<SymbolRef *>(ast.get());
        if(symtable[cur_func].find(symtemp->symbol)!=symtable[cur_func].end()){
            //添加该指令对symtemp->symbol的used_by
            symtable[cur_func][symtemp->symbol]->used_by.push_back(inst);
            return symtable[cur_func][symtemp->symbol]->sym_addr;//在局部表
        }else{
            //添加该指令对symtemp->symbol的used_by
            symtable["Global"][symtemp->symbol]->used_by.push_back(inst);
            //补充函数使用的全局变量
            functable[cur_func]->use_global.insert(symtable["Global"][symtemp->symbol].get());
            return symtable["Global"][symtemp->symbol]->sym_addr;
        } 
        break;
    }
    case AstKind::IntVal:{ //IntVal ast的buildon不需要了
        IntVal * inttemp = dynamic_cast<IntVal *>(ast.get());
        
        ////****ty   name   used_by
        koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
        type->tag=KOOPA_RTT_INT32;
        retvalue->ty =type;
        retvalue->name=nullptr;
        ////****kind
        retvalue->kind.tag=KOOPA_RVT_INTEGER;
        retvalue->kind.data.integer.value = inttemp->value;
        return retvalue;    
        break;
    }case AstKind::FloatVal:{
        FloatVal * floattemp = dynamic_cast<FloatVal *>(ast.get());
        
        ////****ty   name   used_by
        koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
        type->tag=KOOPA_RTT_FLOAT32;
        retvalue->ty =type;
        retvalue->name=nullptr;

        ////****kind
        retvalue->kind.tag=KOOPA_RVT_FLOATNUM;
        retvalue->kind.data.floatnum.value = floattemp->value;
        return retvalue;  
        break;
    }case AstKind::UndefVal:{
        ////****ty   name   used_by
        koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
        type->tag=KOOPA_RTT_UNIT;
        retvalue->ty =type;
        retvalue->name=nullptr;
        ////****kind
        retvalue->kind.tag=KOOPA_RVT_UNDEF;
        return retvalue;  
        break;
    }case AstKind::ZeroInit:{       
    
        ////****ty   name   used_by
        koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
        type->tag=KOOPA_RTT_INT32;
        retvalue->ty =type;
        retvalue->name=nullptr;

        ////****kind
        retvalue->kind.tag=KOOPA_RVT_ZERO_INIT;
        //kind.data.zero是不存在的，zeroinit类型只需要tag
        return retvalue;  break;
    }case AstKind::Aggregate:{
        Aggregate * aggtemp = dynamic_cast<Aggregate *>(ast.get());
        koopa_raw_value_data_t* aggvalue =new koopa_raw_value_data_t;
        ////****ty   name   used_by
        aggvalue->ty =aggtemp->build_on_type();
        aggvalue->name=nullptr;
        //used_by
        aggvalue->used_by.len=0;
        aggvalue->used_by.kind = KOOPA_RSIK_VALUE;
        aggvalue->used_by.buffer=nullptr;
        ////****kind
        aggvalue->kind.tag=KOOPA_RVT_AGGREGATE;
        // kind.data.aggregate   (koopa_raw_aggregate_t)
        int length=aggtemp->elems.size();
        aggvalue->kind.data.aggregate.elems.len=length;
        aggvalue->kind.data.aggregate.elems.kind=KOOPA_RSIK_VALUE;
        //给buffer赋值  koopa_raw_value_data_t* -> void *
        aggvalue->kind.data.aggregate.elems.buffer=new const void * [length];

        for (size_t i = 0; i < length; ++i){
            aggvalue->kind.data.aggregate.elems.buffer[i]=static_cast<const void*>(
                AstBuildHelper::init_buildon(aggtemp->elems[i]  , aggvalue ) );
        }
        return aggvalue;  
        break;
    }default:{
        std::cout<<"parser_value ast kind error"<<std::endl;
        return nullptr;break;
    }}

}

int up_cal_initbuild( const koopa_raw_value_data_t * init){ 
    int cal_f= -1;
    switch(init->kind.tag){
    case KOOPA_RVT_ALLOC  : /// SymRef
    case KOOPA_RVT_GLOBAL_ALLOC:
        return find_sym( std::string(init->name))->cal_f;
        break;
    case  KOOPA_RVT_INTEGER : ///Intval
    case  KOOPA_RVT_FLOATNUM :  ///floatval
    case  KOOPA_RVT_ZERO_INIT :  /// zeroinit
        return 1;break;
    case KOOPA_RVT_UNDEF : /// undef
        return 0 ; break;
    case KOOPA_RVT_AGGREGATE :  /// aggregate
        return 2; break;
    }
    return 0;
}


koopa_raw_value_data_t * AstBuildHelper::params_buildon(std::tuple<std::string,
                        std::unique_ptr<AstBase> >  & params ,int kind , int order ){
    koopa_raw_value_data_t * value = new koopa_raw_value_data_t ;

    //变量名
    std::string  pname = std::get<0>(params);

    value->ty  = AstBuildHelper:: type_buildon(std::get<1>(params));
    value->name = strdup(pname.c_str());
    // used_by 后续再说
    value->used_by.len=0;
    value->used_by.kind = KOOPA_RSIK_VALUE;
    value->used_by.buffer=nullptr;
    //kind
    if(kind==0){
        value->kind.tag= KOOPA_RVT_FUNC_ARG_REF;   //函数参数
        value->kind.data.func_arg_ref.index=order;
    } 
    else if(kind==1) value->kind.tag = KOOPA_RVT_BLOCK_ARG_REF; //基本块参数

    return value;
}





//需要一个数据结构，存储某个的函数的`bbname`和`bb地址koopa_raw_basic_block_t`的<映射>关系
//进入Block的buildon，此时并不知道当前函数是哪个，因此需要一个变量cur_func指示当前build的函数

//还需要维护一个符号表，分函数存储，记录name和其value地址

//*********************************************************************************************************************
// *****************************function, block , global_symbol****************************************************************

/// Function definition.
FunDef::FunDef(AstKind kind,      const std::string& n, 
        std::vector<std::tuple<std::string,std::unique_ptr<AstBase>> >&& p, 
        std::unique_ptr<AstBase>&& r,      std::vector<std::unique_ptr<AstBase>>&& bb):
    AstBase(kind), name(n), params(std::move(p)), ret(std::move(r)),bbs(std::move(bb)) {}
koopa_raw_function_data_t * FunDef::build_on(){
    //需要记录的是每个bbname和koopa_raw_basic_block_t<映射>关系，这个映射在Block的buildon也会需要
    //因此该映射应该为全局变量
    cur_func=this->name;


    ////****生成 koopa_raw_function_t func
    koopa_raw_function_data_t * func;

    if( functable.find(this->name)!= functable.end() ){
        //在函数表中找到了该函数，说明已经声明过了，只需要给出params和bbs的定义即可
        func=functable[this->name]->func_addr;
    }else{//未声明过，需要新建变量，并对name ty字段赋值
        func=new koopa_raw_function_data_t;
        std::unique_ptr<FuncInfo> funcinfo = std::make_unique<FuncInfo>(this->name , func); 
        program->functable.push_back( funcinfo.get() ); //插入program的函数表
        functable[this->name]=std::move(funcinfo) ;  //函数data的地址插入函数表

        ////****name (char *)  name已经声明的时候定义了
        func->name= strdup((this->name).c_str());

        ////****ty (koopa_raw_type_t--function)
        koopa_raw_type_kind_t* funty = new koopa_raw_type_kind_t;
        funty->tag=KOOPA_RTT_FUNCTION;
        int length=this->params.size();
        // params (koopa_raw_slice_t)
        if(length>0){
            funty->data.function.params.len = length;
            funty->data.function.params.kind = KOOPA_RSIK_TYPE;
            funty->data.function.params.buffer = new const void *[length];
            for (size_t i =0; i<length ; i++){
                funty->data.function.params.buffer[i] = static_cast<const void*>(
                    AstBuildHelper::  type_buildon(   std::get<1>(this->params[i])  ));
            }
        }else {
            funty->data.function.params.kind=KOOPA_RSIK_TYPE;
            funty->data.function.params.len = 0;
            funty->data.function.params.buffer=nullptr;
        }
        // ret   yes or void
        if(this->ret == nullptr){
            //函数返回值应该为空
            koopa_raw_type_kind_t *funret=new koopa_raw_type_kind_t;
            funret->tag = KOOPA_RTT_UNIT;   //data无意义
            funty->data.function.ret = funret;
        }else funty->data.function.ret = AstBuildHelper::  type_buildon(this->ret);
        func->ty = funty;
    } 
     

    ////****params (koopa_raw_slice_t--value )
    //每个params生成一个koopa_raw_value_t，ty是其对应ty，name是元组第一个，kind的tag为FUNC_ARG_REF，对应data的index是第几个函数参数
    func->params.len=this->params.size();
    func->params.kind = KOOPA_RSIK_VALUE;
    func->params.buffer=new const void *[this->params.size()];
    for(size_t i =0;i<func->params.len; ++i){ //函数参数
        koopa_raw_value_data_t * param_value = AstBuildHelper:: params_buildon((this->params)[i],0 ,i);

        std::unique_ptr<SymInfo> syminfo = std::make_unique<SymInfo>( std::string(param_value->name) , param_value , VarStatus::FUNC_ARG) ;
        functable[cur_func]->params.push_back( syminfo.get() );
        symtable[cur_func][param_value->name]=std::move(syminfo) ;

        func->params.buffer[i]= static_cast<const void*>( param_value);
    }


    ////****bbs (koopa_raw_slice_t--block)
    std::vector<koopa_raw_basic_block_data_t *> bbs_buf;
    
    //迭代bb（vector），将每个bbname和koopa_raw_basic_block_t<映射>关系弄好
    std::unordered_map<std::string, std::unique_ptr<BBInfo>  > bb_addr_map;
    for(auto &bb:this->bbs){
        //bb 为 Block:AstBase类型
        std::string bbname = dynamic_cast< Block *>(bb.get())->name;
        //新建一个koopa_raw_basic_block_data_t
        koopa_raw_basic_block_data_t * bbaddr=new koopa_raw_basic_block_data_t;
        bbs_buf.push_back(bbaddr);
        bbaddr->name=strdup(bbname.c_str()); 
        //新建一个BBInfo
        std::unique_ptr<BBInfo> bbinfo =std::make_unique<BBInfo>(bbname,bbaddr);
        functable[cur_func]->bb_list.push_back(bbinfo.get()  );
        bb_addr_map.insert({bbname,  std::move(bbinfo) });
    }
    bbtable.insert( {cur_func, std::move(bb_addr_map)  } );
    // std::cout<<bbtable<<std::endl;
    //迭代bb，对每个bb应用Block的buildon，根据bbname直接build地址为koopa_raw_basic_block_t的即可
    for(auto &bb:this->bbs){
        //bb 为 Block:AstBase类型
        Block * bbptr = dynamic_cast< Block *>(bb.get());
        bbptr->build_on(bbtable[cur_func][bbptr->name]->bb_addr);
    }
    //将弄好的bb list生成buffer，对应赋值func的bbs(koopa_raw_slice_t)
    func->bbs.buffer=new const void*[bbs_buf.size()];
    func->bbs.len   =bbs_buf.size();
    func->bbs.kind = KOOPA_RSIK_BASIC_BLOCK;
    for (size_t i = 0; i < bbs_buf.size(); ++i) 
        func->bbs.buffer[i] = static_cast<const void*>(bbs_buf[i]);
    return func;
}

/// Basic block.
Block::Block(AstKind kind,const std::string& n, 
        std::vector<std::tuple<std::string,std::unique_ptr<AstBase>> >&& p, 
        std::vector<std::unique_ptr<AstBase>>&& s   ):
    AstBase(kind), name(n),params(std::move(p)), stmts(std::move(s)) {}
void Block::build_on(koopa_raw_basic_block_data_t *bb_data){
    ////****传入koopa_raw_basic_block_t  block
    cur_bb=this->name;
    // std::cout<<(void*)bb_data<<bb_data->name<<std::endl;

    ////****params (koopa_raw_slice_t--value )当前为空，非ssa形式 
    //根据vector params生成参数列表,先生成一个koopa_raw_slice_t
    //迭代vector params给buffer赋值
    //每个params生成一个koopa_raw_value_t，ty是其对应ty，name是元组第一个，kind的tag为FUNC_ARG_REF，对应data的index是第几个函数参数
    bb_data->params.len=this->params.size();
    bb_data->params.kind = KOOPA_RSIK_VALUE;
    bb_data->params.buffer=nullptr;   

    ////****ty (koopa_raw_type_t--function)
    //--生成一个tag

    ////****used_by (koopa_raw_slice_t--value)
    //--生成一个tag为function的koopa_raw_type_t

    //--对应的data的ret为返回值的koopa_raw_type_t（根据ret获得），params为参数type列表koopa_raw_slice_t（其kind为KOOPA_RSIK_TYPE)

    ////****insts (koopa_raw_slice_t--value)
    bb_data->insts.kind =KOOPA_RSIK_VALUE;
    bb_data->insts.buffer=new const void*[this->stmts.size()];
    bb_data->insts.len   =this->stmts.size();
    
    koopa_raw_value_data_t * inst=nullptr;

    //迭代 stmts ,对每个 inst 应用 ast对应的buildon
    // std::cout<<stmts.size()<<std::endl;
    for (size_t i = 0; i < this->stmts.size(); ++i){
        auto ptr=std::move(this->stmts[i]);
        // std::cout<<ptr->kind<<std::endl;
        switch(ptr->kind){
        //某条指令顶层ast的kind只要SymbolDef、FunCall、Store、Branch、Jump、Return
        case AstKind::SymbolDef: {
//要记录SymbolDef定义这个value和变量名name的映射，其他变量也是，要记录name和其对应的value的映射
            inst= dynamic_cast<SymbolDef*>(ptr.get())->build_on() ;
            break;
        }case AstKind::FunCall: {
            inst = new koopa_raw_value_data_t;
            ////****name (char * ) 单纯的函数调用，没有变量定义赋值，name为空
            inst->name=nullptr;
            ////****usd_by 单纯的函数调用指令应该没被use
            inst->used_by.len=0;
            inst->used_by.kind = KOOPA_RSIK_VALUE;
            inst->used_by.buffer=nullptr ;
            // 先不做
            ////****kind
            dynamic_cast<FunCall*>(ptr.get())->build_on(inst->kind ,inst);
            //在函数表中记录 caLL_fun_name 函数被该 inst 指令调用
            std::string call_fun_name=inst->kind.data.call.callee->name;
            functable[call_fun_name]->used_by.push_back(inst);
            ////****ty 是函数返回值的类型
            inst->ty=inst->kind.data.call.callee->ty->data.function.ret;           
            break;
        }case AstKind::Store: {
            inst =  dynamic_cast<Store*>(ptr.get())->build_on();
            // SymInfo * syminfo = find_sym(std::string(inst->kind.data.store.dest->name));
            // bbtable[cur_func][cur_bb]->sym_list.push_back( syminfo ) ; //插入bb的sym_list中
            break;
        }case AstKind::Branch: {
            inst =  dynamic_cast<Branch*>(ptr.get())->build_on() ;
            break;
        }case AstKind::Jump: {
            inst = dynamic_cast<Jump*>(ptr.get())->build_on();
            break;
        }case AstKind::Return: {
            inst = dynamic_cast<Return*>(ptr.get())->build_on();
            break;
        }default:{
            std::cerr<<"unknown stmt ast kind: "<<ptr->kind<<"\n";
            exit(1);
        }}
        //将inst添加进BBinfo
        bbtable[cur_func][cur_bb]->insts.push_back(inst);
        //将该指令 inst 写入bb的inst_buf
        bb_data->insts.buffer[i]=static_cast<const void*>( inst );
    }
}

/// Function declaration.
FunDecl::FunDecl(AstKind kind,const std::string& n, std::vector<std::unique_ptr<AstBase>>&& p, 
        std::unique_ptr<AstBase>&& r):
    AstBase(kind), name(n), params(std::move(p)), ret(std::move(r)) {}
koopa_raw_function_data_t* FunDecl:: build_on(){
    koopa_raw_function_data_t * func=new koopa_raw_function_data_t;
    std::unique_ptr<FuncInfo> funcinfo=std::make_unique<FuncInfo>(this->name , func); 
    //插入program
    
    program->functable.push_back(funcinfo.get()); //插入program的函数表
    functable[this->name]=std::move(funcinfo) ; //插入函数表

    ////****ty (koopa_raw_type_t)
    koopa_raw_type_kind_t* funty = new koopa_raw_type_kind_t;
    funty->tag=KOOPA_RTT_FUNCTION;
    int length=this->params.size();
    // params (koopa_raw_slice_t)
    if(length>0){
        funty->data.function.params.len = length;
        funty->data.function.params.kind = KOOPA_RSIK_TYPE;
        funty->data.function.params.buffer = new const void *[length];
        for (size_t i =0; i<length ; i++){
            funty->data.function.params.buffer[i] = static_cast<const void*>(
                AstBuildHelper::type_buildon(this->params[i]));
        }
    }else {
        funty->data.function.params.kind=KOOPA_RSIK_TYPE;
        funty->data.function.params.len = 0;
        funty->data.function.params.buffer=nullptr;
    }
    // ret   yes or void
    if(this->ret == nullptr){
        //函数返回值应该为空
        koopa_raw_type_kind_t *funret=new koopa_raw_type_kind_t;
        funret->tag = KOOPA_RTT_UNIT;   //data无意义
        funty->data.function.ret = funret;
    }else funty->data.function.ret =AstBuildHelper:: type_buildon(this->ret);
    func->ty = funty;

    ////****name (const char *)
    func->name= strdup((this->name).c_str());

    ////****params (koopa_raw_slice_t--value )
    // Decl 的params列表为空
    func->params.kind = KOOPA_RSIK_VALUE;
    func->params.len =0;
    func->params.buffer=nullptr;

    ////****bbs (koopa_raw_slice_t)
    func->bbs.kind = KOOPA_RSIK_BASIC_BLOCK;
    func->bbs.len =0;
    func->bbs.buffer=nullptr;

    return func;
}


/// Global symbol definition. 全局变量
GlobalDef::GlobalDef(AstKind kind, const std::string& n, std::unique_ptr<AstBase>&& v) :
    AstBase(kind), name(n), value(std::move(v))   {}
koopa_raw_value_data_t * GlobalDef:: build_on(){
    GlobalDecl * decl = dynamic_cast<GlobalDecl *>(this->value.get());

    koopa_raw_value_data_t * globaldef = new koopa_raw_value_data_t;

    ////****name (const char *)
    globaldef->name= strdup((this->name).c_str());
    std::unique_ptr<SymInfo> syminfo=std::make_unique<SymInfo>(this->name , globaldef, VarStatus::GlOBAL);
    program->global_syms.push_back(  syminfo.get() ); //插入program的全局变量表
    symtable["Global"][this->name]= std::move(syminfo)  ;   ///插入符号表, 全局变量 
    
    ////****ty
    koopa_raw_type_kind_t* memty = new koopa_raw_type_kind_t;
    memty->tag = KOOPA_RTT_POINTER;
    memty->data.pointer.base= AstBuildHelper::type_buildon(decl->ty);
    globaldef->ty = memty;
    ////****used_by 先不做
    globaldef->used_by.len=0;
    globaldef->used_by.kind = KOOPA_RSIK_VALUE;
    globaldef->used_by.buffer=nullptr;
    ////****kind
    globaldef->kind.tag=KOOPA_RVT_GLOBAL_ALLOC;
    globaldef->kind.data.global_alloc.init = AstBuildHelper::init_buildon(decl->init , globaldef);

    ///更新是否可计算
    symtable["Global"][this->name]->cal_f = up_cal_initbuild( globaldef->kind.data.global_alloc.init );

    ////**** used_by 先不做

    return globaldef;

}


//*********************************************************************************************************************
// *****************************type************************************************************************************
IntType::IntType(AstKind kind):AstBase(kind){}
FloatType::FloatType(AstKind kind): AstBase(kind){}

/// Array type.
ArrayType::ArrayType(AstKind kind, std::unique_ptr<AstBase>&&b , int l):
    AstBase(kind), base(std::move(b)), len(l) {}

/// Pointer type.
PointerType::PointerType(AstKind kind, std::unique_ptr<AstBase>&& b):
    AstBase(kind), base(std::move(b)) {}

/// Function type.
FunType::FunType(AstKind kind, std::vector<std::unique_ptr<AstBase>>&& params,
                          std::unique_ptr<AstBase>&& ret):
    AstBase(kind), params(std::move(params)), ret(std::move(ret)) {}




//*********************************************************************************************************************
//****************************init or value************************************************************************************

// Symbol reference.
SymbolRef::SymbolRef(AstKind kind, const std::string& sym): 
    AstBase(kind), symbol(sym) {}

/// Integer literal.
IntVal::IntVal(AstKind kind, int v): AstBase(kind), value(v) {}

/// float literal.
FloatVal::FloatVal(AstKind kind, float v): AstBase(kind), value(v) {}

/// Undefined value.
UndefVal::UndefVal(AstKind kind): AstBase(kind) {}

/// Aggregate value.
Aggregate::Aggregate(AstKind kind, std::vector<std::unique_ptr<AstBase>>&& elems) :
    AstBase(kind), elems(std::move(elems)) {}
koopa_raw_type_kind * Aggregate::build_on_type(){
    koopa_raw_type_kind * type= new koopa_raw_type_kind ;
    type->tag = KOOPA_RTT_ARRAY;
    type->data.array.len = this->elems.size();
    if(this->elems[0]->kind==AstKind::Aggregate){
        type->data.array.base = dynamic_cast<Aggregate*>(this->elems[0].get())->build_on_type();
    }else{
        koopa_raw_type_kind * subtype= new koopa_raw_type_kind ;
        switch(this->elems[0]->kind){
        case AstKind::SymbolRef:
            subtype->tag = KOOPA_RTT_INT32; //注：这里有问题的，如果后端不用就不用管了，如果要使用，后续要修改
            break;
        case AstKind::FloatVal:
            subtype->tag = KOOPA_RTT_FLOAT32;
            break;
        case AstKind::IntVal:
        case AstKind::ZeroInit:
            subtype->tag = KOOPA_RTT_INT32;
            break;
        case AstKind::UndefVal:
            subtype->tag = KOOPA_RTT_UNIT;
        }
        type->data.array.base = subtype;
    }
    return type;
}



/// Zero initializer.
ZeroInit::ZeroInit(AstKind kind): AstBase(kind) {}




//*********************************************************************************************************************
//****************************instructions************************************************************************************

/// Symbol definition.
SymbolDef::SymbolDef(AstKind kind, const std::string& n, std::unique_ptr<AstBase>&& v) :
    AstBase(kind), name(n), value(std::move(v))   {}
koopa_raw_value_data_t * SymbolDef:: build_on(){
    koopa_raw_value_data_t * inst=new koopa_raw_value_data_t;

  
    
    ////****name (char *)
    inst->name=strdup((this->name).c_str());
    // 插入符号表  def都是第一次，无需检测原来有没有值
    std::unique_ptr<SymInfo> syminfo=std::make_unique<SymInfo>(this->name, inst) ;
    
    ////****used_by (koopa_raw_slice_t)
    // 先不做
    inst->used_by.len=0;
    inst->used_by.kind = KOOPA_RSIK_VALUE;
    inst->used_by.buffer=nullptr;
    ////****kind (koopa_raw_value_kind_t)
    switch (this->value->kind){
    case AstKind::MemDecl:{   //局部变量定义   kind.data无意义
        ////****ty
        koopa_raw_type_kind_t* memty = new koopa_raw_type_kind_t;
        memty->tag = KOOPA_RTT_POINTER;
        memty->data.pointer.base= AstBuildHelper::type_buildon(dynamic_cast<MemDecl*>(
                                                    this->value.get())->ty);
        inst->ty = memty;
        inst->kind.tag=KOOPA_RVT_ALLOC;
        break;
    }case AstKind::Load:{ //Load:AstBase 无需再去实现buildon方法了
        inst->kind.tag=KOOPA_RVT_LOAD;
        //将this->value强制转化为load*
        std::string  SymRef=dynamic_cast<Load*>(this->value.get())->symbol;
        if(symtable[cur_func].find(SymRef)!=symtable[cur_func].end()){
            //在局部表
            inst->kind.data.load.src=symtable[cur_func][SymRef]->sym_addr;
            //将该指令使用SymRef添加到符号表used_by中
            symtable[cur_func][SymRef]->used_by.push_back(inst);
            syminfo->cal_f = symtable[cur_func][SymRef]->cal_f;

        }else{
            inst->kind.data.load.src=symtable["Global"][SymRef]->sym_addr;
            //补充函数使用的全局变量
            functable[cur_func]->use_global.insert(symtable["Global"][SymRef].get());
            //将该指令使用SymRef添加到符号表used_by中
            symtable["Global"][SymRef]->used_by.push_back(inst);
            syminfo->cal_f = symtable["Global"][SymRef]->cal_f;

        } 
        ////****ty
        inst->ty = inst->kind.data.load.src->ty->data.array.base;
        break;
    }case AstKind::GetPointer:{
        ////****ty   pointer
        koopa_raw_type_kind_t* getptr = new koopa_raw_type_kind_t;
        getptr->tag = KOOPA_RTT_POINTER;

        inst->kind.tag=KOOPA_RVT_GET_PTR;
        dynamic_cast<GetPointer*>(this->value.get())->build_on(inst->kind , inst);
     
        getptr->data.pointer.base  = inst->kind.data.get_ptr.src->ty->data.array.base;
        // getptr->data.pointer.base = inst->kind.data.get_ptr.src->ty;
        inst->ty = getptr;

        syminfo->cal_f =2;
        break;
    }case AstKind::GetElementPointer:{
        ////****ty   pointer
        koopa_raw_type_kind_t* eleptr = new koopa_raw_type_kind_t;
        eleptr->tag = KOOPA_RTT_POINTER;

        inst->kind.tag=KOOPA_RVT_GET_ELEM_PTR;
        dynamic_cast<GetElementPointer*>(this->value.get())->build_on(inst->kind , inst);
        
        eleptr->data.pointer.base  = inst->kind.data.get_elem_ptr.src->ty->data.array.base->data.array.base;
        // eleptr->data.pointer.base = inst->kind.data.get_elem_ptr.src->ty;
        inst->ty = eleptr;
        syminfo->cal_f =2;
        break;
    }case AstKind::FunCall:{
 
        inst->kind.tag=KOOPA_RVT_CALL;;
        dynamic_cast<FunCall*>(this->value.get())->build_on(inst->kind , inst);
        //在函数表中记录call_fun_name函数被该inst指令调用
        std::string call_fun_name=inst->kind.data.call.callee->name;
        functable[call_fun_name]->used_by.push_back(inst) ;
        ////****ty   函数返回值
        inst->ty = inst->kind.data.call.callee->ty->data.function.ret;
        syminfo->cal_f =3;
        break;
    }case AstKind::BinaryExpr:{
        ////**** ty有待考虑,涉及隐式类型转换


        inst->kind.tag=KOOPA_RVT_BINARY;
        dynamic_cast<BinaryExpr*>(this->value.get())->build_on(inst->kind , inst);

        ///****隐式类型转换
        switch(inst->kind.data.binary.op){
            case KOOPA_RBO_NOT_EQ:
            case KOOPA_RBO_EQ:
            case KOOPA_RBO_GT:
            case KOOPA_RBO_LT:
            case KOOPA_RBO_GE:
            case KOOPA_RBO_LE:
            case KOOPA_RBO_MOD:
            case KOOPA_RBO_SHL:
            case KOOPA_RBO_SHR:
            case KOOPA_RBO_SAR:{
                // 都是int  
                koopa_raw_type_kind_t* resty = new koopa_raw_type_kind_t;
                resty->tag = KOOPA_RTT_INT32;
                inst->ty = resty;
                break;
            }
            case KOOPA_RBO_ADD:
            case KOOPA_RBO_SUB:
            case KOOPA_RBO_MUL:
            case KOOPA_RBO_DIV:
            case KOOPA_RBO_AND:
            case KOOPA_RBO_XOR:
            case KOOPA_RBO_OR:
                if(inst->kind.data.binary.lhs->ty->tag==KOOPA_RTT_FLOAT32 || inst->kind.data.binary.rhs->ty->tag==KOOPA_RTT_FLOAT32  ){
                    if(inst->kind.data.binary.lhs->ty->tag==KOOPA_RTT_FLOAT32 )inst->ty=inst->kind.data.binary.lhs->ty;
                    else inst->ty=inst->kind.data.binary.rhs->ty;
                }else inst->ty =  inst->kind.data.binary.lhs->ty;
                break;
            default:
                inst->ty = inst->kind.data.binary.lhs->ty;
        }
        if(inst->kind.data.binary.lhs->name!=nullptr && inst->kind.data.binary.rhs->name!=nullptr){
            syminfo->cal_f=((find_sym(std::string(inst->kind.data.binary.lhs->name))->cal_f ==1 )&&
                (find_sym(std::string(inst->kind.data.binary.rhs->name))->cal_f ==1 ))? 1 : 0 ; 
        }else syminfo->cal_f=0;
        break;
    }
    default:{
        std::cerr<<"unknown value ast kind: "<<this->value->kind<<std::endl;
        break;
    }}
    if(inst->kind.tag == KOOPA_RVT_ALLOC) syminfo->v_status = VarStatus::LOCAL;
    else syminfo->v_status = VarStatus::TEMP;

    symtable[cur_func][this->name]= std::move(syminfo)  ; 
    if(inst->kind.tag == KOOPA_RVT_ALLOC) 
        bbtable[cur_func][cur_bb]->sym_list.push_back(symtable[cur_func][this->name].get());  //在bbtable

    return inst;
}



/// Global memory declaration.
GlobalDecl::GlobalDecl(AstKind kind, std::unique_ptr<AstBase>&& t,std::unique_ptr<AstBase>&& init):
    AstBase(kind), ty(std::move(t)) , init(std::move(init)) {}


/// Memory declaration.
MemDecl::MemDecl(AstKind kind, std::unique_ptr<AstBase>&& t) :
    AstBase(kind), ty(std::move(t)) {}


/// Load.
Load::Load(AstKind kind,  const std::string& n):
    AstBase(kind), symbol(n) {}


/// Pointer calculation.
GetPointer::GetPointer(AstKind kind,const std::string& s, std::unique_ptr<AstBase>&& v):
    AstBase(kind), symbol(s) , value(std::move(v))  {}
void GetPointer:: build_on(koopa_raw_value_kind_t & kind , koopa_raw_value_data_t * inst){
    if(symtable[cur_func].find(this->symbol)!=symtable[cur_func].end()){
        kind.data.get_ptr.src=symtable[cur_func][this->symbol]->sym_addr;//在局部表
        //将该指令使用 this->symbol 添加到符号表used_by中
        symtable[cur_func][this->symbol]->used_by.push_back(inst);
    }else{
        kind.data.get_ptr.src= symtable["Global"][this->symbol]->sym_addr;
        //将该指令使用 this->symbol 添加到符号表used_by中
        symtable["Global"][this->symbol]->used_by.push_back(inst);
    } 
    kind.data.get_ptr.index=AstBuildHelper::value_buildon(this->value , inst);
}


/// Element pointer calculation.
GetElementPointer::GetElementPointer(AstKind kind,const std::string& s, std::unique_ptr<AstBase>&& v):
    AstBase(kind),symbol(s) , value(std::move(v))   {}
void GetElementPointer:: build_on(koopa_raw_value_kind_t & kind , koopa_raw_value_data_t * inst){
    if(symtable[cur_func].find(this->symbol)!=symtable[cur_func].end()){
        kind.data.get_elem_ptr.src=symtable[cur_func][this->symbol]->sym_addr;//在局部表
        //将该指令使用 this->symbol 添加到符号表used_by中
        symtable[cur_func][this->symbol]->used_by.push_back(inst);
    }else{
        kind.data.get_elem_ptr.src= symtable["Global"][this->symbol]->sym_addr;
        //将该指令使用 this->symbol 添加到符号表used_by中
        symtable["Global"][this->symbol]->used_by.push_back(inst);
    } 
    kind.data.get_elem_ptr.index=AstBuildHelper::value_buildon(this->value , inst);
}


/// Binary expression.
BinaryExpr::BinaryExpr(AstKind kind,BinaryOp op, std::unique_ptr<AstBase>&& l, std::unique_ptr<AstBase>&& r):
    AstBase(kind), op(op), lhs(std::move(l)), rhs(std::move(r)) {}
void BinaryExpr:: build_on(koopa_raw_value_kind_t & kind , koopa_raw_value_data_t * inst){
    kind.data.binary.op= OPTORAWOP.find(this->op)->second;
    kind.data.binary.lhs=AstBuildHelper::value_buildon(this->lhs , inst);
    kind.data.binary.rhs=AstBuildHelper::value_buildon(this->rhs , inst);
}


/// Store.
Store::Store(AstKind kind,  std::unique_ptr<AstBase>&& v,  const std::string& s):
    AstBase(kind), value(std::move(v)), symbol(s) {}
koopa_raw_value_data_t * Store:: build_on(){
    koopa_raw_value_data_t * inst=new koopa_raw_value_data_t;

    ////****ty (koopa_raw_type_t)  store指令的 ty是unit  即void
    koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
    type->tag=KOOPA_RTT_UNIT;
    inst->ty=type;
    
    ////****name (char *)   为空，因为没有变量赋值
    inst->name=nullptr;

    ////****used_by (koopa_raw_slice_t)单纯的函数调用指令应该没被use
    inst->used_by.len=0;
    inst->used_by.kind = KOOPA_RSIK_VALUE;
    inst->used_by.buffer=nullptr;
    //先不做
    ////****kind (koopa_raw_value_kind_t)
    inst->kind.tag=KOOPA_RVT_STORE;
    //value写入目的地的值，由语法分析parse_init()或者SymbolRef的来
    if(this->value->kind==AstKind::SymbolRef){
        std::string symref=dynamic_cast<SymbolRef *>((this->value).get())->symbol;

        if(symtable[cur_func].find(symref)!=symtable[cur_func].end()){
            inst->kind.data.store.value=symtable[cur_func][symref]->sym_addr;  //在局部表
            //将该指令使用 symref 添加到符号表used_by中
            symtable[cur_func][symref]->used_by.push_back(inst);  //在局部表中记录
        }else{
            inst->kind.data.store.value= symtable["Global"][symref]->sym_addr;
            //补充函数使用的全局变量
            functable[cur_func]->use_global.insert(symtable["Global"][symref].get());
            //将该指令使用 symref 添加到符号表used_by中
            symtable["Global"][symref]->used_by.push_back(inst);  //在全局表中记录
        } 

    }else inst->kind.data.store.value=AstBuildHelper::init_buildon(this->value , inst);
    //dest 存储目的地  查符号表
    if(symtable[cur_func].find(this->symbol)!=symtable[cur_func].end()){
        inst->kind.data.store.dest=symtable[cur_func][this->symbol]->sym_addr;  //在局部表
        //将该指令使用 this->symbol 添加到符号表used_by中
        symtable[cur_func][this->symbol]->used_by.push_back(inst);  //在局部表中记录
        bbtable[cur_func][cur_bb]->sym_list.push_back(symtable[cur_func][this->symbol].get());  //在bbtable
    }else {
        inst->kind.data.store.dest= symtable["Global"][this->symbol]->sym_addr;
        //补充函数使用的全局变量
            functable[cur_func]->use_global.insert(symtable["Global"][this->symbol].get());
        ////将该指令使用 this->symbol 添加到符号表used_by中
        symtable["Global"][this->symbol]->used_by.push_back(inst);  //在全局表中记录
        bbtable[cur_func][cur_bb]->sym_list.push_back(symtable["Global"][this->symbol].get());  //在当前bb定值的变量
    } 
    return inst;
}


/// Branch.
Branch::Branch(AstKind kind,std::unique_ptr<AstBase>&& c, 
            const std::string& tb,  std::vector<std::unique_ptr<AstBase>>&& ta, 
            const std::string& fb,  std::vector<std::unique_ptr<AstBase>>&& fa):       
    AstBase(kind), cond(std::move(c)), tbb(tb), targs(std::move(ta)), fbb(fb),fargs(std::move(fa)) {}
koopa_raw_value_data_t * Branch::build_on(){
    koopa_raw_value_data_t * inst=new koopa_raw_value_data_t;

    ////****ty (koopa_raw_type_t)
    koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
    type->tag=KOOPA_RTT_UNIT;
    inst->ty=type;
    
    ////****name (char *)   为空，因为没有变量赋值
    inst->name=nullptr;

    ////****used_by (koopa_raw_slice_t)
    inst->used_by.len=0;
    inst->used_by.kind = KOOPA_RSIK_VALUE;
    inst->used_by.buffer=nullptr;
    //先不做
    ////****kind (koopa_raw_value_kind_t)
    inst->kind.tag=KOOPA_RVT_BRANCH;
    //inst->kind.data.branch    (koopa_raw_branch_t)
    //cond (koopa_raw_value_t)
    inst->kind.data.branch.cond=AstBuildHelper::value_buildon(this->cond , inst);
    

    ////true_bb (koopa_raw_basic_block_t)
    inst->kind.data.branch.true_bb=bbtable[cur_func][this->tbb]->bb_addr;
    //补充基本块被该指令inst使用
    bbtable[cur_func][this->tbb]->used_by.push_back(inst);
    //补充前驱后继关系
    bbtable[cur_func][cur_bb]->next.push_back(bbtable[cur_func][this->tbb].get());//当前基本块的true后继
    bbtable[cur_func][this->tbb]->prev.push_back(bbtable[cur_func][cur_bb].get());//true时跳转的基本块的前驱

    ////false_bb (koopa_raw_basic_block_t)
    inst->kind.data.branch.false_bb=bbtable[cur_func][this->fbb]->bb_addr;
    //补充基本块被该指令inst使用
    bbtable[cur_func][this->fbb]->used_by.push_back(inst);
    //补充前驱后继关系
    bbtable[cur_func][cur_bb]->next.push_back(bbtable[cur_func][this->fbb].get());//当前基本块的 false 后继
    bbtable[cur_func][this->fbb]->prev.push_back(bbtable[cur_func][cur_bb].get());//false 时跳转的基本块的前驱

    //true_args (koopa_raw_slice_t)
    //当前其实是没有基本块参数的，所有无需初始化
    inst->kind.data.branch.true_args.kind=KOOPA_RSIK_VALUE;
    inst->kind.data.branch.true_args.len=0;
    inst->kind.data.branch.true_args.buffer=nullptr;
    //false_args (koopa_raw_slice_t)
    //当前其实是没有基本块参数的，所有无需初始化
    inst->kind.data.branch.false_args.kind=KOOPA_RSIK_VALUE;
    inst->kind.data.branch.false_args.len=0;
    inst->kind.data.branch.false_args.buffer=nullptr;
    return inst;
}   

/// Jump.
Jump::Jump(AstKind kind,const std::string& target, std::vector<std::unique_ptr<AstBase>>&& a):
    AstBase(kind),target(target),args(std::move(a)) {}
koopa_raw_value_data_t * Jump::build_on(){
    koopa_raw_value_data_t * inst=new koopa_raw_value_data_t;

    ////****ty (koopa_raw_type_t)
    koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
    type->tag=KOOPA_RTT_UNIT;
    inst->ty=type;
    
    ////****name (char *)   为空，因为没有变量赋值
    inst->name=nullptr;

    ////****used_by (koopa_raw_slice_t)
    inst->used_by.len=0;
    inst->used_by.kind = KOOPA_RSIK_VALUE;
    inst->used_by.buffer=nullptr;
    ////****kind (koopa_raw_value_kind_t)
    inst->kind.tag=KOOPA_RVT_JUMP;
    //inst->kind.data.jump    (koopa_raw_jump_t)
    //target (koopa_raw_basic_block_t)
    inst->kind.data.jump.target=bbtable[cur_func][this->target]->bb_addr;
    //补充基本块被该指令inst使用
    bbtable[cur_func][this->target]->used_by.push_back(inst);
    //补充前驱后继关系
    bbtable[cur_func][cur_bb]->next.push_back(bbtable[cur_func][this->target].get());//当前基本块的后继
    bbtable[cur_func][this->target]->prev.push_back(bbtable[cur_func][cur_bb].get());//跳转的基本块的前驱


    //args (koopa_raw_slice_t) 一基本块参数当前还没有的，所有无需初始化
    inst->kind.data.jump.args.kind=KOOPA_RSIK_VALUE;
    inst->kind.data.jump.args.len=0;
    inst->kind.data.jump.args.buffer=nullptr;
    
    return inst;
}   

/// Function call.
FunCall::FunCall(AstKind kind,const std::string& fun,   std::vector<std::unique_ptr<AstBase>>&& a):
    AstBase(kind), fun(fun), args(std::move(a)) {}
void FunCall:: build_on(koopa_raw_value_kind_t & kind , koopa_raw_value_data_t * inst){
    kind.tag=KOOPA_RVT_CALL;
    //修改kind.data.call
    kind.data.call.callee=functable[this->fun]->func_addr;
    // 构建args   实参
    kind.data.call.args.kind=KOOPA_RSIK_VALUE;
    kind.data.call.args.len=this->args.size();
    kind.data.call.args.buffer=new const void* [this->args.size()];
    for (size_t i = 0; i < this->args.size(); ++i) {
        // kind.data.call.args.push_back(AstBuildHelper::value_buildon(arg));
        kind.data.call.args.buffer[i] =static_cast<const void*>(
            AstBuildHelper::value_buildon(this->args[i] , inst)
        );
    }
}

/// Return.
Return::Return(AstKind kind, std::unique_ptr<AstBase>&& v):
    AstBase(kind), value(std::move(v)) {}
koopa_raw_value_data_t * Return::build_on(){
    koopa_raw_value_data_t * inst=new koopa_raw_value_data_t;
    ////****ty (koopa_raw_type_t)
    koopa_raw_type_kind_t * type = new koopa_raw_type_kind_t;
    type->tag=KOOPA_RTT_UNIT;
    
    inst->ty=functable[cur_func]->func_addr->ty->data.function.ret;
    
    // inst->ty=type;
    
    ////****name (char *)   为空，因为没有变量赋值
    inst->name=nullptr;

    ////****used_by (koopa_raw_slice_t)
    inst->used_by.len=0;
    inst->used_by.kind = KOOPA_RSIK_VALUE;
    inst->used_by.buffer=nullptr;
    ////****kind (koopa_raw_value_kind_t)
    inst->kind.tag=KOOPA_RVT_RETURN;
    //inst->kind.data.ret    (koopa_raw_return_t)
    //value (koopa_raw_value_t)
    if(this->value){
        //存在返回值
        inst->kind.data.ret.value=AstBuildHelper::value_buildon(this->value , inst);
    }else  inst->kind.data.ret.value=nullptr;
    return inst;
}

/// End of file.
End::End(AstKind kind): AstBase(kind) {}

/// Error.
Error::Error(AstKind kind): AstBase(kind){}

//       ********          ***     **         *****
//       **                ** **   **         **   **
//       *******           **  **  **         **    **
//       **                **   ** **         **   ** 
//       ********          **     ***         *****

            



std::ostream& operator<<(std::ostream& os, AstKind kind) {
    switch (kind) {
        case AstKind::IntType:
            os << "IntType";
            break;
        case AstKind::FloatType:
            os << "FloatType";
            break;
        case AstKind::ArrayType:
            os << "ArrayType";
            break;
        case AstKind::PointerType:
            os << "PointerType";
            break;
        case AstKind::FunType:
            os << "FunType";
            break;
        case AstKind::SymbolRef:
            os << "SymbolRef";
            break;
        case AstKind::IntVal:
            os << "IntVal";
            break;
        case AstKind::FloatVal:
            os << "FloatVal";
            break;
        case AstKind::UndefVal:
            os << "UndefVal";
            break;
        case AstKind::Aggregate:
            os << "Aggregate";
            break;
        case AstKind::ZeroInit:
            os << "ZeroInit";
            break;
        case AstKind::SymbolDef:
            os << "SymbolDef";
            break;
        case AstKind::GlobalDef:
            os << "GlobalDef";
            break;
        case AstKind::MemDecl:
            os << "MemDecl";
            break;
        case AstKind::GlobalDecl:
            os << "GlobalDecl";
            break;
        case AstKind::Load:
            os << "Load";
            break;
        case AstKind::Store:
            os << "Store";
            break;
        case AstKind::GetPointer:
            os << "GetPointer";
            break;
        case AstKind::GetElementPointer:
            os << "GetElementPointer";
            break;
        case AstKind::BinaryExpr:
            os << "BinaryExpr";
            break;
        case AstKind::Branch:
            os << "Branch";
            break;
        case AstKind::Jump:
            os << "Jump";
            break;
        case AstKind::FunCall:
            os << "FunCall";
            break;
        case AstKind::Return:
            os << "Return";
            break;
        case AstKind::FunDef:
            os << "FunDef";
            break;
        case AstKind::Block:
            os << "Block";
            break;
        case AstKind::FunDecl:
            os << "FunDecl";
            break;
        case AstKind::End:
            os << "End";
            break;
        case AstKind::Error:
            os << "Error";
            break;
        default:
            os << "Unknown";
            break;
    }
    return os;
}




            