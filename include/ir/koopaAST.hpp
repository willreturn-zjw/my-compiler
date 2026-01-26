#pragma once
#ifndef KOOPAAST_HPP
#define KOOPAAST_HPP

#include <iostream>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <tuple>
#include <cstring>
#include <unordered_set>

#include "datadef.hpp"
#include "koopa.h"

enum class AstKind
{
    IntType,           /// 32-bit integer type.
    FloatType,         /// 32-bit float type.
    ArrayType,         /// Array type.
    PointerType,       /// Pointer type.
    FunType,           /// Function type.
    SymbolRef,         /// Symbol refernce.
    IntVal,            /// Integer literal.
    FloatVal,          /// float literal.
    UndefVal,          /// Undefined value.
    Aggregate,         /// Aggregate value.
    ZeroInit,          /// Zero initializer.
    SymbolDef,         /// Symbol definition.
    GlobalDef,         /// Global symbol definition.
    MemDecl,           /// Memory declaration.
    GlobalDecl,        /// Global memory declaration.
    Load,              /// Load.
    Store,             /// Store.
    GetPointer,        /// Pointer calculation.
    GetElementPointer, /// Element pointer calculation.
    BinaryExpr,        /// Binary expression.
    Branch,            /// Branch.
    Jump,              /// Jump.
    FunCall,           /// Function call.
    Return,            /// Return.
    FunDef,            /// Function definition.
    Block,             /// Basic block.
    FunDecl,           /// Function declaration.
    End,               /// End of file.
    Error              /// Error.
};
// 重载 operator<< 以便 std::cout 可以输出 AstKind 类型的值
std::ostream &operator<<(std::ostream &os, AstKind kind);

extern std::string cur_func;
extern std::string cur_bb;

class BBInfo;
class SymInfo;
class Program;

enum class VarStatus
{
    LOCAL,     // 局部变量
    GlOBAL,    // 全局变量
    TEMP,      // 临时变量
    FUNC_ARG,  // 函数参数
    BLOCK_ARG, // 基本块参数
    UNKNOWN    // 未知
};

enum class SymValTag
{
    INT32,
    FLOAT32,
    SYMREF,
    UNKNOWN
};

// 活跃区间类
struct interval
{
    int l = INT32_MAX;
    int h = -1;
};

struct LiveRange
{
    SymInfo *sym = nullptr;
    BBInfo *def_BB = nullptr;
    interval def_range;
    std::vector<interval> use_ranges;
    BBInfo *cur_bb = nullptr;
    interval global_range;
    void update_global_range();
    bool operator<(const LiveRange &rhs) const
    {
        if (global_range.l == rhs.global_range.l)
        {
            return global_range.h < rhs.global_range.h; //
        }
        return global_range.l < rhs.global_range.l;
    }
    bool is_dead = false;
};

class SymInfo
{
public:
    std::string sym_name;           // 符号名
    koopa_raw_value_data *sym_addr; // 符号地址
    std::vector<koopa_raw_value_data *> used_by;

    std::vector<BBInfo *> set_pos; // 在哪些基本快被 定值    貌似没啥用，先不管了
    /// @brief 转ssa用 区分全局局部临时变量等
    VarStatus v_status = VarStatus::UNKNOWN; // 变量状态

    /// @brief 优化用
    int cal_f = -1;                         // -1表示未定义  0表示重复赋值， 1表示可计算，值是常数 , 2表示是一个数组   3表示是一个函数调用返回值
    SymValTag val_tag = SymValTag::UNKNOWN; // 值的类型
    union
    {
        int integer;     // cal_f = 1    SymValTag::INT32
        float float_val; // cal_f = 1    SymValTag::FLOAT32
        SymInfo *symRef; // 变量传播   SymValTag::SYMREF
        /// SymValTag::UNKNOWN该值就是底层，不能再传播了
    } def_val;

    bool live = true; // 是否有效

    /// @brief 寄存器分配需要
    LiveRange lr; // 活跃区间

    SymInfo() : sym_name(""), sym_addr(nullptr), used_by(), set_pos(), cal_f() {}

    SymInfo(const std::string &name, koopa_raw_value_data *addr) : sym_name(name), sym_addr(addr), used_by(), set_pos(), cal_f() {}
    SymInfo(const std::string &name, koopa_raw_value_data *addr, VarStatus status) : sym_name(name), sym_addr(addr), used_by(), set_pos(), v_status(status), cal_f() {}

    bool isGlobal();

    static bool Compare(const SymInfo *lhs, const SymInfo *rhs)
    {
        return lhs->sym_name < rhs->sym_name;
    }

    void clear_value();
    koopa_raw_value_data *mk_const_value();
};

class BBInfo
{
public:
    std::string bb_name;
    koopa_raw_basic_block_data_t *bb_addr;       // 当前bb的地址
    std::vector<koopa_raw_value_data *> used_by; // 被哪些jump,br指令跳转
    std::vector<BBInfo *> prev;                  // 前驱
    std::vector<BBInfo *> next;                  // 后继
    // 指令链表
    std::list<koopa_raw_value_data *> insts; // 当前bb的指令列表

    // 转ssa需要
    std::set<SymInfo *> params;      // 当前基本块参数
    std::vector<SymInfo *> sym_list; // 在当前基本块定值的变量

    /// 消除无用分支需要
    bool live = true; // 是否有效

    // 判断BB是否在循环中
    bool in_loop = false;
    int dfn = 0;           // 深度优先搜索的序号
    int low = INT32_MAX;   // 最小的dfn
    bool in_stack = false; // 是否在栈中
    int loop_id = -1;      // 所在循环的id

    // 寄存器分配需要
    bool visited = false; // 是否被访问过(用于建立基本块的线性序列)

    BBInfo() : bb_name(""), bb_addr(nullptr), used_by(), prev(), next(), insts(), params(), sym_list() {}

    BBInfo(const std::string &name, koopa_raw_basic_block_data_t *addr) : bb_name(name), bb_addr(addr), used_by(), prev(), next(), insts(), params(), sym_list() {}
    // 删除无用分支的时候用，获取当前bb的前驱传来的基本块参数 （仅当前驱唯一时可以调用）
    std::vector<SymInfo *> get_bb_arg_val();

    // 首先调用该函数时，当前bb的最后一条指令必是branch
    // void del_br_NextBB(bool cond_val);

    // 删除当前bb或当前bb和其前驱prev_bb的关系，以及后续的各种关联
    void del_bb(Program *program, BBInfo *root_BB);
};

class FuncInfo
{
public:
    std::string func_name;                       // 函数名
    koopa_raw_function_data_t *func_addr;        // koopa.h中函数数据
    std::vector<SymInfo *> params;               // 函数参数
    std::vector<koopa_raw_value_data *> used_by; // 被哪些指令调用
    std::vector<BBInfo *> bb_list;               // 当前函数的基本块列表

    std::set<SymInfo *> use_global; // 哪些全局变量被use和def  (不包括数组)

    FuncInfo() : func_name(""), func_addr(nullptr), used_by(), bb_list() {}

    FuncInfo(const std::string &name, koopa_raw_function_data_t *addr) : func_name(name), func_addr(addr), used_by(), bb_list() {}
};

class Program
{
public:
    std::vector<SymInfo *> global_syms;
    std::vector<FuncInfo *> functable;
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<BBInfo *>>> sdom;   // bb支配哪些基本块
    std::unordered_map<std::string, std::unordered_map<std::string, std::vector<BBInfo *>>> sdomed; // bb被哪些基本块支配

    std::unordered_map<FuncInfo *, std::unordered_set<FuncInfo *>> callGraph; // 函数调用图

    Program() : global_syms(), functable(), sdom() {}

    koopa_raw_program_t *buildon(); // 建立内存koopa.h
};

extern std::unique_ptr<Program> program;

// 函数名与函数地址的映射关系  以及重载std::cout<<运算
extern std::unordered_map<std::string, std::unique_ptr<FuncInfo>> functable;
std::ostream &operator<<(std::ostream &os, const std::unordered_map<std::string, std::unique_ptr<FuncInfo>> &unmap);

// 函数-bbname与bbname对应的bb地址映射关系    以及重载std::cout<<运算
extern std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<BBInfo>>> bbtable;
std::ostream &operator<<(std::ostream &os, const std::unordered_map<std::string, std::unique_ptr<BBInfo>> &unmap);
std::ostream &operator<<(std::ostream &os, const std::unordered_map<std::string,
                                                                    std::unordered_map<std::string, std::unique_ptr<BBInfo>>> &unmap);

// 符号表     函数名-变量名-value地址映射   以及重载std::cout<<运算
// 全局变量时 函数名为“global”  ，其他函数名以@开头，因此可以区分
extern std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<SymInfo>>> symtable;
std::ostream &operator<<(std::ostream &os, const std::unordered_map<std::string, std::unique_ptr<SymInfo>> &unmap);
std::ostream &operator<<(std::ostream &os, const std::unordered_map<std::string,
                                                                    std::unordered_map<std::string, std::unique_ptr<SymInfo>>> &unmap);

// 通过cur_func全局变量和sym找对应的符号
SymInfo *find_sym(std::string sym);

// GlobalDef(parse_global_def) , FunDef(parse_fun_def) , FunDecl(parse_fun_decl)

class AstBase
{
public:
    AstKind kind;
    AstBase(AstKind kind);

    virtual ~AstBase() = default;

    // virtual void build_on() = 0;
};

class AstBuildHelper
{
public:
    // 对parser_type返回的ast进行buildon
    static koopa_raw_type_kind_t *type_buildon(std::unique_ptr<AstBase> &ast);

    // 对parser_value返回的ast进行buildon
    static koopa_raw_value_data_t *value_buildon(std::unique_ptr<AstBase> &ast, koopa_raw_value_data_t *inst);

    // 对parser_init返回的ast进行buildon
    static koopa_raw_value_data_t *init_buildon(std::unique_ptr<AstBase> &ast, koopa_raw_value_data_t *inst);

    // 为FunDef或Block类型的生成函数参数, 0是为函数参数的    1是为基本块参数的
    static koopa_raw_value_data_t *params_buildon(std::tuple<std::string,
                                                             std::unique_ptr<AstBase>> &params,
                                                  int kind = 0, int order = 0);
};

//*************************************************************************************************
// *****************************function, block , global_symbol****************************************************************
/// Function definition.
class FunDef : public AstBase
{
public:
    std::string name;
    std::vector<std::tuple<std::string, std::unique_ptr<AstBase>>> params;
    std::unique_ptr<AstBase> ret;
    std::vector<std::unique_ptr<AstBase>> bbs;
    FunDef(AstKind kind, const std::string &n,
           std::vector<std::tuple<std::string, std::unique_ptr<AstBase>>> &&p,
           std::unique_ptr<AstBase> &&r, std::vector<std::unique_ptr<AstBase>> &&bb);
    koopa_raw_function_data_t *build_on();
};

/// Basic block.
class Block : public AstBase
{
public:
    std::string name;
    std::vector<std::tuple<std::string, std::unique_ptr<AstBase>>> params;
    std::vector<std::unique_ptr<AstBase>> stmts;
    Block(AstKind kind, const std::string &n, std::vector<std::tuple<std::string, std::unique_ptr<AstBase>>> &&p,
          std::vector<std::unique_ptr<AstBase>> &&s);
    void build_on(koopa_raw_basic_block_data_t *bb_data);
};

/// Global symbol definition.
class GlobalDef : public AstBase
{
public:
    std::string name;
    std::unique_ptr<AstBase> value;

    GlobalDef(AstKind kind, const std::string &n, std::unique_ptr<AstBase> &&v);
    koopa_raw_value_data_t *build_on();
};

/// Global memory declaration.
class GlobalDecl : public AstBase
{
public:
    std::unique_ptr<AstBase> ty;
    std::unique_ptr<AstBase> init;

    GlobalDecl(AstKind kind, std::unique_ptr<AstBase> &&t, std::unique_ptr<AstBase> &&init);
};

/// Function declaration.
class FunDecl : public AstBase
{
public:
    std::string name;
    std::vector<std::unique_ptr<AstBase>> params;
    std::unique_ptr<AstBase> ret;
    FunDecl(AstKind kind, const std::string &n, std::vector<std::unique_ptr<AstBase>> &&p, std::unique_ptr<AstBase> &&r);

    koopa_raw_function_data_t *build_on();

};

//*************************************************************************************************
// *****************************type****************************************************************
class IntType : public AstBase
{
public:
    IntType(AstKind kind);
};

class FloatType : public AstBase
{
public:
    FloatType(AstKind kind);
};

/// Array type.
class ArrayType : public AstBase
{
public:
    std::unique_ptr<AstBase> base;
    int len;
    ArrayType(AstKind kind, std::unique_ptr<AstBase> &&b, int l);
};

/// Pointer type.
class PointerType : public AstBase
{
public:
    std::unique_ptr<AstBase> base;
    PointerType(AstKind kind, std::unique_ptr<AstBase> &&b);
};

/// Function type.
class FunType : public AstBase
{
public:
    std::vector<std::unique_ptr<AstBase>> params;
    std::unique_ptr<AstBase> ret;

    FunType(AstKind kind, std::vector<std::unique_ptr<AstBase>> &&params, std::unique_ptr<AstBase> &&ret);
};

//*************************************************************************************************
//****************************init or value****************************************************************
// Symbol reference.
class SymbolRef : public AstBase
{
public:
    std::string symbol;
    SymbolRef(AstKind kind, const std::string &sym);
};

/// Integer literal.
class IntVal : public AstBase
{
public:
    int value;
    IntVal(AstKind kind, int v) ;
};

/// float literal.
class FloatVal : public AstBase
{
public:
    float value;
    FloatVal(AstKind kind, float v);
};

/// Undefined value.
class UndefVal : public AstBase
{
public:
    UndefVal(AstKind kind);
};

/// Aggregate value.
class Aggregate : public AstBase
{
public:
    std::vector<std::unique_ptr<AstBase>> elems;
    Aggregate(AstKind kind, std::vector<std::unique_ptr<AstBase>> &&elems);

    koopa_raw_type_kind *build_on_type();
};

/// Zero initializer.
class ZeroInit : public AstBase
{
public:
    ZeroInit(AstKind kind);
};

//*************************************************************************************************
//****************************instructions****************************************************************

/// Symbol definition.
class SymbolDef : public AstBase
{
public:
    std::string name;
    std::unique_ptr<AstBase> value;

    SymbolDef(AstKind kind, const std::string &n, std::unique_ptr<AstBase> &&v);
    koopa_raw_value_data_t *build_on();
};

/// Memory declaration.
class MemDecl : public AstBase
{
public:
    std::unique_ptr<AstBase> ty;

    MemDecl(AstKind kind, std::unique_ptr<AstBase> &&t);
};

/// Load.
class Load : public AstBase
{
public:
    std::string symbol;
    Load(AstKind kind, const std::string &n);
};

/// Pointer calculation.
class GetPointer : public AstBase
{
public:
    std::string symbol;
    std::unique_ptr<AstBase> value;

    GetPointer(AstKind kind, const std::string &s, std::unique_ptr<AstBase> &&v);

    void build_on(koopa_raw_value_kind_t &kind, koopa_raw_value_data_t *inst);
};

/// Element pointer calculation.
class GetElementPointer : public AstBase
{
public:
    std::string symbol;
    std::unique_ptr<AstBase> value;

    GetElementPointer(AstKind kind, const std::string &s, std::unique_ptr<AstBase> &&v);
    void build_on(koopa_raw_value_kind_t &kind, koopa_raw_value_data_t *inst);
};

/// Binary expression.
class BinaryExpr : public AstBase
{
public:
    BinaryOp op;
    std::unique_ptr<AstBase> lhs;
    std::unique_ptr<AstBase> rhs;
    BinaryExpr(AstKind kind, BinaryOp op, std::unique_ptr<AstBase> &&l, std::unique_ptr<AstBase> &&r);
    void build_on(koopa_raw_value_kind_t &kind, koopa_raw_value_data_t *inst);
};

/// Store.
class Store : public AstBase
{
public:
    std::unique_ptr<AstBase> value;
    std::string symbol;

    Store(AstKind kind, std::unique_ptr<AstBase> &&v, const std::string &s);

    koopa_raw_value_data_t *build_on();
};

/// Branch.
class Branch : public AstBase
{
public:
    std::unique_ptr<AstBase> cond;
    std::string tbb;
    std::vector<std::unique_ptr<AstBase>> targs;
    std::string fbb;
    std::vector<std::unique_ptr<AstBase>> fargs;
    Branch(AstKind kind, std::unique_ptr<AstBase> &&c,
           const std::string &tb, std::vector<std::unique_ptr<AstBase>> &&ta,
           const std::string &fb, std::vector<std::unique_ptr<AstBase>> &&fa);

    koopa_raw_value_data_t *build_on();
};

/// Jump.
class Jump : public AstBase
{
public:
    std::string target;
    std::vector<std::unique_ptr<AstBase>> args;
    Jump(AstKind kind, const std::string &target, std::vector<std::unique_ptr<AstBase>> &&a);

    koopa_raw_value_data_t *build_on();
};

/// Function call.
class FunCall : public AstBase
{
public:
    std::string fun;
    std::vector<std::unique_ptr<AstBase>> args;
    FunCall(AstKind kind, const std::string &fun, std::vector<std::unique_ptr<AstBase>> &&a);

    void build_on(koopa_raw_value_kind_t &kind, koopa_raw_value_data_t *inst);
};

/// Return.
class Return : public AstBase
{
public:
    std::unique_ptr<AstBase> value;
    Return(AstKind kind, std::unique_ptr<AstBase> &&v);
    koopa_raw_value_data_t *build_on();
};

/// End of file.
class End : public AstBase
{
public:
    End(AstKind kind);
};

/// Error.
class Error : public AstBase
{
public:
    Error(AstKind kind);
};

#endif // KOOPAAST_HPP
