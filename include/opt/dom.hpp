// #pragma once
// #ifndef DOM_HPP
// #define DOM_HPP
// #include <iostream>
// #include <vector>
// #include <stack>
// #include <set>
// #include <algorithm>
// #include <unordered_map>
// #include <string>
// #include <list>
// #include <signal.h>
// #include "../ir/koopa.h"
// #include "../ir/koopaAST.hpp"
// #include <assert.h>

// class DominatorTree {
// private:
//     std::vector<BBInfo*> blocks; // BB树
//     std::string fname; // Func Name
//     BBInfo* block_entry; // BB 进入节点
//     std::unordered_map<BBInfo*, BBInfo*> idom; // 直接支配节点
//     std::unordered_map<BBInfo*, std::set<BBInfo*>> domTree; // 支配树
//     std::unordered_map<BBInfo*, std::set<BBInfo*>> domFrontier; // 支配边界
// public:
    

//     ///转ssa需要
//     DominatorTree(const std::vector<BBInfo*>& blocks, std::string fname) : blocks(blocks), fname(fname) {}
//     void computeDominators(BBInfo* entry);
//     void buildDomTree();
//     void computeDominanceFrontier();

//     ////// 打印支配信息
//     std::set<BBInfo*> getDominanceFrontier(BBInfo* bb);
//     bool addBBParams(BBInfo* bb, std::set<SymInfo*> var);
//     void buildBBParams(std::unordered_map<BBInfo*, std::set<SymInfo*>>& bb2sym);
//     void avoidInvalidParams(BBInfo* bb, std::unordered_map<BBInfo*, std::set<SymInfo*>>& bb2sym, std::unordered_map<BBInfo*, bool> & Visited, std::set<SymInfo*> CurrentSym);

//     ///获得严格支配关系，复制传播需要
//     void  addDom(BBInfo * bb , BBInfo * visit_bb , std::unordered_map<std::string,std::vector< BBInfo*>> & sdom);
//     std::unordered_map<std::string ,std::vector< BBInfo*>> getStrictDom() ;
 
 

//     ///获得严格支配关系（该节点被其他节点支配），复制传播需要
//     void  addDomed(BBInfo * bb , BBInfo * visit_bb , std::unordered_map<std::string,std::vector< BBInfo*>> & sdomed) ;
//     std::unordered_map<std::string ,std::vector< BBInfo*>> getStrictDomed() ;
// };

// class SSAFuncBuilder{
// private:
//     typedef struct info{
//         bool visited;
//         std::unordered_map<SymInfo*, int>* out;
//         info(): visited(false), out(nullptr){};
//     }info;
//     typedef struct exit_info{
//         BBInfo* bb;
//         koopa_raw_value_data* inst;
//     }exit_info;
//     std::string fname; // name of the function
//     std::vector<BBInfo*>* bbs;
//     size_t temp_cnt;
//     // 命名规则 fname_temp_(temp_cnt)
//     BBInfo* entry;
//     std::unordered_map<BBInfo*, std::set<SymInfo*>> bb2sym;
//     DominatorTree* dom;
//     std::unordered_map<BBInfo*, info*> bb2info;
//     std::stack<BBInfo*> bb_stack;
//     std::unordered_map<SymInfo*, int> sym_max_cnt;
//     std::list<koopa_raw_value_data*>* current_insts;
//     std::unordered_map<SymInfo*, std::vector<exit_info*>> exit_temp_table; // 用以记录局部变量的使用情况
//     std::set<std::string> exit_sym_visited; // 用以记录局部变量的使用情况
// public:

//     SSAFuncBuilder(){};
//     ~SSAFuncBuilder();
//     bool isGlobalVar(const std::string str);
//     void Build(std::vector<BBInfo*>& func_basic_blocks, std::string func_name);
//     std::set<SymInfo*> LocalVariable(BBInfo* bb);
//     void DFS(BBInfo* bb);
//     void Rename(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     SymInfo* addAllocInst(std::string name, koopa_raw_type_kind_t* & ty, std::list<koopa_raw_value_data*>::iterator & it);
//     void addBlockParams(BBInfo* bb);
//     void ExitBlockParams();

//     std::vector<BBInfo*> getSSABB();
//     void RenameAlloc(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameLoad(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameStore(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameGetPtr(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameGetElemPtr(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameBinary(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameBranch(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameJump(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameCall(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void RenameReturn(koopa_raw_value_data* inst, std::unordered_map<SymInfo*, int> & current_sym, std::list<koopa_raw_value_data*>::iterator & it);
//     void SaveGlobalVar(SymInfo* global_var , std::list<koopa_raw_value_data*>::iterator & it);
//     void LoadGlobalVar(SymInfo* global_var , std::list<koopa_raw_value_data*>::iterator & it);
//     std::string GetSymInitName(std::string sym_name);

//     //退出重命名
//     void Exit();
//     void ExitTemp(); // 删除多余的load和store语句
//     void ExitDFS(BBInfo* bb);
//     int GetRenameCnt(std::string name);
//     void ExitInst(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitAlloc(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitLoad(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitStore(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitAggregate(koopa_raw_value_data* inst, koopa_raw_value_t value); // store的值可能为聚合体
//     void ExitGetPtr(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitGetElemPtr(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitBinary(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitBranch(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitJump(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitCall(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     void ExitReturn(koopa_raw_value_data* inst, std::list<koopa_raw_value_data*>::iterator & it);
//     bool JudgeDelete(exit_info load, exit_info store);


// };

// class SSABuilder{
//     std::unordered_map<std::string, SSAFuncBuilder*> map;
// public:
//     void build(Program & program);
//     void BBCheck(FuncInfo* func);
//     void exit(Program & program);
// };
// #endif
