#include "../../include/opt/optimizer.hpp"
#include "../../include/opt/dom.hpp"

// 临时变量映射局部变量  zy_use   
std::unordered_map<SymInfo * , SymInfo * >  temp2local;  //局部变量包括 纯局部变量和基本块参数
// 局部变量映射临时变量列表  zyc_use  
std::unordered_map<SymInfo * , std::vector< std::pair< SymInfo *, BBInfo * >>  >  local2temp;  
//local2temp[局部变量名] <-> 一个vector,  vector的元素为pair :   pair<load局部变量的临时变量名  , load的基本块 >
std::unordered_map<koopa_raw_binary_op_t, std::unordered_map<koopa_raw_value_t, std::unordered_map<koopa_raw_value_t, std::unordered_map<BBInfo *, koopa_raw_value_data *>>>> binary_exp;
std::unordered_map <koopa_raw_value_t, std::unordered_map<koopa_raw_value_t , std::vector <std::pair<BBInfo *, SymInfo * > > >> ptr_exp;

extern std::string outpath;
extern int now;

// //迭代aggregate类型的value，将里面的变量添加 inst的使用
void Add_UsedBy_Aggregate( koopa_raw_value_t  value  ,koopa_raw_value_data* inst ){
    int len = value->kind.data.aggregate.elems.len;
    // koopa_raw_value_t tmp = nullptr;
    for(int i = 0; i < len; i++){
        koopa_raw_value_t tmp =reinterpret_cast<koopa_raw_value_t>(value->kind.data.aggregate.elems.buffer [i]);
        if(tmp->kind.tag == KOOPA_RVT_AGGREGATE)Add_UsedBy_Aggregate( tmp  , inst);
        //不再是一个聚合体了，而是一个赋值，可能是float，int或symref
        else if(tmp->name){ //有名字，是一个symref
            find_sym( tmp->name )->used_by.push_back(inst);
        }
    }
}

// /// 每轮次优化循环开始前调用，进行优化初始化（ used_by建立）
void Optimizer:: init_used_by(Program * program){
    // 先清空used_by
    for (auto& pair : symtable)
        for(auto & item : pair.second)  //second还是一个map
            item.second->used_by.clear();// item.first是函数名   , second是 std::unique_ptr<SymInfo> syminfo
    //迭代所有指令，建立usedby
    for(FuncInfo * func : program->functable) {
        if(func->bb_list.size()==0)continue;
        cur_func = func->func_name;
        for(BBInfo * bb : func->bb_list) {
            cur_bb = bb->bb_name;
            for (std::list<koopa_raw_value_data* >::iterator it = bb->insts.begin(); it != bb->insts.end(); ++it) {
                //value是一条指令，对该指令做 used_by
                koopa_raw_value_data* value = *it;
                std::string symbol = "";
                SymInfo * syminfo = nullptr;
                if (value->name)symbol += value->name;
                if (value->name)syminfo = find_sym(symbol);
                switch(value->kind.tag){ //根据指令种类做不同操作
                case KOOPA_RVT_ALLOC:{ //ALLOC   @local_a = alloc i32 or @local_arr = alloc [i32,10]
                    break;
                }case KOOPA_RVT_LOAD:{ //LOAD    %1 = load @local_a  or  %1 = load %ptr0  
                    //src的变量名
                    std::string src_name( value->kind.data.load.src->name);   
                    //// used_by建立  只是使用了src
                    find_sym(src_name)->used_by.push_back(value);
                    break;
                }case KOOPA_RVT_GET_PTR:{ //GET_PTR  %ptr0 = getelemptr @arg_arr , 1
                }case KOOPA_RVT_GET_ELEM_PTR:{ //GET_ELEM_PTR  %ptr0 = getelemptr @local_arr , 1
                    /// 添加used_by
                    find_sym(value->kind.data.get_elem_ptr.src->name)->used_by.push_back(value);
                    if(value->kind.data.get_elem_ptr.index->name)find_sym(value->kind.data.get_elem_ptr.index->name)->used_by.push_back(value);
                    break;
                }case KOOPA_RVT_BINARY:{ /// Binary operation.  %3 = add  %1, %2
                    koopa_raw_value_t   lhs_val = value->kind.data.binary.lhs;
                    koopa_raw_value_t   rhs_val = value->kind.data.binary.rhs;

                    if(lhs_val->name)find_sym(lhs_val->name)->used_by.push_back(value);///设置used_by
                    if(rhs_val->name)find_sym(rhs_val->name)->used_by.push_back(value); ///设置used_by
                    break;
                }case KOOPA_RVT_CALL:{ //CALL  %4 = call @add(%3)  or   call @add(%3) 
                    //call有两种情况，有无返回值,无返回值时不是Sym_Def 
                    /// used_by
                    for(int i = 0 ; i <value->kind.data.call.args.len ; i++ ){
                        koopa_raw_value_t tmp =reinterpret_cast<koopa_raw_value_t>(value->kind.data.call.args.buffer[i]);
                        if(tmp->name)find_sym(tmp->name)->used_by.push_back(value);
                    }
                    break;
                }case KOOPA_RVT_STORE:{ //Memory store.  store %1 , @local_a
                    koopa_raw_value_t store_val = value->kind.data.store.value;
                    SymInfo * val = nullptr;
                    if(store_val->name)val = find_sym(store_val->name);  //store的value是一个变量
                    // store的目的地
                    koopa_raw_value_t dest = value->kind.data.store.dest;
                    symbol += dest->name;  //目的变量名
                    syminfo = find_sym(symbol);  //目的变量

                    //// store的目的地是一个全局变量
                    if(syminfo->isGlobal()){ ///store的目的是全局变量  //used_by建立
                        syminfo->used_by.push_back(value);
                        if(val) val->used_by.push_back(value);  //store的 value 是一个变量
                        break;
                    }
                    //// used_by的添加     还使用了dest
                    syminfo->used_by.push_back(value);
                    if(val) val->used_by.push_back(value);  //store的 value 是一个变量
                    ///store的value可能是一个aggregate
                    if(store_val->kind.tag == KOOPA_RVT_AGGREGATE)  Add_UsedBy_Aggregate(  store_val  ,  value );
                    break;
                }case KOOPA_RVT_BRANCH:{ //branch    br %4, %then0(@b2 , @a1) , %else0(@b2 , @a1)
                    /// 没有赋值啦，不需要值传递
                    //// used_by的添加
                    /// 跳转条件used
                    if(value->kind.data.branch.cond->name)find_sym(value->kind.data.branch.cond->name)->used_by.push_back(value);
                    ///基本块参数usedby
                    koopa_raw_value_t tmp = nullptr;
                    // true bb args
                    for(int i = 0 ; i < value->kind.data.branch.true_args.len ; i++ ){
                        tmp =reinterpret_cast<koopa_raw_value_t>(value->kind.data.branch.true_args.buffer[i]);
                        if(tmp->name)find_sym(tmp->name)->used_by.push_back(value);
                    }
                    // false bb args
                    for(int i = 0 ; i < value->kind.data.branch.false_args.len ; i++ ){
                        tmp =reinterpret_cast<koopa_raw_value_t>(value->kind.data.branch.false_args.buffer[i]);
                        if(tmp->name)find_sym(tmp->name)->used_by.push_back(value);
                    }
                    break;
                }case KOOPA_RVT_JUMP:{ //jump.     jump %while_entry0(@b2 , @a1)
                    /// 没有赋值啦，不需要值传递
                    /// used_by的添加 target基本块参数usedby
                    for(int i = 0 ; i < value->kind.data.jump.args.len ; i++ ){
                        koopa_raw_value_t tmp =reinterpret_cast<koopa_raw_value_t>(value->kind.data.jump.args.buffer[i]);
                        if(tmp->name)find_sym(tmp->name)->used_by.push_back(value);
                    }
                    break;
                }case KOOPA_RVT_RETURN:{ //return.   ret 0 ;
                    //// used_by的添加
                    if (value->kind.data.ret.value){ //返回值存在
                        if(value->kind.data.ret.value->name)find_sym(value->kind.data.ret.value->name)->used_by.push_back(value);
                    }
                    break;
                }default:{
                    break;
                }
                }

            }
        }
    }

}
