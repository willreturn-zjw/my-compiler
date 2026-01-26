#include "../../include/opt/dfa.hpp"
#include "../../include/ir/koopaAST.hpp"
#include "../../include/back/koopa_print.hpp"
#include <unordered_set>
#define LIVE 1
#define DEAD 0
#define TODO 0
#define PRINT_DEBUG 0


// 总变量数
int var_num;
// 创建新的 map，将符号地址与计数器建立键值对
std::unordered_map<koopa_raw_value_data *, int> var_index_map;
// 映射用于存储 ValueInfo
std::unordered_map<koopa_raw_value_data *, ValueInfo *> value_info_map;

ValueInfo::ValueInfo(koopa_raw_value_data *v, int var_num)
    : value(v)
{
    // Allocate and initialize in_vector and out_vector
    in_vector = new int8_t[var_num]();  // All elements initialized to 0
    out_vector = new int8_t[var_num](); // All elements initialized to 0
}
// 获取 in_vector/out_vector
int8_t *get_inst_vector(koopa_raw_value_data &inst, bool in)
{
    if (in)
        return value_info_map[&inst]->in_vector;
    else
        return value_info_map[&inst]->out_vector;
}

// 获取 in_vector/out_vector 重载，使得后端visit可以直接用koopa_raw_value_t调用
int8_t *get_inst_vector(koopa_raw_value_t &inst, bool in)
{
    koopa_raw_value_data_t *non_const_inst = const_cast<koopa_raw_value_data_t *>(inst);
    if (in)
        return value_info_map[non_const_inst]->in_vector;
    else
        return value_info_map[non_const_inst]->out_vector;
}

/*******************************Live_Var**********************************/
// live value的union  tranfer function:IN[B]=Use[b]U(OUT[B]-def[B])
void Union_for_LV(int8_t *res, int8_t *input, int n, int control_bit)
{
    for (int i = 0; i < n; i++)
    {
        res[i] = res[i] || input[i];
    }
}
// 符号表     函数名-变量名-value地址映射   以及重载std::cout<<运算
// 全局变量时 函数名为“global”  ，其他函数名以@开头，因此可以区分
extern std::unordered_map<std::string, std::unordered_map<std::string, std::unique_ptr<SymInfo>>> symtable;

// 初始化value_info_map
void init_value_info_map(Program *program)
{
    for (auto func : program->functable)
    {
        // std::cerr << "current func: " << func->func_name << std::endl;
        for (auto bb : func->bb_list)
        {

            int i = 0;
            for (auto inst : bb->insts)
            {
                koopa_raw_value_data *value = inst;
                if (value == nullptr)
                    continue;
                ValueInfo *tmp = new ValueInfo(value, var_num);
   
                value_info_map.insert({value, tmp});
                i++;
                if (i == bb->insts.size())
                    break;
            }
        }
    }
}

// // 为每条指令设置前驱后继
void set_all_prev_next(Program *program)
{
    for (int i = 0; i < program->functable.size(); i++)
    {
        FuncInfo *func = program->functable[i];
        // std::cerr << "func name: " << func->func_name << std::endl;
        for (int j = 0; j < func->bb_list.size(); j++)
        {
            // std::cerr << "bb name: " << func->bb_list[j]->bb_name << std::endl;
            BBInfo *bb = func->bb_list[j];
            if (bb->insts.size() == 0)
                continue;
            // 获取第一个迭代器、第二个迭代器和倒数第二个迭代器
            std::list<koopa_raw_value_data *>::iterator first = bb->insts.begin();
            std::list<koopa_raw_value_data *>::iterator second = std::next(first);
            std::list<koopa_raw_value_data *>::iterator last = bb->insts.end();        // list末尾的迭代器，不是最后一个元素的迭代器
            std::list<koopa_raw_value_data *>::iterator second_last = std::prev(last); // 最后一个元素的迭代器

            // 遍历从第二条到倒数第二条指令
            if (bb->insts.size() > 1)
            {
                for (std::list<koopa_raw_value_data *>::iterator it = second; it != second_last; ++it)
                {
                    koopa_raw_value_data *current = *it;
                    std::list<koopa_raw_value_data *>::iterator next_it = std::next(it);
                    koopa_raw_value_data *next = *next_it;
                    // 设置前驱后继
                    ValueInfo *tmp = value_info_map[current];
                    ValueInfo *tmp_next = value_info_map[next];
                    /// 设置当前指令的后继
                    tmp->succ.insert(tmp_next);
                    /// 设置下一条指令的前驱
                    tmp_next->pred.insert(tmp);
                }
            }
            // 上述循环执行完后，第一条指令的后继和最后一条指令的后继还未设置

            // 设置第一条指令的后继
            if (bb->insts.size() > 1)
            {
                value_info_map[*first]->succ.insert(value_info_map[*second]);
            }
            // 设置最后一条指令的后继
            int succs_size = bb->next.size();
            ValueInfo *last_inst = value_info_map[*second_last];
            for (int i = 0; i < succs_size; i++)
            {

                koopa_raw_value_data *tmp = bb->next[i]->insts.front();
  
                if (bb->next[i]->insts.size() == 0)
                {

                    continue;
                }
                last_inst->succ.insert(value_info_map[tmp]);
            }

            // 设置第一条指令的前驱
            int pre_size = bb->prev.size();
            ValueInfo *first_inst = value_info_map[*first];
            for (int i = 0; i < pre_size; i++)
            {
                koopa_raw_value_data *tmp = bb->prev[i]->insts.back();
                first_inst->pred.insert(value_info_map[tmp]);
            }
        }
    }
}

// // 统计变量数，为每个变量分配在vector中的位置
// // 考虑了alloc类型的变量
void set_var_index()
{
    int count = 0;
    // 遍历符号表
    for (const auto &funcname : symtable)
    {
        const auto &symbols = funcname.second;
        for (const auto &varname : symbols)
        {
            SymInfo *info = varname.second.get();
            if (var_index_map.find(info->sym_addr) != var_index_map.end())
            {
                std::cerr << "var_index_map has already had this key: " << info->sym_addr << std::endl;
                assert(false);
            }
            var_index_map.insert({info->sym_addr, count++});
        }
    }
    var_num = count;
}

// // 得到某个value在所有变量中的位置
int get_var_index(const koopa_raw_value_t &value)
{
    koopa_raw_value_data_t *non_const_value = const_cast<koopa_raw_value_data_t *>(value);
    // 查找键是否存在
    auto it = var_index_map.find(non_const_value);
    if (it != var_index_map.end())
    {
        // 如果键存在，返回对应的值
        return it->second;
    }
    else
    {
        // 如果键不存在，返回一个特定值，表示键不存在，比如 -1
        return -1; // 你可以根据需要选择其他值或错误处理机制
    }
}

// 判断变量是否在var_index_map中
inline void in_index_map(int index, koopa_raw_value_data *value)
{
    if (index == -1)
    {
        if (value->name)
            std::cerr << value->name << " is not in var_index_map" << std::endl;
        std::cerr << "index is -1" << std::endl;
        assert(false);
    }
}

inline void in_index_map(int index, koopa_raw_value_t value)
{
    if (index == -1)
    {
        if (value->name)
            std::cerr << value->name << " is not in var_index_map" << std::endl;
        std::cerr << "index is -1" << std::endl;
        assert(false);
    }
}

// 将变量放入def / use set
/// 将变量放入def set
void in_def_set(std::unordered_set<int> &def_set, koopa_raw_value_data *value)
{
    // 如果没有返回值，不加入def_set
    if (value->ty->tag == KOOPA_RTT_UNIT || value->kind.tag == KOOPA_RVT_RETURN)
        return;
    int index = get_var_index(value);
    in_index_map(index, value);
    def_set.insert(index);
}

void in_def_set(std::unordered_set<int> &def_set, const koopa_raw_value_t &value)
{
    // 如果没有返回值，不加入def_set
    if (value->ty->tag == KOOPA_RTT_UNIT || value->kind.tag == KOOPA_RVT_RETURN)
        return;
    int index = get_var_index(value);
    in_index_map(index, value);
    def_set.insert(index);
}

void traverse_aggregate(std::unordered_set<int> &use_set, const koopa_raw_value_t &value)
{
    if (value->kind.tag == KOOPA_RVT_AGGREGATE)
    {
        for (int i = 0; i < value->kind.data.aggregate.elems.len; i++)
        {
            traverse_aggregate(use_set, (koopa_raw_value_t)(value->kind.data.aggregate.elems.buffer[i]));
        }
    }
    else if (value->kind.tag != KOOPA_RVT_INTEGER && value->kind.tag != KOOPA_RVT_FLOATNUM && value->kind.tag != KOOPA_RVT_ZERO_INIT && value->kind.tag != KOOPA_RVT_UNDEF)
    {
        int index = get_var_index(value);
        in_index_map(index, value);
        use_set.insert(index);
    }
}

/// 将变量放入use set
void in_use_set(std::unordered_set<int> &use_set, koopa_raw_value_data *value)
{
    // 如果是浮点数、整数、0初始化、聚合类型，不加入use_set
    koopa_raw_value_tag_t tag = value->kind.tag;
    if (tag == KOOPA_RVT_AGGREGATE)
    {
        traverse_aggregate(use_set, value);
        return;
    }
    if (tag == KOOPA_RVT_FLOATNUM || tag == KOOPA_RVT_INTEGER ||
        tag == KOOPA_RVT_ZERO_INIT || tag == KOOPA_RVT_UNDEF)
    {
        return;
    }
    int index = get_var_index(value);
    in_index_map(index, value);
    use_set.insert(index);
}

void in_use_set(std::unordered_set<int> &use_set, const koopa_raw_value_t &value)
{
    // 如果是浮点数、整数、0初始化、聚合类型，不加入use_set
    koopa_raw_value_tag_t tag = value->kind.tag;
    if (tag == KOOPA_RVT_AGGREGATE)
    {
        traverse_aggregate(use_set, value);
        return;
    }
    if (tag == KOOPA_RVT_FLOATNUM || tag == KOOPA_RVT_INTEGER ||
        tag == KOOPA_RVT_ZERO_INIT || tag == KOOPA_RVT_UNDEF)
    {
        return;
    }
    int index = get_var_index(value);
    in_index_map(index, value);
    use_set.insert(index);
}

// 单条指令活变量分析
bool LV_for_one_inst(koopa_raw_value_data *inst, bool visited)
{
    ValueInfo *tmp = value_info_map[inst];
    koopa_raw_value_data *value = tmp->value;
    int8_t *inst_in = tmp->in_vector;
    int8_t *inst_out = tmp->out_vector;
    // 用于比较，以判断是否发生变化
    int8_t tmp_in[var_num];
    int8_t tmp_out[var_num];
    // set 的内容是变量在bit set中的位置
    std::unordered_set<int> &def_set = tmp->def_set;
    std::unordered_set<int> &use_set = tmp->use_set;

    // 复制in 和 Out
    memcpy(tmp_in, inst_in, var_num * sizeof(int8_t));
    memcpy(tmp_out, inst_out, var_num * sizeof(int8_t));

    // 只有第一次被遍历的时候才需要处理def 和 use 集合
    if (!visited)
    {
        koopa_raw_slice_t args;
        koopa_raw_value_t tmp;
        switch (value->kind.tag)
        {
        case KOOPA_RVT_ALLOC: // @a = alloc i32
            in_def_set(def_set, value);
            break;
        case KOOPA_RVT_LOAD: // %1 = load @a
            in_def_set(def_set, value);
            in_use_set(use_set, value->kind.data.load.src);
            break;
        case KOOPA_RVT_STORE: // store %1, @a, store 1, @a
            in_use_set(use_set, value->kind.data.store.value);
            in_use_set(use_set, value->kind.data.store.dest);
            break;
        case KOOPA_RVT_GET_PTR: // %1 = get_ptr @a, 1  %1 = get_ptr @a, %1
            in_def_set(def_set, value);
            in_use_set(use_set, value->kind.data.get_ptr.src);
            in_use_set(use_set, value->kind.data.get_ptr.index);
            break;
        case KOOPA_RVT_GET_ELEM_PTR: // %1 = get_elem_ptr @a, 1  %1 = get_elem_ptr @a, %1
            in_def_set(def_set, value);
            in_use_set(use_set, value->kind.data.get_elem_ptr.src);
            in_use_set(use_set, value->kind.data.get_elem_ptr.index);
            break;
        case KOOPA_RVT_BINARY: // %23= add %21, %22, %23 = add %21, 1, %23= add 1, %22
            in_def_set(def_set, value);
            in_use_set(use_set, value->kind.data.binary.lhs);
            in_use_set(use_set, value->kind.data.binary.rhs);
            break;
        case KOOPA_RVT_JUMP: // jump label
            // value->kind.data.jump;
            break;
        case KOOPA_RVT_BRANCH: // br %cond, %while_body, %while_end
            in_use_set(use_set, value->kind.data.branch.cond);
            break;
        case KOOPA_RVT_CALL: // %38= call @func_float(%36,%37) / call @putint (%0)
            in_def_set(def_set, value);
            // 遍历参数列表，判断是否将参数加入use_set
            args = value->kind.data.call.args;
            for (int i = 0; i < args.len; i++)
            {
                tmp = reinterpret_cast<koopa_raw_value_t>(value->kind.data.call.args.buffer[i]);
                in_use_set(use_set, tmp);
            }
            break;
        case KOOPA_RVT_RETURN: // ret %1 ret 0 ret
            if (value->kind.data.ret.value)
            {
                in_use_set(use_set, value->kind.data.ret.value);
            }
            break;
        default:
            std::cerr << value->kind.tag << " is not supported in LV_for_one_inst" << std::endl;
            assert(false);
            break;
        }
    }

    // 计算out
    // OUT[s]=U IN[n] n∈succ(s)
    for (auto succ : tmp->succ)
    {
        if (succ == nullptr)
        {
            std::cerr << "succ->in_vector is nullptr" << std::endl;
            std::cerr << "len of succ:" << tmp->succ.size() << std::endl;
            std::cerr << "kind.tag:" << tmp->value->kind.tag << std::endl;
            assert(false);
        }
        Union_for_LV(inst_out, succ->in_vector, var_num, 1);
    }

    // 计算in
    // IN[s]=Use[s]U(OUT[s]-Def[s])
    /// IN[s]=OUT[s]
    memcpy(inst_in, inst_out, var_num * sizeof(int8_t));
    /// IN[s] -= Def[s]
    for (auto def_var : def_set)
    {
        inst_in[def_var] = DEAD;
    }
    /// IN[s] = IN[s] U Use[s]
    for (auto use_var : use_set)
    {
        inst_in[use_var] = LIVE;
    }

    bool in_changed = memcmp(tmp_in, inst_in, var_num * sizeof(int8_t));
    bool out_changed = memcmp(tmp_out, inst_out, var_num * sizeof(int8_t));
    return in_changed || out_changed;
}




// 全局活变量分析
void Live_Var(Program *program)
{
    // 为每个变量分配在vector中的位置
    set_var_index();
    // 初始化value_info_map
    init_value_info_map(program);
    // 建立前驱后继关系，其实这个应该建立BB的时候就做
    set_all_prev_next(program);

    for (int i = 0; i < program->functable.size(); i++)
    {
        FuncInfo *func = program->functable[i];
        Live_for_func(func);
    }


}

void Live_for_func(FuncInfo *func)
{
    std::list<ValueInfo *> wl;
    for (int j = func->bb_list.size() - 1; j >= 0; j--)
    {
        BBInfo *bb = func->bb_list[j];
        for (auto it = bb->insts.rbegin(); it != bb->insts.rend(); ++it)
        {
            koopa_raw_value_data *current = *it;
            ValueInfo *tmp = value_info_map[current];
            wl.push_back(tmp);
        }
    }
    bool flag = false;        // 标记是否有变化
    bool visited_all = false; // 标记是否第一次遍历完所有指令，用于避免重复处理def 和 use 集合
    while (1)
    {
        flag = false;
        for (auto it = wl.begin(); it != wl.end(); ++it)
        {
            flag = LV_for_one_inst((*it)->value, visited_all) || flag;
        }
        visited_all = true;
        if (!flag)
            break;
    }
}


// 基本块的Tarjan算法
std::stack<BBInfo *> cc_stack;
int dfncnt = 0;
std::vector<scc> loops;
int sc = 0;

void tarjan(BBInfo *b)
{
    b->low = b->dfn = ++dfncnt;
    cc_stack.push(b);
    b->in_stack = true;
    for (auto next : b->next)
    {
        if (next->dfn == 0)
        {
            tarjan(next);
            b->low = std::min(b->low, next->low);
        }
        else if (next->in_stack)
        {
            b->low = std::min(b->low, next->dfn);
        }
    }
    if (b->low == b->dfn && cc_stack.empty() == 0)
    {
        loops.push_back(scc());
        BBInfo *tmp;
        scc &tmp_scc = loops.back();

        while (cc_stack.top() != b)
        {
            tmp = cc_stack.top();
            cc_stack.pop();
            tmp->in_stack = false;
            tmp->loop_id = sc;
            if (tmp->insts.size())
            {
                auto first = tmp->insts.front();
                auto last = tmp->insts.back();
                if (first == nullptr || last == nullptr)
                    continue;
                tmp_scc.l = std::min(tmp_scc.l, value_info_map[first]->index);
                tmp_scc.h = std::max(tmp_scc.h, value_info_map[last]->index);
            }
            tmp_scc.size++;
        }
        // cc_stack.top() == b
        tmp = cc_stack.top();
        cc_stack.pop();
        tmp->in_stack = false;
        tmp->loop_id = sc;
        if (tmp->insts.size())
        {
            auto first = tmp->insts.front();
            auto last = tmp->insts.back();
            if (first != nullptr && last != nullptr)
            {
                tmp_scc.l = std::min(tmp_scc.l, value_info_map[first]->index);
                tmp_scc.h = std::max(tmp_scc.h, value_info_map[last]->index);
            }
        }
        tmp_scc.size++;
        ++sc;
    }
}

/*线性扫描寄存器分配*/
std::vector<BBInfo *> block_sequence;
// 基本块的拓扑排序
void topo_sort(BBInfo *BB)
{
    if (BB->visited)
        return;
    BB->visited = true;
    for (auto next : BB->next)
    {
        topo_sort(next);
    }
    block_sequence.push_back(BB);
    return;
}

// 针对函数 建立线性序列
void linearize(FuncInfo *func)
{
    for (auto bb : func->bb_list)
    {
        topo_sort(bb);
    }
}

// 先对单个函数进行标号，不行再换到全局
// 函数的定义那一行也要算作一个指令，因为参数在此处被定义

// 使用拓扑排序得到的基本块的线性序列对指令进行标号
void number()
{
    int index = 1; // 从1开始，0是函数定义，用作参数和全局变量定义的那一行
    ValueInfo *tmp;
    // 使用拓扑排序得到的基本块的线性序列对指令进行标号
    // 因为topo_sort得到的基本块的线性序列是从后往前的，所以这里要从后往前遍历block_sequence
    for (int i = block_sequence.size() - 1; i >= 0; i--)
    {
        auto bb = block_sequence[i];
        for (auto inst : bb->insts)
        {
            tmp = value_info_map[inst];
            tmp->index = index++;
        }
    }
    // 清空block_sequence，以便下一个函数使用
    block_sequence.clear();
}

// 使用IR的行号来对指令进行标号
void number(FuncInfo *func)
{
    int index = 1; // 从1开始，0是函数定义，用作参数和全局变量定义的那一行
    ValueInfo *tmp;
    for (auto bb : func->bb_list)
    {
        for (auto inst : bb->insts)
        {
            tmp = value_info_map[inst];
            tmp->index = index++;
        }
    }
}

// // 更新函数内每个变量的活跃区间
void update_intervals(SymInfo *sym, ValueInfo *tmp, BBInfo *bb)
{
    if (tmp->use_set.find(get_var_index(sym->sym_addr)) != tmp->use_set.end())
    {
        int index = tmp->index;
        // 异常处理
        if (index == -1)
        {
            std::cerr << "index of " << tmp->value << " is -1" << std::endl;
            if (tmp->value->name)
                std::cerr << "value_name is " << tmp->value->name << std::endl;
            assert(false);
        }
        if (bb == sym->lr.def_BB)
        {
            sym->lr.def_range.h = std::max(sym->lr.def_range.h, index);
        }
        else
        {
            if (sym->lr.cur_bb == nullptr || sym->lr.cur_bb != bb)
            {
                sym->lr.cur_bb = bb;
                auto first_inst = value_info_map[bb->insts.front()];
                sym->lr.use_ranges.push_back({first_inst->index, index});
            }
            if (bb->in_loop)
            {
                // 尝试1：直接把最大环的区间赋给变量，出错：原因是free_intervals排序没写对，修改后没有出错
                sym->lr.use_ranges.back().l = loops[bb->loop_id].l;
                sym->lr.use_ranges.back().h = loops[bb->loop_id].h;
            }
            else
            {
                sym->lr.use_ranges.back().h = std::max(sym->lr.use_ranges.back().h, index);
            }
        }
    }
}

void LiveRange::update_global_range()
{
    for (auto lr : use_ranges)
    {
        global_range.l = std::min(global_range.l, lr.l);
        global_range.h = std::max(global_range.h, lr.h);
    }
    global_range.l = std::min(global_range.l, def_range.l);
    global_range.h = std::max(global_range.h, def_range.h);
    if (global_range.l == global_range.h || global_range.l == INT32_MAX || global_range.h == -1)
    {
        is_dead = true;
    }
    if (PRINT_DEBUG)
    {
        std::cerr << "  @global_range of " << sym->sym_name << " is [" << global_range.l << ", " << global_range.h << "]" << std::endl;
    }
}

// // 计算函数内每个变量的活跃区间
void calculate_intervals(FuncInfo *func)
{
    if (PRINT_DEBUG)
    {
        std::cerr << "@calculate_intervals of " << func->func_name << std::endl;
    }
    ValueInfo *tmp;
    SymInfo *sym;
    auto &func_symtable = symtable[func->func_name]; // func_symtable中既有局部变量，也有参数
    auto &global_symtable = symtable["Global"];

    // 判断基本块是否在循环中
    for (auto bb : func->bb_list)
    {
        if (bb->loop_id == -1)
        {
            std::cerr << "bb:" << bb->bb_name << " is not in scc" << std::endl;
            assert(false);
        }
        if (loops[bb->loop_id].size > 1)
        {
            bb->in_loop = true;
        }
    }

    // 初始化参数的def_range
    for (auto param : func->params)
    {
        param->lr.sym = param;
        param->lr.def_range.l = 0; // 参数在函数定义处定义
        param->lr.def_range.h = 0;
        param->lr.def_BB = func->bb_list[0];
    }

    // re-initialize 全局变量的活跃区间
    for (auto &pair : global_symtable)
    {
        auto global_sym = pair.second.get();
        global_sym->lr.sym = global_sym;
        global_sym->lr.def_range.l = 0;
        global_sym->lr.def_range.h = 0;
        global_sym->lr.def_BB = func->bb_list[0];
        global_sym->lr.global_range.l = INT32_MAX;
        global_sym->lr.global_range.h = -1;
        global_sym->lr.use_ranges.clear();
    }

    // 初始化局部变量的def_range
    for (auto bb : func->bb_list)
    {
        for (auto inst : bb->insts)
        {
            tmp = value_info_map[inst];
            if (tmp->value->ty->tag != KOOPA_RTT_UNIT && tmp->value->kind.tag != KOOPA_RVT_RETURN)
            {
                auto tmp_sym = symtable[func->func_name][tmp->value->name].get();
                tmp_sym->lr.sym = tmp_sym;
                tmp_sym->lr.def_BB = bb;
                tmp_sym->lr.def_range.l = tmp->index;
                tmp_sym->lr.def_range.h = tmp->index;
            }
        }
    }

    // 计算活跃区间
    for (auto bb : func->bb_list)
    {
        for (auto inst : bb->insts)
        {
            tmp = value_info_map[inst];
            for (auto &pair : func_symtable)
            {
                sym = pair.second.get();
                update_intervals(sym, tmp, bb);
            }
            for (auto &pair : global_symtable)
            {
                sym = pair.second.get();
                update_intervals(sym, tmp, bb);
            }

        }
    }

    // 计算global range
    /// 全局变量
    for (auto &pair : global_symtable)
    {
        auto global_sym = pair.second.get();
        global_sym->lr.update_global_range();
    }

    /// 局部变量
    for (auto &pair : func_symtable)
    {
        auto local_sym = pair.second.get();
        local_sym->lr.update_global_range();
    }

    if (PRINT_DEBUG)
    {
        std::cerr << std::endl;
    }
}

// // 单个函数的活跃区间算法
void interval_analysis(FuncInfo *func)
{
    // linearize(func);
    if (func->bb_list.size() == 0)
    {
        return;
    }
    if (!cc_stack.empty())
    {
        std::cerr << "cc_stack is not empty before tarjan for a function" << std::endl;
        assert(false);
    }
    dfncnt = 0;
    loops.clear();
    sc = 0;
    // 先采用直接使用IR里面的行号来对指令进行标号
    number(func);
    for (auto bb : func->bb_list)
    {
        if (bb->dfn == 0)
        {
            tarjan(bb);
        }
    }
    calculate_intervals(func);
}