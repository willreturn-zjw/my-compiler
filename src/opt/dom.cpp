// #include "../../include/opt/dom.hpp"

// void SSABuilder:: build(Program & program){
//     for(FuncInfo* func: program.functable){
//         BBCheck(func); // 去除除entry外前驱为空的基本块
//         SSAFuncBuilder* ssa_builder = new SSAFuncBuilder();
//         ssa_builder->Build(func->bb_list, func->func_name);
//         map[func->func_name] = ssa_builder;
//     }
// }

// void SSABuilder:: BBCheck(FuncInfo* func){
//     if(func->bb_list.size() == 0){
//         return;
//     } // 库定义函数
//     BBInfo* entry = func->bb_list[0];
//     for(auto it = func->bb_list.begin(); it != func->bb_list.end(); it++){
//         BBInfo* bb = *it;
//         if(bb != entry && bb->prev.size() == 0){
//             for(BBInfo* next_bb : bb->next){
//                 for(auto next_it = next_bb->prev.begin(); next_it != next_bb->prev.end(); next_it ++){
//                     if(*next_it == bb){
//                         next_bb->prev.erase(next_it);
//                         break;
//                     }
//                 }
//             } // 删除所有后继节点相关的前驱
//             it = func->bb_list.erase(it);
//             if(it != func->bb_list.begin()){
//                 it--;
//             } // 删除该节点本身
//         }
//     }
// }

// extern std::string outpath;
// void SSABuilder:: exit(Program & program){
//     for(FuncInfo* func: program.functable){
//         SSAFuncBuilder* ssa_builder = map[func->func_name];
//         ssa_builder->ExitBlockParams();
//     }

//     for(FuncInfo* func: program.functable){
//         SSAFuncBuilder* ssa_builder = map[func->func_name];
//         ssa_builder->Exit();
//     }
// }

// ////***************************************************************************************************************************************
// ////*******************单个函数转SSA直接方法****************************************************************************************************
// ////***************************************************************************************************************************************
// bool SSAFuncBuilder:: isGlobalVar(const std::string str){
//     if(symtable["Global"].find(str) != symtable["Global"].end()){
//         return true;
//     }    
//     else{
//         return false;
//     }
// }

// // 输入: std::vector<BBInfo*> : 0号元素为entry
// // 保存: std::vector<BBInfo*>
// void SSAFuncBuilder:: Build(std::vector<BBInfo*>& func_basic_blocks, std::string func_name){
//     bbs = &func_basic_blocks;
//     fname = func_name;
//     temp_cnt = 0;
//     // 引用保证
//     if((*bbs).size() == 0 ){// 避免bbs为空的情况
//         return;
//     } 
//     entry = (*bbs)[0];
//     bb2sym.clear();
//     dom = new DominatorTree((*bbs), fname);
//     // 初始化变量
//     for(BBInfo* bb : (*bbs)){
//         bb2sym[bb] = LocalVariable(bb);
//         bb2info[bb] = new info();
//     }
//     // 过滤基本块变量，保留非数组类型的全局变量和局部变量
//     dom->computeDominators(entry);
//     dom->buildDomTree();
//     dom->computeDominanceFrontier();
//     dom->buildBBParams(bb2sym);

//     DFS(entry);
//     assert(bb_stack.size() == 0);
//     // 深度遍历各Block并进行重命名操作
//     // 深度遍历完成后，bb_stack应为空
// }

// // BBInfo* 存储的sym_list包含数组、全局变量和局部变量，需要过滤
// // 过滤条件:
// // value->ty.tag == array || value->name[0] == '\%' -> 数组，不进行考虑
// std::set<SymInfo*> SSAFuncBuilder:: LocalVariable(BBInfo* bb){
//     std::set<SymInfo*> sym_set;
//     for(SymInfo* sym : bb->sym_list){
//         if(sym->v_status != VarStatus::TEMP && sym->sym_addr->ty->tag != KOOPA_RTT_ARRAY && sym->sym_addr->ty->data.pointer.base->tag != KOOPA_RTT_ARRAY){
//             sym_set.insert(sym);
//         }
//     }
//     for(auto bb_insts : bb->insts){
//         if(bb_insts->kind.tag == KOOPA_RVT_CALL){
//             std::set<SymInfo*> union_set;
//             // std::set<SymInfo*> used_global = functable[bb_insts->kind.data.call.callee->name]->use_global;
//             std::set<SymInfo*> used_global = functable[fname]->use_global;

//             std::set_union(sym_set.begin(), sym_set.end(),
//                             used_global.begin(), used_global.end(),
//                             std::inserter(union_set, union_set.begin()));
//             sym_set = union_set;
//         }
//     }
//     return sym_set;
// }

// // a. 重命名Block的Param，并添加相对应的Alloc在Entry基本块
// // b. 选择栈顶元素, 继承变量状态（栈空，变量状态为空）
// // c. 将栈顶元素对应info的visited改为true
// // d. 对Block所有指令进行遍历重命名
// // e. 将当前bb对应的info中out和sym_max_cnt
// //     I) 若不存在，将对应<SymInfo*, int>插入
// //     II) 若存在，取二者最大值
// // f. 将当前block压入栈中
// // g. 遍历所有后继节点，若后继有未访问的节点，执行DFS()
// // h. 压出栈顶
// // i. 返回
// void SSAFuncBuilder:: DFS(BBInfo* bb){
//     std::unordered_map<SymInfo*, int> current_sym;
//     current_sym.clear();
//     // 创建当前基本块的符号重命名系统
//     std::set<SymInfo *> params;
//     params.clear();
//     std::set<SymInfo*, decltype(&SymInfo::Compare)> sortedParams(bb->params.begin(), bb->params.end(), SymInfo::Compare);
//     for(auto it = sortedParams.begin(); it != sortedParams.end(); it++){
//         SymInfo* param_sym = *it;
//         if(sym_max_cnt.find(param_sym) != sym_max_cnt.end()){
//             // 符号此前存在过
//             sym_max_cnt[param_sym] = sym_max_cnt[param_sym] + 1;
//         }else{
//             // 符号此前不存在
//             sym_max_cnt[param_sym] = 0;
//         }
//         current_sym[param_sym] = sym_max_cnt[param_sym];
//         std::string re_param_name = std::string(param_sym->sym_addr->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[param_sym]);
//         koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//         memcpy((void*)re_type, (const void*)param_sym->sym_addr->ty, sizeof(koopa_raw_type_kind_t));
//         current_insts = &entry->insts;
//         auto alloc_it = entry->insts.end();
//         param_sym = addAllocInst(re_param_name, re_type, alloc_it);
//         params.insert(param_sym);
//         // 此时不插入，仅在符号表中插入相关符号
//     }
//     bb->params = params;
//     // 重命名Block的Param，并添加相对应的Alloc在Entry基本块
//     addBlockParams(bb);
//     if(bb_stack.size() > 0){
//         BBInfo* lastbb = bb_stack.top();
//         for(auto out : *bb2info[lastbb]->out){
//             if(current_sym.find(out.first) == current_sym.end()){
//                 current_sym[out.first] = out.second;
//             }
//         }
//     }
//     // 如果栈顶存在元素并且在基本块参数中不存在，则将其加入当前符号重命名系统，代表数据流流入
//     current_insts = &bb->insts;
//     auto begin_it = bb->insts.begin();
//     if (bb == entry){
//         for(auto global_sym_info : functable[fname]->use_global){
//             if(global_sym_info->v_status == VarStatus::TEMP && global_sym_info->sym_addr->ty->tag != KOOPA_RTT_ARRAY && global_sym_info->sym_addr->ty->data.pointer.base->tag != KOOPA_RTT_ARRAY){
//                 std::cout << global_sym_info->sym_name  << std::endl;
//                 continue;
//             }// 不进行数组操作
//             sym_max_cnt[global_sym_info] = 0;
//             std::string re_sym_name = std::string(global_sym_info->sym_addr->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[global_sym_info]);
//             koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//             current_sym[global_sym_info] = sym_max_cnt[global_sym_info];
//             memcpy((void*)re_type, (const void*)global_sym_info->sym_addr->ty, sizeof(koopa_raw_type_kind_t));
//             auto temp_insts = current_insts;
//             // current_insts = &entry->insts;
//             SymInfo* src_global_sym_info = global_sym_info;
//             SymInfo* target_global_sym_info = addAllocInst(re_sym_name, re_type, begin_it);
//             //
//             koopa_raw_value_data* load_cur_global = new koopa_raw_value_data;
//             koopa_raw_value_data* save_cur_global = new koopa_raw_value_data; 
//             // 定义
//             std::string temp_varible_name =  std::string("\%") + std::string(fname.c_str()) + std::string("_temp_") + std::to_string(temp_cnt);
//             temp_cnt ++;
//             koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//             memcpy((void*)temp_varible_type, (const void*)src_global_sym_info->sym_addr->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//             // 赋值临时变量
//             load_cur_global->ty = temp_varible_type;
//             char* temp_varible_name_cstr = new char[temp_varible_name.size() + 1];
//             strcpy(temp_varible_name_cstr, temp_varible_name.c_str());
//             load_cur_global->name = temp_varible_name_cstr;
//             load_cur_global->kind.tag = KOOPA_RVT_LOAD;
//             load_cur_global->kind.data.load.src = src_global_sym_info->sym_addr;
//             std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_cstr, load_cur_global, VarStatus::TEMP);
//             symtable[fname][temp_varible_name_cstr] = std::move(load_info);
//             (*current_insts).insert(begin_it, load_cur_global);
//             // 插入load指令
//             koopa_raw_type_kind_t* temp_save_type = new koopa_raw_type_kind_t;
//             temp_save_type->tag = KOOPA_RTT_UNIT;
//             save_cur_global->ty = temp_save_type;
//             save_cur_global->name = nullptr;
//             save_cur_global->kind.tag = KOOPA_RVT_STORE;
//             save_cur_global->kind.data.store.value = load_cur_global;
//             save_cur_global->kind.data.store.dest = target_global_sym_info->sym_addr;
//             (*current_insts).insert(begin_it, save_cur_global);
//             // 插入store指令
//         }
//     }
//     for(auto it = begin_it; it != bb->insts.end();){
//         koopa_raw_value_data* inst = *it;
//         current_insts = &bb->insts;
//         Rename(inst, current_sym, it);
//         it++;
//     }
//     // 对Block所有指令进行遍历重命名
//     bb2info[bb]->visited = true;
//     bb2info[bb]->out = &current_sym;
//     // 更新当前块状态信息
//     for(auto pair: current_sym){
//         if(sym_max_cnt.find(pair.first) != sym_max_cnt.end()){
//             sym_max_cnt[pair.first] = std::max(sym_max_cnt[pair.first], pair.second);
//         }// 避免空指针
//         else{
//             sym_max_cnt[pair.first] = pair.second;
//         }
//     }
//     // 将当前bb对应的info中out和sym_max_cnt信息合并
//     bb_stack.push(bb);
//     // 将当前block压入栈中
//     for(BBInfo* next_bb : bb->next){
//         if(bb2info[next_bb]->visited == false){
//             DFS(next_bb);
//         }
//     }
//     // 遍历所有后继节点，若后继有未访问的节点，执行DFS()
//     bb_stack.pop();
//     // 压出栈顶
//     return;
// }

// // 此处输入共三个，inst的value，当前的符号，迭代器(方便插入)
// // 重命名规则: 原名字 + "_" + 函数名字 + "_" + 重命名序号 (主要是避免全局变量)
// // Load 语句: 仅需要更改名字
// // Store 语句: 1. 更改名字 2. 加入符号表 3. 前方插入Alloc语句
// // 注意:
// // 全局变量在现有算法中会因为Store语句变为局部变量，
// // 执行call指令时需先将全局变量对应的当前的局部变量Store至全局变量中
// // 并在执行call指令后将全局变量store至新的局部变量
// void SSAFuncBuilder:: Rename(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     switch(inst->kind.tag){
//         case KOOPA_RVT_ALLOC:
//             RenameAlloc(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_LOAD:
//             RenameLoad(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_STORE:
//             RenameStore(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_GET_PTR:
//             RenameGetPtr(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_GET_ELEM_PTR:
//             RenameGetElemPtr(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_BINARY:
//             RenameBinary(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_BRANCH:
//             RenameBranch(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_JUMP:
//             RenameJump(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_CALL:
//             RenameCall(inst, current_sym, it);
//             break;
//         case KOOPA_RVT_RETURN:
//             RenameReturn(inst, current_sym, it);
//             break;
//         default:
//             assert(false);
//             break;
//     }
// }

// // 需要变量名: string: name, 变量的类型: type, 以及插入位置(迭代器，插入是插入到当前位置的上一条)
// // 此处将重命名的变量插入符号表
// SymInfo* SSAFuncBuilder:: addAllocInst(std::string name, koopa_raw_type_kind_t* & ty, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_value_data* addr = new koopa_raw_value_data;
//     char* addr_name = new char[name.size() + 1];
//     strcpy(addr_name, name.c_str());
//     addr->name = addr_name;
//     addr->ty = ty;
//     addr->kind.tag = KOOPA_RVT_ALLOC;
//     if(it != (*current_insts).end()){
//         (*current_insts).insert(it, addr);
//     }
//     std::unique_ptr<SymInfo> result = std::make_unique<SymInfo>(name, addr, VarStatus::LOCAL);
//     symtable[fname][name] = std::move(result);
//     return symtable[fname][name].get();
// }

// // 将BBInfo*中的params转移到basic_block的args中
// void SSAFuncBuilder:: addBlockParams(BBInfo* bb){
//     koopa_raw_basic_block_data_t* bb_addr = bb->bb_addr;
//     std::vector<koopa_raw_value_t> args;
//     size_t sym_index = 0;
//     std::set<SymInfo*, decltype(&SymInfo::Compare)> sortedParams(bb->params.begin(), bb->params.end(), SymInfo::Compare);
//     for(SymInfo* sym_info : sortedParams){
//         // koopa_raw_value_data* sym_addr = new koopa_raw_value_data;
//         // sym_addr->name = sym_info->sym_addr->name;
//         // sym_addr->ty = sym_info->sym_addr->ty;
//         // sym_addr->kind.tag = KOOPA_RVT_BLOCK_ARG_REF;
//         // sym_addr->kind.data.block_arg_ref.index = sym_index;
//         // sym_index = sym_index + 1;
//         koopa_raw_value_data* sym_addr = sym_info->sym_addr;
//         sym_addr->kind.tag = KOOPA_RVT_BLOCK_ARG_REF;
//         sym_addr->kind.data.block_arg_ref.index = sym_index;
//         sym_index = sym_index + 1;
//         args.push_back(sym_addr);
//     }

//     koopa_raw_slice_t* bb_params = new koopa_raw_slice_t;
//     bb_params->len = sym_index;
//     bb_params->kind = KOOPA_RSIK_VALUE;
//     bb_params->buffer = new const void *[sym_index];
//     for(int i = 0; i < sym_index; i++){
//         bb_params->buffer[i] = static_cast<const void*> (args[i]);
//     }
//     bb_addr->params = *bb_params;
//     return;
// }

// // 删除basic_blcok的args，同时在各个block的跳转指令处加上赋值语句
// // Example:
// // Before:
// // BB0:
// //  ...
// // jump BB1(@a0, @b2)
// // BB1(@a10, @b15):
// // ...
// // After:
// // Step 1: add Store/Load to Before jump
// // BB0:
// //  ...
// // load %1, @a0
// // store %1, @a10
// // load %2, @b2
// // store %2, @b15
// // jump BB1
// // BB1(@a10, @b15):
// // ...
// // Step 2: add params's Alloc in Entry Block and delete block params
// // Entry:
// // @a10 = alloc i32
// // @b15 = alloc f32
// // BB1:
// // ...
// // ！！！！！！！！！
// // 注意事项
// // 依赖set属性，删除Block参数时，必须删除前驱相同位置的元素
// void SSAFuncBuilder:: ExitBlockParams(){
//     // Step 1: 跳转指令修改
//     for(BBInfo* bb : (*bbs)){
//         if(bb->insts.size() <= 0){
//             continue;
//         }
//         current_insts = &bb->insts;
//         auto end_ptr = --(bb->insts.end());
//         koopa_raw_value_data* end_inst = *end_ptr;
//         switch(end_inst->kind.tag){
//             case KOOPA_RVT_JUMP:
//             {
//                 koopa_raw_slice_t *args = &end_inst->kind.data.jump.args;
//                 BBInfo* target_bb = bbtable[fname][std::string(end_inst->kind.data.jump.target->name)].get();
//                 for(int i = 0 ; i < (*args).len ; i++){
//                     koopa_raw_value_t src = reinterpret_cast<koopa_raw_value_t>((*args).buffer[i]);
//                     koopa_raw_value_t dest = reinterpret_cast<koopa_raw_value_t>(target_bb->bb_addr->params.buffer[i]);
//                     //////////////////////////////////////////////////////
//                     // 首先判断是否为常量
//                     if(src->ty->tag == KOOPA_RTT_INT32 || src->ty->tag == KOOPA_RTT_FLOAT32 || (src->name && symtable[fname][src->name]->v_status == VarStatus::TEMP)){
//                         koopa_raw_value_data* store = new koopa_raw_value_data;
//                         koopa_raw_type_kind_t* temp_store_type = new koopa_raw_type_kind_t;
//                         temp_store_type->tag = KOOPA_RTT_UNIT;
//                         store->ty = temp_store_type;
//                         store->name = nullptr;
//                         store->kind.tag = KOOPA_RVT_STORE;
//                         store->kind.data.store.value = src;
//                         store->kind.data.store.dest = dest;
//                         (*current_insts).insert(end_ptr, store);
//                     }
//                     /////////////////////////////////////////////////////
//                     // 以下代码为非常量形式
//                     else{
//                         koopa_raw_value_data* load = new koopa_raw_value_data;
//                         koopa_raw_value_data* store = new koopa_raw_value_data;
//                         // 定义
//                         std::string temp_varible_name = "\%" + std::string(fname.c_str()) + "_temp_" + std::to_string(temp_cnt);
//                         temp_cnt ++;
//                         koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//                         memcpy((void*)temp_varible_type, (const void*)src->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//                         // 赋值临时变量
//                         load->ty = temp_varible_type;
//                         char* temp_varible_name_c_str = new char[temp_varible_name.size() + 1];
//                         strcpy(temp_varible_name_c_str, temp_varible_name.c_str());
//                         load->name = temp_varible_name_c_str;
//                         load->kind.tag = KOOPA_RVT_LOAD;
//                         load->kind.data.load.src = src;
//                         (*current_insts).insert(end_ptr, load);
//                         std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_c_str, load, VarStatus::TEMP);
//                         symtable[fname][temp_varible_name_c_str] = std::move(load_info);
//                         // 插入load指令
//                         koopa_raw_type_kind_t* temp_store_type = new koopa_raw_type_kind_t;
//                         temp_store_type->tag = KOOPA_RTT_UNIT;
//                         store->ty = temp_store_type;
//                         store->name = nullptr;
//                         store->kind.tag = KOOPA_RVT_STORE;
//                         store->kind.data.store.value = load;
//                         store->kind.data.store.dest = dest;
//                         (*current_insts).insert(end_ptr, store);
//                         // 插入store指令
//                     }
//                 }
//                 (*args).len = 0;
//                 (*args).buffer = nullptr;
//             }
//                 break;
//             case KOOPA_RVT_BRANCH:
//             {
//                 koopa_raw_value_t cond = end_inst->kind.data.branch.cond;
//                 if(cond == *std::prev(end_ptr)){
//                     end_ptr = std::prev(end_ptr);
//                 }
//                 koopa_raw_slice_t *true_args = &end_inst->kind.data.branch.true_args;
//                 BBInfo* true_bb = bbtable[fname][std::string(end_inst->kind.data.branch.true_bb->name)].get();
//                 koopa_raw_slice_t *false_args = &end_inst->kind.data.branch.false_args;
//                 BBInfo* false_bb = bbtable[fname][std::string(end_inst->kind.data.branch.false_bb->name)].get();
//                 // true args
//                 for(int i = 0 ; i < (*true_args).len ; i++){
//                     koopa_raw_value_t src = reinterpret_cast<koopa_raw_value_t>((*true_args).buffer[i]);
//                     koopa_raw_value_t dest = reinterpret_cast<koopa_raw_value_t>(true_bb->bb_addr->params.buffer[i]);
//                     bool isTempVar = false;
//                     if(src->name && symtable[fname][src->name]->v_status == VarStatus::TEMP){
//                         isTempVar = true;
//                     }
//                     //////////////////////////////////////////////////////
//                     // 首先判断是否为常量
//                     if(isTempVar || src->ty->tag == KOOPA_RTT_INT32 || src->ty->tag == KOOPA_RTT_FLOAT32){
//                         koopa_raw_value_data* store = new koopa_raw_value_data;
//                         koopa_raw_type_kind_t* temp_store_type = new koopa_raw_type_kind_t;
//                         temp_store_type->tag = KOOPA_RTT_UNIT;
//                         store->ty = temp_store_type;
//                         store->name = nullptr;
//                         store->kind.tag = KOOPA_RVT_STORE;
//                         store->kind.data.store.value = src;
//                         store->kind.data.store.dest = dest;
//                         if(isTempVar){
//                             if(src != *end_ptr){
//                                 (*current_insts).insert(end_ptr, store);
//                             }else{
//                                 (*current_insts).insert(std::next(end_ptr), store);
//                             }
//                         }else{
//                             (*current_insts).insert(end_ptr, store);
//                         }
//                     }
//                     /////////////////////////////////////////////////////
//                     // 以下代码为非常量形式
//                     else{
//                         koopa_raw_value_data* load = new koopa_raw_value_data;
//                         koopa_raw_value_data* store = new koopa_raw_value_data;
//                         // 定义
//                         std::string temp_varible_name = "\%" + std::string(fname.c_str()) + "_temp_" + std::to_string(temp_cnt);
//                         temp_cnt ++;
//                         koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//                         memcpy((void*)temp_varible_type, (const void*)src->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//                         // 赋值临时变量
//                         load->ty = temp_varible_type;
//                         char* temp_varible_name_c_str = new char[temp_varible_name.size() + 1];
//                         strcpy(temp_varible_name_c_str, temp_varible_name.c_str());
//                         load->name = temp_varible_name_c_str;
//                         load->kind.tag = KOOPA_RVT_LOAD;
//                         load->kind.data.load.src = src;
//                         (*current_insts).insert(end_ptr, load);
//                         std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_c_str, load, VarStatus::TEMP);
//                         symtable[fname][temp_varible_name_c_str] = std::move(load_info);
//                         // 插入load指令
//                         koopa_raw_type_kind_t* temp_store_type = new koopa_raw_type_kind_t;
//                         temp_store_type->tag = KOOPA_RTT_UNIT;
//                         store->ty = temp_store_type;
//                         store->name = nullptr;
//                         store->kind.tag = KOOPA_RVT_STORE;
//                         store->kind.data.store.value = load;
//                         store->kind.data.store.dest = dest;
//                         (*current_insts).insert(end_ptr, store);
//                         // 插入store指令
//                     }
//                 }
//                 (*true_args).len = 0;
//                 (*true_args).buffer = nullptr;
//                 // false args
//                 for(int i = 0 ; i < (*false_args).len ; i++){
//                     koopa_raw_value_t src = reinterpret_cast<koopa_raw_value_t>((*false_args).buffer[i]);
//                     koopa_raw_value_t dest = reinterpret_cast<koopa_raw_value_t>(false_bb->bb_addr->params.buffer[i]);
//                     bool isTempVar = false;
//                     if(src->name && symtable[fname][src->name]->v_status == VarStatus::TEMP){
//                         isTempVar = true;
//                     }
//                     //////////////////////////////////////////////////////
//                     // 首先判断是否为常量
//                     if(isTempVar || src->ty->tag == KOOPA_RTT_INT32 || src->ty->tag == KOOPA_RTT_FLOAT32){
//                         koopa_raw_value_data* store = new koopa_raw_value_data;
//                         koopa_raw_type_kind_t* temp_store_type = new koopa_raw_type_kind_t;
//                         temp_store_type->tag = KOOPA_RTT_UNIT;
//                         store->ty = temp_store_type;
//                         store->name = nullptr;
//                         store->kind.tag = KOOPA_RVT_STORE;
//                         store->kind.data.store.value = src;
//                         store->kind.data.store.dest = dest;
//                         if(isTempVar){
//                             if(src != *end_ptr){
//                                 (*current_insts).insert(end_ptr, store);
//                             }else{
//                                 (*current_insts).insert(std::next(end_ptr), store);
//                             }
//                         }else{
//                             (*current_insts).insert(end_ptr, store);
//                         }
//                     }
//                     /////////////////////////////////////////////////////
//                     // 以下代码为非常量形式
//                     else{
//                         koopa_raw_value_data* load = new koopa_raw_value_data;
//                         koopa_raw_value_data* store = new koopa_raw_value_data;
//                         // 定义
//                         std::string temp_varible_name = "\%" + std::string(fname.c_str()) + "_temp_" + std::to_string(temp_cnt);
//                         temp_cnt ++;
//                         koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//                         memcpy((void*)temp_varible_type, (const void*)src->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//                         // 赋值临时变量
//                         load->ty = temp_varible_type;
//                         char* temp_varible_name_c_str = new char[temp_varible_name.size() + 1];
//                         strcpy(temp_varible_name_c_str, temp_varible_name.c_str());
//                         load->name = temp_varible_name_c_str;
//                         load->kind.tag = KOOPA_RVT_LOAD;
//                         load->kind.data.load.src = src;
//                         (*current_insts).insert(end_ptr, load);
//                         std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_c_str, load, VarStatus::TEMP);
//                         symtable[fname][temp_varible_name_c_str] = std::move(load_info);
//                         // 插入load指令
//                         koopa_raw_type_kind_t* temp_store_type = new koopa_raw_type_kind_t;
//                         temp_store_type->tag = KOOPA_RTT_UNIT;
//                         store->ty = temp_store_type;
//                         store->name = nullptr;
//                         store->kind.tag = KOOPA_RVT_STORE;
//                         store->kind.data.store.value = load;
//                         store->kind.data.store.dest = dest;
//                         (*current_insts).insert(end_ptr, store);
//                         // 插入store指令
//                     }
//                 }
//                 (*false_args).len = 0;
//                 (*false_args).buffer = nullptr;
//             }
//                 break;
//             default:
//                 continue; // 无跳转指令，继续下一个BB跳转指令的修改
//                 break;
//         }
//     }
//     // Step 2: 基本块参数修改
//     for(BBInfo* bb : (*bbs)){
//         koopa_raw_slice_t* param_args = &bb->bb_addr->params;
//         current_insts = &entry->insts;
//         for(int i = 0 ; i < (*param_args).len ; i++){
//             koopa_raw_value_t param = reinterpret_cast<koopa_raw_value_t>((*param_args).buffer[i]);
//             koopa_raw_value_data* sym_addr = symtable[fname][std::string(param->name)].get()->sym_addr;
//             sym_addr->kind.tag = KOOPA_RVT_ALLOC;
//             (*current_insts).insert(entry->insts.begin(), sym_addr);
//         }
//         (*param_args).len = 0;
//         (*param_args).buffer = nullptr;
//     }
//     return;
// } // 依赖set属性，删除Block参数时，必须删除前驱相同位置的元素


// void SSAFuncBuilder:: Exit(){
//     // 遍历所有块，退出重命名形式
//     for(auto pair : bb2info){
//         pair.second->visited = false;
//     } // 所有基本块访问状态为false
//     if((*bbs).size() > 0){
//         BBInfo* entry = (*bbs)[0];
//         ExitDFS(entry);    
//         ExitTemp();
//     }
//     return;
// }

// void SSAFuncBuilder:: ExitTemp(){
//     for(auto pair : exit_temp_table){
//         std::vector<exit_info*> & exit_infos = pair.second;
//         if(exit_infos.size() == 1){
//             if(exit_infos[0]->inst->kind.tag == KOOPA_RVT_LOAD){
//                 exit_infos[0]->bb->insts.remove(exit_infos[0]->inst);
//             }
//             exit_infos.clear();
//         } //  理论上优化后不存在
//         else{
//             koopa_raw_value_data* load = nullptr;
//             exit_info load_info;
//             std::vector<exit_info*> exit_stores;
//             exit_stores.clear();
//             for(auto e_info : exit_infos){
//                 if((e_info->inst->kind.tag == KOOPA_RVT_GET_PTR) || (e_info->inst->kind.tag == KOOPA_RVT_GET_ELEM_PTR)){
//                     break;
//                 } // 不作数组
//                 else if(e_info->inst->kind.tag == KOOPA_RVT_LOAD){
//                     load_info = *e_info;
//                     load = e_info->inst;
//                 }else if(e_info->inst->kind.tag == KOOPA_RVT_STORE){
//                     exit_stores.push_back(e_info);
//                 } // 虽然数组的ptr也会有对应的STORE，但是没有对应LOAD，不会产生影响，格式一定是LOAD的src和STORE的dest匹配
//             }
//             if(load != nullptr){
//                 koopa_raw_value_t load_src = load->kind.data.load.src;
//                 for(exit_info* exit_store : exit_stores){
//                     koopa_raw_value_t src_dest = exit_store->inst->kind.data.store.dest;
//                     if(load_src == src_dest){
//                         if(JudgeDelete(load_info, *exit_store)){
//                             exit_store->bb->insts.remove(exit_store->inst);
//                             exit_infos.erase(std::remove(exit_infos.begin(), exit_infos.end(), exit_store), exit_infos.end());
//                         }
//                     }
//                 }
//                 if(exit_infos.size() == 1){
//                     if(exit_infos[0]->inst->kind.tag == KOOPA_RVT_LOAD){
//                         exit_infos[0]->bb->insts.remove(exit_infos[0]->inst);
//                     }
//                     exit_infos.clear();
//                 }// 只剩load指令
//             }
//         }
//     }
// }

// bool SSAFuncBuilder:: JudgeDelete(exit_info load, exit_info store){
//     if(load.bb == store.bb){
//         BBInfo* bb = load.bb;
//         bool begin = false;
//         bool CanDelete = true;
//         for(auto it = bb->insts.begin(); (*it) != store.inst && it != bb->insts.end(); it++){
//             if((*it) == load.inst){
//                 begin = true;
//             } 
//             if(begin){
//                 if((*it)->kind.tag == KOOPA_RVT_STORE){
//                     koopa_raw_value_t temp_target = (*it)->kind.data.store.dest;
//                     if(temp_target == store.inst->kind.data.store.dest){
//                         CanDelete = false;
//                         return CanDelete;
//                     }
//                 }
//             }
//         }
//         return CanDelete;
//     }else{
//         return false;
//     }
// }


// void SSAFuncBuilder:: ExitDFS(BBInfo* bb){
//     bb_stack.push(bb);
//     bb2info[bb]->visited = true;
//     for(auto it = bb->insts.begin(); it != bb->insts.end();){
//         koopa_raw_value_data* inst = *it;
//         current_insts = &bb->insts;
//         ExitInst(inst, it);
//     }
//     // 将当前block压入栈中
//     for(BBInfo* next_bb : bb->next){
//         if(bb2info[next_bb]->visited == false){
//             ExitDFS(next_bb);
//         }
//     }
//     // 遍历所有后继节点，若后继有未访问的节点，执行DFS()
//     bb_stack.pop();
//     // 压出栈顶
//     return;
// }

// void SSAFuncBuilder:: ExitInst(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     switch (inst->kind.tag)
//     {
//     case KOOPA_RVT_ALLOC:
//         ExitAlloc(inst, it);
//         break;
//     case KOOPA_RVT_LOAD:
//         ExitLoad(inst, it);
//         break;
//     case KOOPA_RVT_STORE:
//         ExitStore(inst, it);
//         break;
//     case KOOPA_RVT_GET_PTR:
//         ExitGetPtr(inst, it);
//         break;
//     case KOOPA_RVT_GET_ELEM_PTR:
//         ExitGetElemPtr(inst, it);
//         break;
//     case KOOPA_RVT_BINARY:
//         ExitBinary(inst, it);
//         break;
//     case KOOPA_RVT_BRANCH:
//         ExitBranch(inst, it);
//         break;
//     case KOOPA_RVT_JUMP:
//         ExitJump(inst, it);
//         break;
//     case KOOPA_RVT_CALL:
//         ExitCall(inst, it);
//         break;
//     case KOOPA_RVT_RETURN:
//         ExitReturn(inst, it);
//         break;
//     default:
//         assert(false);
//         break;
//     }
// }

// int SSAFuncBuilder:: GetRenameCnt(std::string name){
//     size_t pos = name.rfind('_');
//     if (pos != std::string::npos) {
//         // 提取下划线后的子字符串
//         std::string numStr = name.substr(pos + 1);
//         // 将字符串转换为整数
//         int num = std::stoi(numStr);
//         return num;
//     } else {
//         return -1;
//     }
// }

// void SSAFuncBuilder:: ExitAlloc(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     if(inst->name){
//         if(inst->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY){
//             it++;
//             return;
//         }// 不做数组
//         if(symtable[fname].find(inst->name) == symtable[fname].end()){
//             assert(false);
//         }else{
//             std::string name = std::string(inst->name);
//             std::string init_name = GetSymInitName(name);
//             if(exit_sym_visited.find(init_name) == exit_sym_visited.end()){
//                 exit_sym_visited.insert(init_name);
//                 // 重命名并添加到exit_temp_table
//                 if(symtable[fname].find(init_name) != symtable[fname].end()){
//                     koopa_raw_value_data* init_inst = symtable[fname][init_name]->sym_addr;
//                     current_insts->insert(it, init_inst);
//                 }
//             }
//             auto temp_it = it;
//             it = (*current_insts).erase(temp_it);
//             if(std::next(it) == current_insts->end()){
//                 it = std::next(it);
//             }
//             // 删除重命名后的alloc语句
//             //
//             // int num = GetRenameCnt(name);
//             // if(num < 0){
//             //     assert(false);
//             // }else{
//             //     if(num == 0){
//             //         std::string init_name = GetSymInitName(name);
//             //         if(symtable[fname].find(init_name) != symtable[fname].end()){
//             //             koopa_raw_value_data* init_inst = symtable[fname][init_name]->sym_addr;
//             //             current_insts->insert(it, init_inst);
//             //         }
//             //     }
//             //     auto temp_it = it;
//             //     it = (*current_insts).erase(temp_it);
//             //     // 删除重命名后的alloc语句
//             // }
//         }
//     }else{
//         assert(false);
//     }
// }

// void SSAFuncBuilder:: ExitLoad(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     if(inst->name){
//         std::string name = std::string(inst->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     }// 非变量一定不存在名字
//     else{
//         assert(false);
//     }// 更新临时变量信息
//     koopa_raw_load_t & load = inst->kind.data.load;
//     std::string name = std::string(load.src->name);
//     std::string init_name = GetSymInitName(name);
//     if(symtable["Global"].find(init_name) != symtable["Global"].end()){
//         load.src = symtable["Global"][init_name]->sym_addr; // 原符号为全局变量
//     }else if(symtable[fname].find(init_name) != symtable[fname].end()){
//         load.src = symtable[fname][init_name]->sym_addr; // 原符号为局部变量
//     }else{
//         assert(false); // 原符号一定在符号表中
//     }
//     it++;
//     return;
// }

// void SSAFuncBuilder:: ExitStore(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_store_t & store = inst->kind.data.store;
//     koopa_raw_value_t value = store.value;
//     if(value->name){
//         std::string name = std::string(value->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     }// 非变量一定不存在名字
//     else if(value->kind.tag == KOOPA_RVT_AGGREGATE){
//         ExitAggregate(inst, value);
//     }

//     std::string name = std::string(store.dest->name);
//     // store的dest需先判定是否为临时变量(数组情况)
//     if(symtable[fname].find(name) != symtable[fname].end()){
//         if(symtable[fname][name]->v_status == VarStatus::TEMP){
//             SymInfo* temp_var = symtable[fname][name].get();
//             if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                 std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                 temp_info->clear();
//                 exit_temp_table[temp_var] = *temp_info;
//                 exit_info* exit_temp_info = new exit_info;
//                 exit_temp_info->bb = bb_stack.top();
//                 exit_temp_info->inst = inst;
//                 exit_temp_table[temp_var].push_back(exit_temp_info);
//             }else{
//                 exit_info* exit_temp_info = new exit_info;
//                 exit_temp_info->bb = bb_stack.top();
//                 exit_temp_info->inst = inst;
//                 exit_temp_table[temp_var].push_back(exit_temp_info);
//             }
//             it++;
//             return;
//         }
//     }else if(symtable["Global"].find(name) != symtable["Global"].end()){
//         it++;
//         return;
//     } // 全局数组情况
//     else{
//         assert(false); // 非临时变量/全局变量一定在符号表中
//     }
//     std::string init_name = GetSymInitName(name);
//     if(symtable["Global"].find(init_name) != symtable["Global"].end()){
//         store.dest = symtable["Global"][init_name]->sym_addr; // 原符号为全局变量
//     }else if(symtable[fname].find(init_name) != symtable[fname].end()){
//         store.dest = symtable[fname][init_name]->sym_addr; // 原符号为局部变量
//     }else{
//         assert(false); // 原符号一定在符号表中
//     }
//     it++;
//     return;
// }

// void SSAFuncBuilder:: ExitAggregate(koopa_raw_value_data* inst, koopa_raw_value_t value){
//     if(value->kind.tag == KOOPA_RVT_AGGREGATE){
//         koopa_raw_aggregate_t aggregate = value->kind.data.aggregate;
//         for(int i = 0; i < aggregate.elems.len; i++){
//             ExitAggregate(inst, reinterpret_cast<koopa_raw_value_t>(aggregate.elems.buffer[i]));
//         }
//     }else{
//         if(value->name){
//             std::string name = std::string(value->name);
//             if(symtable[fname].find(name) == symtable[fname].end()){
//                 assert(false);
//                 //临时变量一定存于symtable中
//             }else{
//                 if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                     SymInfo* temp_var = symtable[fname][name].get();
//                     if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                         std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                         temp_info->clear();
//                         exit_temp_table[temp_var] = *temp_info;
//                         exit_info* exit_temp_info = new exit_info;
//                         exit_temp_info->bb = bb_stack.top();
//                         exit_temp_info->inst = inst;
//                         exit_temp_table[temp_var].push_back(exit_temp_info);
//                     }else{
//                         exit_info* exit_temp_info = new exit_info;
//                         exit_temp_info->bb = bb_stack.top();
//                         exit_temp_info->inst = inst;
//                         exit_temp_table[temp_var].push_back(exit_temp_info);
//                     }
//                 }
//             }
//         } // 非变量一定不存在名字
//     }
// }

// void SSAFuncBuilder:: ExitGetPtr(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     if(inst->name){
//         std::string name = std::string(inst->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     else{
//         assert(false);
//     }

//     if(inst->kind.data.get_ptr.src->name){
//         std::string name = std::string(inst->kind.data.get_ptr.src->name);
//         if(symtable[fname].find(name) != symtable[fname].end()){
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字

//     if(inst->kind.data.get_ptr.index->name){
//         std::string name = std::string(inst->kind.data.get_ptr.index->name);
//         if(symtable[fname].find(name) != symtable[fname].end()){
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字

//     it++;
//     return;
// }

// void SSAFuncBuilder:: ExitGetElemPtr(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     if(inst->name){
//         std::string name = std::string(inst->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     else{
//         assert(false);
//     }

//     if(inst->kind.data.get_elem_ptr.src->name){
//         std::string name = std::string(inst->kind.data.get_elem_ptr.src->name);
//         if(symtable[fname].find(name) != symtable[fname].end()){
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字

//     if(inst->kind.data.get_elem_ptr.index->name){
//         std::string name = std::string(inst->kind.data.get_elem_ptr.index->name);
//         if(symtable[fname].find(name) != symtable[fname].end()){
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字

//     it++;
//     return;
// }

// void SSAFuncBuilder:: ExitBinary(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     if(inst->name){
//         std::string name = std::string(inst->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     else{
//         assert(false);
//     }
//     // 目标值
//     if(inst->kind.data.binary.lhs->name){
//         std::string name = std::string(inst->kind.data.binary.lhs->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     // 左操作数
//     if(inst->kind.data.binary.rhs->name){
//         std::string name = std::string(inst->kind.data.binary.rhs->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     // 右操作数
//     it++;
//     return;
// } // 仅涉及临时变量

// void SSAFuncBuilder:: ExitBranch(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_branch_t & branch = inst->kind.data.branch;
//     if(branch.cond->name){
//         std::string name = std::string(branch.cond->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     it++;
//     return;
// } // 仅涉及临时变量

// void SSAFuncBuilder:: ExitJump(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     it++;
//     return;
// } // 仅涉及临时变量

// void SSAFuncBuilder:: ExitCall(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_call_t & call = inst->kind.data.call;
//     for(int i = 0; i < call.args.len; i++){
//         koopa_raw_value_t arg = (koopa_raw_value_t)call.args.buffer[i];
//         if(arg->name){
//             std::string name = std::string(arg->name);
//             if(symtable[fname].find(name) == symtable[fname].end()){
//                 assert(false);
//                 //临时变量一定存于symtable中
//             }else{
//                 if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                     SymInfo* temp_var = symtable[fname][name].get();
//                     if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                         std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                         temp_info->clear();
//                         exit_temp_table[temp_var] = *temp_info;
//                         exit_info* exit_temp_info = new exit_info;
//                         exit_temp_info->bb = bb_stack.top();
//                         exit_temp_info->inst = inst;
//                         exit_temp_table[temp_var].push_back(exit_temp_info);
//                     }else{
//                         exit_info* exit_temp_info = new exit_info;
//                         exit_temp_info->bb = bb_stack.top();
//                         exit_temp_info->inst = inst;
//                         exit_temp_table[temp_var].push_back(exit_temp_info);
//                     }
//                 }
//             }
//         } // 非变量一定不存在名字
//     }
//     it++;
//     return;
// } // 仅涉及临时变量

// void SSAFuncBuilder:: ExitReturn(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_return_t & ret = inst->kind.data.ret;
//     if(ret.value && ret.value->name){
//         std::string name = std::string(ret.value->name);
//         if(symtable[fname].find(name) == symtable[fname].end()){
//             assert(false);
//             //临时变量一定存于symtable中
//         }else{
//             if(symtable[fname][name]->v_status == VarStatus::TEMP){
//                 SymInfo* temp_var = symtable[fname][name].get();
//                 if(exit_temp_table.find(temp_var) == exit_temp_table.end()){
//                     std::vector<exit_info*>* temp_info = new std::vector<exit_info*>;
//                     temp_info->clear();
//                     exit_temp_table[temp_var] = *temp_info;
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }else{
//                     exit_info* exit_temp_info = new exit_info;
//                     exit_temp_info->bb = bb_stack.top();
//                     exit_temp_info->inst = inst;
//                     exit_temp_table[temp_var].push_back(exit_temp_info);
//                 }
//             }
//         }
//     } // 非变量一定不存在名字
//     it++;
//     return;
// }

// // 获取此处SSABuilder对应的基本块
// std::vector<BBInfo*> SSAFuncBuilder:: getSSABB(){
//     return (*bbs);
// }

// void SSAFuncBuilder:: RenameAlloc(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     if(inst->ty->tag == KOOPA_RTT_ARRAY || (symtable[fname].find(inst->name) != symtable[fname].end() && symtable[fname][inst->name]->v_status == VarStatus::TEMP) || inst->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY){
//         return;   
//     }
//     // 不进行数组操作    
//     else{
//         std::string sym_name = std::string(inst->name);
//         SymInfo* sym_info;
//         if(!isGlobalVar(sym_name)){
//             sym_info = symtable[fname][sym_name].get();
//         }
//         else{
//             sym_info = symtable["Global"][sym_name].get();
//         }
//         if(sym_max_cnt.find(sym_info) == sym_max_cnt.end()){
//             sym_max_cnt[sym_info] = 0;
//         }else{
//             sym_max_cnt[sym_info] = sym_max_cnt[sym_info] + 1;
//         }
//         std::string re_sym_name = std::string(inst->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[sym_info]);
//         koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//         current_sym[sym_info] = sym_max_cnt[sym_info];
//         memcpy((void*)re_type, (const void*)inst->ty, sizeof(koopa_raw_type_kind_t));
//         addAllocInst(re_sym_name, re_type, it);
//         auto temp_it = it;
//         it = (*current_insts).erase(temp_it);
//         it--;
//         return;
//     }
// }

// void SSAFuncBuilder:: RenameLoad(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_load_t & load = inst->kind.data.load;
//     koopa_raw_value_t & src = load.src;
//     if(src->ty->tag == KOOPA_RTT_ARRAY || (symtable[fname].find(src->name) != symtable[fname].end() && symtable[fname][src->name]->v_status == VarStatus::TEMP) || src->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY){
//         return;
//     }
//     std::string sym_name = std::string(src->name);
//     SymInfo* sym_info;
//     bool hasGlobalVar = false;
//     if(!isGlobalVar(sym_name)){
//         sym_info = symtable[fname][sym_name].get();
//     }
//     else{
//         sym_info = symtable["Global"][sym_name].get();
//         hasGlobalVar = true;
//     }
//     if(current_sym.find(sym_info) == current_sym.end()){
//         if(sym_max_cnt.find(sym_info) == sym_max_cnt.end()){
//             sym_max_cnt[sym_info] = 0;
//             std::string re_sym_name = std::string(src->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[sym_info]);
//             koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//             current_sym[sym_info] = sym_max_cnt[sym_info];
//             memcpy((void*)re_type, (const void*)src->ty, sizeof(koopa_raw_type_kind_t));
//             auto temp_insts = current_insts;
//             // current_insts = &entry->insts;
//             // auto alloc_it = entry->insts.begin();
//             auto alloc_it = it;
//             SymInfo* init_sym_info = sym_info;
//             SymInfo* sym_info = addAllocInst(re_sym_name, re_type, alloc_it);
//             //
//             if(hasGlobalVar){
//                 koopa_raw_value_data* load_cur_global = new koopa_raw_value_data;
//                 koopa_raw_value_data* save_cur_global = new koopa_raw_value_data; 
//                 // 定义
//                 std::string temp_varible_name =  std::string("\%") + std::string(fname.c_str()) + std::string("_temp_") + std::to_string(temp_cnt);
//                 temp_cnt ++;
//                 koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//                 memcpy((void*)temp_varible_type, (const void*)init_sym_info->sym_addr->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//                 // 赋值临时变量
//                 load_cur_global->ty = temp_varible_type;
//                 char* temp_varible_name_cstr = new char[temp_varible_name.size() + 1];
//                 strcpy(temp_varible_name_cstr, temp_varible_name.c_str());
//                 load_cur_global->name = temp_varible_name_cstr;
//                 load_cur_global->kind.tag = KOOPA_RVT_LOAD;
//                 load_cur_global->kind.data.load.src = init_sym_info->sym_addr;
//                 std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_cstr, load_cur_global, VarStatus::TEMP);
//                 symtable[fname][temp_varible_name_cstr] = std::move(load_info);
//                 // 插入load指令
//                 koopa_raw_type_kind_t* temp_save_type = new koopa_raw_type_kind_t;
//                 temp_save_type->tag = KOOPA_RTT_UNIT;
//                 save_cur_global->ty = temp_save_type;
//                 save_cur_global->name = nullptr;
//                 save_cur_global->kind.tag = KOOPA_RVT_STORE;
//                 save_cur_global->kind.data.store.value = load_cur_global;
//                 save_cur_global->kind.data.store.dest = sym_info->sym_addr;
//                 // 插入store指令
//                 (*current_insts).insert(alloc_it, load_cur_global);
//                 (*current_insts).insert(alloc_it, save_cur_global);
//             }
//             // 从代码中，将全局变量读出来，存进去
//             current_insts = temp_insts;
//             load.src = sym_info->sym_addr; // 替换原有变量名
//             return;
//         } // 全局变量情况
//         else{
//             sym_max_cnt[sym_info] = sym_max_cnt[sym_info] + 1;
//             current_sym[sym_info] = sym_max_cnt[sym_info];
//             std::string re_sym_name = std::string(src->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[sym_info]);
//             koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//             current_sym[sym_info] = sym_max_cnt[sym_info];
//             memcpy((void*)re_type, (const void*)src->ty, sizeof(koopa_raw_type_kind_t));
//             auto temp_insts = current_insts;
//             // current_insts = &entry->insts;
//             // auto alloc_it = entry->insts.begin();
//             auto alloc_it = it;
//             SymInfo* sym_info = addAllocInst(re_sym_name, re_type, alloc_it);
//             current_insts = temp_insts;
//             load.src = sym_info->sym_addr; // 替换原有变量名
//             return;
//         } // 多分支可能不会被定义所有变量在Load前势必被定义
//     }
//     else{
//         std::string re_sym_name = std::string(src->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[sym_info]);
//         load.src = symtable[fname][re_sym_name].get()->sym_addr;
//         return;
//     }
// }

// void SSAFuncBuilder:: RenameStore(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_store_t & store = inst->kind.data.store;
//     // store的src必定为临时变量
//     koopa_raw_value_t & dest = store.dest;
//     bool hasGlobalVar = false;
//     if(dest->ty->tag == KOOPA_RTT_ARRAY || (symtable[fname].find(dest->name) != symtable[fname].end() && symtable[fname][dest->name]->v_status == VarStatus::TEMP) || dest->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY){
//         return;
//     }
//     std::string sym_name = std::string(dest->name); 
//     SymInfo* sym_info;
//     if(!isGlobalVar(sym_name)){
//         sym_info = symtable[fname][sym_name].get();
//     }
//     else{
//         hasGlobalVar = true;
//         sym_info = symtable["Global"][sym_name].get();
//     }
//     if(current_sym.find(sym_info) == current_sym.end()){
//         if(sym_max_cnt.find(sym_info) == sym_max_cnt.end()){
//             sym_max_cnt[sym_info] = 0;
//             std::string re_sym_name = std::string(dest->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[sym_info]);
//             koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//             current_sym[sym_info] = sym_max_cnt[sym_info];
//             memcpy((void*)re_type, (const void*)dest->ty, sizeof(koopa_raw_type_kind_t));
//             auto temp_insts = current_insts;
//             // current_insts = &entry->insts;
//             // auto alloc_it = entry->insts.begin();
//             auto alloc_it = it;
//             SymInfo* init_sym_info = sym_info;
//             SymInfo* sym_info = addAllocInst(re_sym_name, re_type, alloc_it);
//             //
//             if(hasGlobalVar){
//                 dest = sym_info->sym_addr;
//                 // koopa_raw_value_data* load_cur_global = new koopa_raw_value_data;
//                 // koopa_raw_value_data* save_cur_global = new koopa_raw_value_data; 
//                 // // 定义
//                 // std::string temp_varible_name =  std::string("\%") + std::string(fname.c_str()) + std::string("_temp_") + std::to_string(temp_cnt);
//                 // temp_cnt ++;
//                 // koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//                 // memcpy((void*)temp_varible_type, (const void*)init_sym_info->sym_addr->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//                 // // 赋值临时变量
//                 // load_cur_global->ty = temp_varible_type;
//                 // char* temp_varible_name_cstr = new char[temp_varible_name.size() + 1];
//                 // strcpy(temp_varible_name_cstr, temp_varible_name.c_str());
//                 // load_cur_global->name = temp_varible_name_cstr;
//                 // load_cur_global->kind.tag = KOOPA_RVT_LOAD;
//                 // load_cur_global->kind.data.load.src = init_sym_info->sym_addr;
//                 // std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_cstr, load_cur_global, VarStatus::TEMP);
//                 // symtable[fname][temp_varible_name_cstr] = std::move(load_info);
//                 // // 插入load指令
//                 // koopa_raw_type_kind_t* temp_save_type = new koopa_raw_type_kind_t;
//                 // temp_save_type->tag = KOOPA_RTT_UNIT;
//                 // save_cur_global->ty = temp_save_type;
//                 // save_cur_global->name = nullptr;
//                 // save_cur_global->kind.tag = KOOPA_RVT_STORE;
//                 // save_cur_global->kind.data.store.value = load_cur_global;
//                 // save_cur_global->kind.data.store.dest = sym_info->sym_addr;
//                 // // 插入store指令
//                 // (*current_insts).insert(alloc_it, load_cur_global);
//                 // (*current_insts).insert(alloc_it, save_cur_global);
//             }
//             // 从代码中，将全局变量读出来，存进去
//             current_insts = temp_insts;
//             store.dest = sym_info->sym_addr; // 替换原有变量名
//             return;
//         } // 全局变量情况
//         else{
//             sym_max_cnt[sym_info] = sym_max_cnt[sym_info] + 1;
//             std::string re_sym_name = std::string(dest->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(sym_max_cnt[sym_info]);
//             koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//             current_sym[sym_info] = sym_max_cnt[sym_info];
//             memcpy((void*)re_type, (const void*)dest->ty, sizeof(koopa_raw_type_kind_t));
//             auto temp_insts = current_insts;
//             // current_insts = &entry->insts;
//             // auto alloc_it = entry->insts.begin();
//             auto alloc_it = it;
//             SymInfo* sym_info = addAllocInst(re_sym_name, re_type, alloc_it);
//             current_insts = temp_insts;
//             store.dest = sym_info->sym_addr; // 替换原有变量名
//             return;
//         } // 多个ret分支
//     }
//     else{
//         if(sym_max_cnt.find(sym_info) == sym_max_cnt.end()){
//             assert(false);
//         } // 错误情况
//         else{
//             int temp_cnt = sym_max_cnt[sym_info] + 1;
//             sym_max_cnt[sym_info] = temp_cnt;
//             current_sym[sym_info] = temp_cnt;
//             std::string re_sym_name = std::string(dest->name) + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[sym_info]);
//             koopa_raw_type_kind_t* re_type = new koopa_raw_type_kind_t;
//             memcpy((void*)re_type, (const void*)sym_info->sym_addr->ty, sizeof(koopa_raw_type_kind_t));
//             SymInfo* sym_info = addAllocInst(re_sym_name, re_type, it);
//             store.dest = sym_info->sym_addr;
//             return;
//         } // alloc 新变量
//     }

// }

// void SSAFuncBuilder:: RenameGetPtr(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     return;
// }
// // GetPtr不涉及非数组形式的局部变量

// void SSAFuncBuilder:: RenameGetElemPtr(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     return;
// }
// // GetElemPtr不涉及非数组形式的局部变量

// void SSAFuncBuilder:: RenameBinary(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     return;
// }
// // Binary不涉及局部变量

// void SSAFuncBuilder:: RenameBranch(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_branch_t & branch = inst->kind.data.branch;
//     std::string false_bb_name(branch.false_bb->name);
//     std::string true_bb_name(branch.true_bb->name);
//     //
//     // false_bb
//     BBInfo* false_bb = bbtable[fname][false_bb_name].get();
//     info* false_bb_info = bb2info[false_bb];
//     std::vector<koopa_raw_value_t> false_args;
//     if(false_bb_info->visited){
//         std::set<SymInfo*, decltype(&SymInfo::Compare)> false_sortedParams(false_bb->params.begin(), false_bb->params.end(), SymInfo::Compare);
//         for(SymInfo* false_bb_sym : false_sortedParams){
//             std::string false_sym_name = GetSymInitName(false_bb_sym->sym_name);
//             SymInfo* false_sym_info;
//             if(!isGlobalVar(false_sym_name)){
//                 false_sym_info = symtable[fname][false_sym_name].get();
//             }
//             else{
//                 false_sym_info = symtable["Global"][false_sym_name].get();
//             }
//             std::string cur_false_sym_name = false_sym_name + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[false_sym_info]);
//             false_args.push_back(symtable[fname][cur_false_sym_name]->sym_addr);
//         }
//     } // 目标bb访问过，需要将目标参数还原
//     else{
//         std::set<SymInfo*, decltype(&SymInfo::Compare)> false_sortedParams(false_bb->params.begin(), false_bb->params.end(), SymInfo::Compare);
//         for(SymInfo* false_bb_sym : false_sortedParams){
//             std::string false_sym_name = false_bb_sym->sym_name;
//             SymInfo* false_sym_info;
//             if(!isGlobalVar(false_sym_name)){
//                 false_sym_info = symtable[fname][false_sym_name].get();
//             }
//             else{
//                 false_sym_info = symtable["Global"][false_sym_name].get();
//             }
//             std::string cur_false_sym_name = false_sym_name + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[false_sym_info]);
//             false_args.push_back(symtable[fname][cur_false_sym_name]->sym_addr);
//         }
//     } // 目标bb未被访问过，直接用参数传参即可
//     koopa_raw_slice_t* false_slice_params = new koopa_raw_slice_t;
//     int false_params_lenth = false_args.size();
//     false_slice_params->len = false_params_lenth;
//     false_slice_params->kind = KOOPA_RSIK_VALUE;
//     false_slice_params->buffer = new const void *[false_params_lenth];
//     for(int i = 0; i < false_params_lenth; i++){
//         false_slice_params->buffer[i] = static_cast<const void*> (false_args[i]);
//     }
//     branch.false_args = *false_slice_params;
//     //
//     // true_bb
//     BBInfo* true_bb = bbtable[fname][true_bb_name].get();
//     info* true_bb_info = bb2info[true_bb];
//     std::vector<koopa_raw_value_t> true_args;
//     if(true_bb_info->visited){
//         std::set<SymInfo*, decltype(&SymInfo::Compare)> true_sortedParams(true_bb->params.begin(), true_bb->params.end(), SymInfo::Compare);
//         for(SymInfo* true_bb_sym : true_sortedParams){
//             std::string true_sym_name = GetSymInitName(true_bb_sym->sym_name);
//             SymInfo* true_sym_info;
//             if(!isGlobalVar(true_sym_name)){
//                 true_sym_info = symtable[fname][true_sym_name].get();
//             }
//             else{
//                 true_sym_info = symtable["Global"][true_sym_name].get();
//             }
//             std::string cur_true_sym_name = true_sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[true_sym_info]);
//             true_args.push_back(symtable[fname][cur_true_sym_name]->sym_addr);
//         }
//     } // 目标bb访问过，需要将目标参数还原
//     else{
//         std::set<SymInfo*, decltype(&SymInfo::Compare)> true_sortedParams(true_bb->params.begin(), true_bb->params.end(), SymInfo::Compare);
//         for(SymInfo* true_bb_sym : true_sortedParams){
//             std::string true_sym_name = true_bb_sym->sym_name;
//             SymInfo* true_sym_info;
//             if(!isGlobalVar(true_sym_name)){
//                 true_sym_info = symtable[fname][true_sym_name].get();
//             }
//             else{
//                 true_sym_info = symtable["Global"][true_sym_name].get();
//             }
//             std::string cur_true_sym_name = true_sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[true_sym_info]);
//             true_args.push_back(symtable[fname][cur_true_sym_name]->sym_addr);
//         }
//     } // 目标bb未被访问过，直接用参数传参即可
//     koopa_raw_slice_t* true_slice_params = new koopa_raw_slice_t;
//     int true_params_lenth = true_args.size();
//     true_slice_params->len = true_params_lenth;
//     true_slice_params->kind = KOOPA_RSIK_VALUE;
//     true_slice_params->buffer = new const void *[true_params_lenth];
//     for(int i = 0; i < true_params_lenth; i++){
//         true_slice_params->buffer[i] = static_cast<const void*> (true_args[i]);
//     }
//     branch.true_args = *true_slice_params;
//     //
//     return;
// }

// void SSAFuncBuilder:: RenameJump(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_jump_t & jump = inst->kind.data.jump;
//     std::string bb_name(jump.target->name);
//     BBInfo* bb = bbtable[fname][bb_name].get();
//     info* bb_info = bb2info[bb];
//     std::vector<koopa_raw_value_t> args;
//     if(bb_info->visited){
//         std::set<SymInfo*, decltype(&SymInfo::Compare)> sortedParams(bb->params.begin(), bb->params.end(), SymInfo::Compare);
//         for(SymInfo* bb_sym : sortedParams){
//             std::string sym_name = GetSymInitName(bb_sym->sym_name);
//             SymInfo* sym_info;
//             if(!isGlobalVar(sym_name)){
//                 sym_info = symtable[fname][sym_name].get();
//             }
//             else{
//                 sym_info = symtable["Global"][sym_name].get();
//             }
//             std::string cur_sym_name = sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[sym_info]);
//             args.push_back(symtable[fname][cur_sym_name]->sym_addr);
//         }
//     } // 目标bb访问过，需要将目标参数还原
//     else{
//         std::set<SymInfo*, decltype(&SymInfo::Compare)> sortedParams(bb->params.begin(), bb->params.end(), SymInfo::Compare);
//         for(SymInfo* bb_sym : sortedParams){
//             std::string sym_name = bb_sym->sym_name;
//             SymInfo* sym_info;
//             if(!isGlobalVar(sym_name)){
//                 sym_info = symtable[fname][sym_name].get();
//             }
//             else{
//                 sym_info = symtable["Global"][sym_name].get();
//             }
//             std::string cur_sym_name = sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[sym_info]);
//             args.push_back(symtable[fname][cur_sym_name]->sym_addr);
//         }
//     } // 目标bb未被访问过，直接用参数传参即可
//     koopa_raw_slice_t* slice_params = new koopa_raw_slice_t;
//     int params_lenth = args.size();
//     slice_params->len = params_lenth;
//     slice_params->kind = KOOPA_RSIK_VALUE;
//     slice_params->buffer = new const void *[params_lenth];
//     for(int i = 0; i < params_lenth; i++){
//         slice_params->buffer[i] = static_cast<const void*> (args[i]);
//     }
//     jump.args = *slice_params;
//     return;
// }

// // 类似于寄存器的保护机制，将下一个函数用到的全局变量保护起来
// void SSAFuncBuilder:: RenameCall(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     koopa_raw_call_t & call = inst->kind.data.call;
//     // std:: string func_name(call.callee->name);
//     std:: string func_name(fname);
//     std::set<SymInfo*> & use_global = functable[func_name].get()->use_global;
//     std::set<SymInfo*> used_global;
//     used_global.clear();
//     // 变量重命名
//     for(int i = 0; i < call.args.len; i++){
//         koopa_raw_value_t arg = (koopa_raw_value_t)call.args.buffer[i];
//         if(arg->ty->tag == KOOPA_RTT_ARRAY || (symtable[fname].find(arg->name) != symtable[fname].end() && symtable[fname][arg->name]->v_status == VarStatus::TEMP) || arg->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY){
//             continue;
//         } // 不做数组
//         SymInfo* arg_info = symtable[fname][std::string(arg->name)].get();
//         std::string cur_arg_name = arg_info->sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[arg_info]);
//         call.args.buffer[i] = (const void*)symtable[fname][cur_arg_name].get()->sym_addr;
//     }
//     // 全局变量保护
//     for(SymInfo* func_global_var : use_global){
//         if(current_sym.find(func_global_var) != current_sym.end()){
//             std::string cur_global_var_name = func_global_var->sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[func_global_var]);
//             SymInfo* cur_global_var = symtable[fname][cur_global_var_name].get();
//             SaveGlobalVar(cur_global_var, it);
//             used_global.insert(func_global_var);
//         }
//     }
//     (*current_insts).insert(it, inst);    
//     for(SymInfo* func_global_var : use_global){
//         if(sym_max_cnt.find(func_global_var) != sym_max_cnt.end()){
//             sym_max_cnt[func_global_var] = sym_max_cnt[func_global_var] + 1;
//             current_sym[func_global_var] = sym_max_cnt[func_global_var];
//             std::string cur_global_var_name = func_global_var->sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[func_global_var]);
//             koopa_raw_type_kind_t* cur_global_type = new koopa_raw_type_kind_t;
//             memcpy((void*)cur_global_type, (const void*)func_global_var->sym_addr->ty, sizeof(koopa_raw_type_kind_t));
//             SymInfo* cur_global_var = addAllocInst(cur_global_var_name, cur_global_type, it);
//             LoadGlobalVar(cur_global_var, it);
//         }
//         else{
//             sym_max_cnt[func_global_var] = 0;
//             current_sym[func_global_var] = sym_max_cnt[func_global_var];
//             std::string cur_global_var_name = func_global_var->sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[func_global_var]);
//             koopa_raw_type_kind_t* cur_global_type = new koopa_raw_type_kind_t;
//             memcpy((void*)cur_global_type, (const void*)func_global_var->sym_addr->ty, sizeof(koopa_raw_type_kind_t));
//             SymInfo* cur_global_var = addAllocInst(cur_global_var_name, cur_global_type, it);
//             LoadGlobalVar(cur_global_var, it);
//         }
//     }
//     auto temp_it = it;
//     it = (*current_insts).erase(temp_it);
//     it--;
// }

// void SSAFuncBuilder:: RenameReturn(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it){
//     std::set<SymInfo*> & use_global = functable[fname].get()->use_global;
//     for(SymInfo* func_global_var : use_global){
//         if(current_sym.find(func_global_var) != current_sym.end()){ // 这一步过滤数组形式的全局变量
//             std::string cur_global_var_name = func_global_var->sym_name + + "_" + std::string(fname.c_str()) + "_" + std::to_string(current_sym[func_global_var]);
//             SymInfo* cur_global_var = symtable[fname][cur_global_var_name].get();
//             SaveGlobalVar(cur_global_var, it);
//         }
//     }    
// }

// void SSAFuncBuilder:: SaveGlobalVar(SymInfo* global_var , std::list<koopa_raw_value_data*>::iterator & it){
//     std::string init_name = GetSymInitName(global_var->sym_name);
//     SymInfo* init_global_var;
//     if(!isGlobalVar(init_name)){
//         init_global_var = symtable[fname][init_name].get();
//     }
//     else{
//         init_global_var = symtable["Global"][init_name].get();
//     }
//     koopa_raw_value_data* load_cur_global = new koopa_raw_value_data;
//     koopa_raw_value_data* save_cur_global = new koopa_raw_value_data; 
//     // 定义
//     std::string temp_varible_name =  std::string("\%") + std::string(fname.c_str()) + std::string("_temp_") + std::to_string(temp_cnt);
//     temp_cnt ++;
//     koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//     memcpy((void*)temp_varible_type, (const void*)global_var->sym_addr->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//     // 赋值临时变量
//     load_cur_global->ty = temp_varible_type;
//     char* temp_varible_name_cstr = new char[temp_varible_name.size() + 1];
//     strcpy(temp_varible_name_cstr, temp_varible_name.c_str());
//     load_cur_global->name = temp_varible_name_cstr;
//     load_cur_global->kind.tag = KOOPA_RVT_LOAD;
//     load_cur_global->kind.data.load.src = global_var->sym_addr;
//     (*current_insts).insert(it, load_cur_global);
//     std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_cstr, load_cur_global, VarStatus::TEMP);
//     symtable[fname][temp_varible_name_cstr] = std::move(load_info);
//     // 插入load指令
//     koopa_raw_type_kind_t* temp_save_type = new koopa_raw_type_kind_t;
//     temp_save_type->tag = KOOPA_RTT_UNIT;
//     save_cur_global->ty = temp_save_type;
//     save_cur_global->name = nullptr;
//     save_cur_global->kind.tag = KOOPA_RVT_STORE;
//     save_cur_global->kind.data.store.value = load_cur_global;
//     save_cur_global->kind.data.store.dest = init_global_var->sym_addr;
//     (*current_insts).insert(it, save_cur_global);
//     // 插入store指令
//  }

// void SSAFuncBuilder:: LoadGlobalVar(SymInfo* global_var , std::list<koopa_raw_value_data*>::iterator & it){
//     std::string init_name = GetSymInitName(global_var->sym_name);
//     SymInfo* init_global_var;
//     if(!isGlobalVar(init_name)){
//         init_global_var = symtable[fname][init_name].get();
//     }
//     else{
//         init_global_var = symtable["Global"][init_name].get();
//     }
//     koopa_raw_value_data* load_cur_global = new koopa_raw_value_data;
//     koopa_raw_value_data* save_cur_global = new koopa_raw_value_data; 
//     // 定义
//     std::string temp_varible_name = std::string("\%") + std::string(fname.c_str()) + "_" + "temp" + "_" + std::to_string(temp_cnt);
//     temp_cnt ++;
//     koopa_raw_type_kind_t* temp_varible_type = new koopa_raw_type_kind_t;
//     memcpy((void*)temp_varible_type, (const void*)init_global_var->sym_addr->ty->data.pointer.base, sizeof(koopa_raw_type_kind_t));
//     // 赋值临时变量
//     load_cur_global->ty = temp_varible_type;
//     char* temp_varible_name_cstr = new char[temp_varible_name.size() + 1];
//     strcpy(temp_varible_name_cstr, temp_varible_name.c_str());
//     load_cur_global->name = temp_varible_name_cstr;
//     load_cur_global->kind.tag = KOOPA_RVT_LOAD;
//     load_cur_global->kind.data.load.src = init_global_var->sym_addr;
//     (*current_insts).insert(it, load_cur_global);
//     std::unique_ptr<SymInfo> load_info = std::make_unique<SymInfo>(temp_varible_name_cstr, load_cur_global, VarStatus::TEMP);
//     symtable[fname][temp_varible_name_cstr] = std::move(load_info);
//     // 插入load指令
//     koopa_raw_type_kind_t* temp_save_type = new koopa_raw_type_kind_t;
//     temp_save_type->tag = KOOPA_RTT_UNIT;
//     save_cur_global->ty = temp_save_type;
//     save_cur_global->name = nullptr;
//     save_cur_global->kind.tag = KOOPA_RVT_STORE;
//     save_cur_global->kind.data.store.value = load_cur_global;
//     save_cur_global->kind.data.store.dest = global_var->sym_addr;
//     (*current_insts).insert(it, save_cur_global);
// }

// std::string SSAFuncBuilder:: GetSymInitName(std::string sys_name){
//     std::string str = sys_name;
//     size_t pos = str.rfind('_');
//     // 如果找到了下划线，就截取从开始到该下划线位置的子字符串
//     if (pos != std::string::npos) {
//         std::string temp_str = str.substr(0, pos);
//         std::string suffix = "_" + std::string(fname.c_str());
//         if (temp_str.size() >= suffix.size() && temp_str.compare(temp_str.size() - suffix.size(), suffix.size(), suffix) == 0) {
//             // 移除尾缀
//             str = temp_str.substr(0, temp_str.size() - suffix.size());
//         }
//     }
//     return str;
// }

// ////******************************************************************************************************************************************
// ////*******************建立支配关系、支配边界****************************************************************************************************
// ////******************************************************************************************************************************************
// ///获得严格支配关系（该节点支配其他节点），复制传播需要
// void DominatorTree:: addDom(BBInfo * bb , BBInfo * visit_bb , std::unordered_map<std::string,std::vector< BBInfo*>> & sdom){
//     sdom[bb->bb_name].push_back(visit_bb);
//     if(this->domTree.find(visit_bb)==this->domTree.end())return ;//找不到对应得 
//     if(this->domTree[visit_bb].size()==0)return;
//     for(BBInfo* next_bb : this->domTree[visit_bb]){
//         addDom(bb , next_bb , sdom);
//     }      
// }

// std::unordered_map<std::string,std::vector< BBInfo*>> DominatorTree:: getStrictDom() {
//     std::unordered_map<std::string,std::vector< BBInfo*>> sdom ;
//     if(this->domTree.size()==0){
//         this->computeDominators(this->blocks[0]);
//         this->buildDomTree();
//     }
    
//     for(BBInfo* bb : this->blocks){
//         sdom[bb->bb_name];  
//         if(this->domTree.find(bb)==this->domTree.end())continue;//找不到对应得 
//         if(this->domTree[bb].size()==0)continue;
//         for(BBInfo* next_bb : this->domTree[bb]){
//             addDom(bb , next_bb , sdom);
//         }        
//     }
//     return sdom;
// }

// void DominatorTree:: addDomed(BBInfo * bb , BBInfo * visit_bb , std::unordered_map<std::string,std::vector< BBInfo*>> & sdomed){
//     sdomed[bb->bb_name].push_back(visit_bb);
//     if(this->idom.find(visit_bb)==this->idom.end())return ;//找不到对应得 
//     addDomed(bb,this->idom[visit_bb] , sdomed);   
          
// }

// ///获得支配关系（该节点被其他节点支配,包括本身），复制传播需要
// std::unordered_map<std::string,std::vector< BBInfo*>> DominatorTree:: getStrictDomed() {
//     std::unordered_map<std::string,std::vector< BBInfo*>> sdomed ;
//     if(this->idom.size()==0){
//         this->computeDominators(this->blocks[0]);
//         this->buildDomTree();
//     }
//     for(BBInfo* bb : this->blocks){
//         sdomed[bb->bb_name].push_back(bb);  
//         if(this->idom.find(bb)==this->idom.end())continue;//找不到对应的被支配关系 说明这是entry节点
//         addDomed(bb, this->idom[bb] , sdomed) ;     
//     }
//     return sdomed;

// }
// void DominatorTree:: computeDominators(BBInfo* entry) {
//     std::unordered_map<BBInfo*, std::set<BBInfo*>> dom;
//     block_entry = entry;
//     // 支配节点，注意此处支配节点为支配BB*的节点，而不是BB*支配的节点
//     for (BBInfo* block : blocks) {
//         dom[block] = std::set<BBInfo*>(blocks.begin(), blocks.end());
//     }
//     dom[entry] = {entry};
//     bool changed = true;
//     while (changed) {
//         changed = false;
//         for (BBInfo* block : blocks) {
//             if (block == entry) continue;
//             std::set<BBInfo*> newDom = {block};
//             std::set<BBInfo*> tempInter(blocks.begin(), blocks.end());
//             if(block->prev.size() == 0){
//                 tempInter.clear();
//             }
//             for (BBInfo* pred : block->prev) {
//                 std::set<BBInfo*> tempDom = dom[pred];
//                 std::set<BBInfo*> intersect;
//                 std::set_intersection(tempInter.begin(), tempInter.end(),
//                                         tempDom.begin(), tempDom.end(),
//                                         std::inserter(intersect, intersect.begin()));
//                 tempInter = intersect;
//             }
//             std::set<BBInfo*> unionSet;
//             std::set_union(tempInter.begin(), tempInter.end(),
//                                 newDom.begin(), newDom.end(),
//                                 std::inserter(unionSet, unionSet.begin()));
//             newDom = unionSet;
//             if (dom[block] != newDom) {
//                 dom[block] = newDom;
//                 changed = true;
//             }
//         }
//     }
//     // 寻找支配关系
//     // for(BBInfo* block : blocks){
//     //     for(BBInfo* tempbb : dom[block]){
//     //         std::cout << tempbb->bb_name << " ";
//     //     }
//     //     std:: cout << "dominate: " << block->bb_name << std::endl;
//     // }
//     for (BBInfo* block : blocks) {
//         BBInfo* tempIdom = entry;
//         if(block == entry){
//             idom[block] = nullptr;
//             continue;
//         }// entry block的输入为空
//         // 所有节点都有entry作为其支配节点
//         for (BBInfo* domBlock : dom[block]) {
//             if (domBlock != block) {
//                 if(dom[domBlock].size() > dom[tempIdom].size()){
//                     tempIdom = domBlock;
//                 }
//                 // domBlock的支配节点比tempBlock的支配节点多时，代表domBlock是tempBlock支配的节点
//             }
//         }
//         idom[block] = tempIdom;
//     }
// }

// void DominatorTree:: buildDomTree() {
//     for (const auto& pair : idom) {
//         BBInfo* child = pair.first;
//         BBInfo* parent = pair.second;
//         if(parent){
//             domTree[parent].insert(child);
//         }
//         // entry的parent为空
//     }
// }

// void DominatorTree:: computeDominanceFrontier() {
//     for (BBInfo* block : blocks) {
//         if (block->prev.size() < 2) continue;
//         for (BBInfo* pred : block->prev) {
//             BBInfo* runner = pred;
//             while (runner != idom[block]) {
//                 domFrontier[runner].insert(block);
//                 runner = idom[runner];
//             }
//         }
//     }
// }//How Cooper Relize


// std::set<BBInfo*> DominatorTree:: getDominanceFrontier(BBInfo* bb){
//     return domFrontier[bb];
// }

// bool DominatorTree:: addBBParams(BBInfo* bb, std::set<SymInfo*> var){
//     bool changed = false;
//     if(bb->params != var){
//         bb->params = var;
//         changed = true;
//     }
//     return changed;
// }

// void DominatorTree:: buildBBParams(std::unordered_map<BBInfo*, std::set<SymInfo*>>& bb2sym){
//     bool changed = true;
//     std::unordered_map<BBInfo*, std::set<SymInfo*>> interSet;
//     std::set_union(bb2sym[block_entry].begin(), bb2sym[block_entry].end(),
//                     functable[fname]->use_global.begin(), functable[fname]->use_global.end(),
//                     std::inserter(bb2sym[block_entry], bb2sym[block_entry].begin()));
//     while(changed){
//         changed = false;
//         //
//         for(BBInfo* bb: blocks){
//             std::set<SymInfo*> sym_list = bb2sym[bb];
//             std::set<SymInfo*> unionSet;
//             std::set_union(sym_list.begin(), sym_list.end(),
//                             bb->params.begin(), bb->params.end(),
//                             std::inserter(unionSet, unionSet.begin()));
//             for(BBInfo* des_bb: getDominanceFrontier(bb)){
//                 if(interSet.find(des_bb) == interSet.end()){
//                     interSet[des_bb] = unionSet;
//                 }
//                 else{
//                     std::set<SymInfo*> tempUnionSet;
//                     std::set_union(interSet[des_bb].begin(), interSet[des_bb].end(),
//                                                 unionSet.begin(), unionSet.end(),
//                                                 std::inserter(tempUnionSet, tempUnionSet.begin()));
//                     interSet[des_bb] = tempUnionSet;
//                 }
//             }
//         }
//         for(auto pair : interSet){
//             if(addBBParams(pair.first, pair.second)){
//                 changed = true;
//             }
//         }
//         interSet.clear();
//     }
//     // 删除多余的Params, 避免后续算法报错
//     std::unordered_map<BBInfo*, bool> Visited;
//     Visited.clear();
//     for(auto bb : blocks){
//         Visited[bb] = false;
//     }
//     std::set<SymInfo*> CurrentSym;
//     CurrentSym.clear();
//     avoidInvalidParams(block_entry, bb2sym, Visited, CurrentSym);
// }

// void DominatorTree:: avoidInvalidParams(BBInfo* bb, std::unordered_map<BBInfo*, std::set<SymInfo*>>& bb2sym, std::unordered_map<BBInfo*, bool> & Visited, std::set<SymInfo*> CurrentSym){
//     Visited[bb] = true;
//     std::set<SymInfo*> sym_list = bb2sym[bb];
//     std::set<SymInfo*> unionSet;
//     std::set_union(sym_list.begin(), sym_list.end(),
//                     CurrentSym.begin(), CurrentSym.end(),
//                     std::inserter(unionSet, unionSet.begin()));
//     CurrentSym = unionSet;
//     // 明确当前数据流中哪些符号已被定义
//     for(BBInfo* next_bb : bb->next){
//         std::set<SymInfo*> interSet;
//         std::set_intersection(CurrentSym.begin(), CurrentSym.end(),
//                                 next_bb->params.begin(), next_bb->params.end(),
//                                 std::inserter(interSet, interSet.begin()));
//         next_bb->params = interSet;
//         if(Visited[next_bb] == false){
//             avoidInvalidParams(next_bb, bb2sym, Visited, CurrentSym);
//         }
//     }
//     return;
// }
