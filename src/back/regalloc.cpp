#include "../../include/back/regalloc.hpp"
#include <vector>
#include <algorithm> // For std::sort

#define TO_DO 10
#define PRINT_DEBUG 0

// TODO8: 把传参寄存器也用来分配
// 当前函数名
extern std::string cur_func;
// 可用寄存器
std::stack<std::string> free_w_regs;
std::stack<std::string> free_s_regs;
bool w_regs_alloced[w_regs_num];
bool s_regs_alloced[s_regs_num];
std::vector<LiveRange *> win_w_lrs[w_regs_num];
std::vector<LiveRange *> win_s_lrs[s_regs_num];

// 未分配的pair
/// INT32
std::vector<LiveRange *> free_intervals_i;
/// FLOAT32
std::vector<LiveRange *> free_intervals_f;

// 创建一个最大堆的优先级队列，使用自定义比较器,存储已分配的pair
/// INT32
std::priority_queue<LiveRange *, std::vector<LiveRange *>> occupied_intervals_i;
/// FLOAT32
std::priority_queue<LiveRange *, std::vector<LiveRange *>> occupied_intervals_f;

// 分配情况表
std::unordered_map<koopa_raw_value_t, std::string> reg_alloc_map;

// 初始化可用寄存器
void free_regs_init()
{
    free_w_regs = std::stack<std::string>();
    free_s_regs = std::stack<std::string>();
    for (int i = w_regs_reserved + w_regs_for_arg + zero_w_reg_num; i < w_regs_num; i++)
    {
        free_w_regs.push("x" + std::to_string(i));
    }
    for (int i = s_regs_reserved + s_regs_for_arg + zero_s_reg_num; i < s_regs_num; i++)
    {
        free_s_regs.push("s" + std::to_string(i));
    }
}

// 初始化分配表
void reg_alloc_init(FuncInfo *func)
{
    reg_alloc_map.clear();
    free_intervals_i.clear();
    free_intervals_f.clear();
    occupied_intervals_i = std::priority_queue<LiveRange *, std::vector<LiveRange *>>();
    occupied_intervals_f = std::priority_queue<LiveRange *, std::vector<LiveRange *>>();
    // 用寄存器传参的参数的分配
    TO_DO;
    if(PRINT_DEBUG){
        for (int i = w_regs_for_arg + w_regs_reserved + zero_w_reg_num; i < w_regs_num; i++){
            win_w_lrs[i].clear();
        }
        for(int i = s_regs_for_arg + s_regs_reserved + zero_s_reg_num; i < s_regs_num; i++){
            win_s_lrs[i].clear();
        }
    }
}

// 区分变量应该使用的寄存器类型
int get_value_type(koopa_raw_value_t &value)
{
    if (value->ty->tag == KOOPA_RTT_FLOAT32)
        return USE_FLOAT_REG;
    return USE_INT_REG;
}

int get_value_type(koopa_raw_value_data *value)
{
    if (value->ty->tag == KOOPA_RTT_FLOAT32)
        return USE_FLOAT_REG;
    return USE_INT_REG;
}

// 初始化变量对应活跃区间
/// 现在的实现方式是直接把用寄存器传参的参数给忽略掉，也就是说不会给他们分配 除掉传参寄存器和临时寄存器之外的寄存器，但是通过栈帧传递的参数依旧参与分配
void init_free_intervals(FuncInfo *func)
{
    // 遍历符号表
    auto &func_symtable = symtable[func->func_name]; // func_symtable中既有局部变量，也有参数
    auto &global_symtable = symtable["Global"];
    /// 局部变量
    for (auto &pair : func_symtable)
    {
        auto sym = pair.second.get();
        if (sym->lr.is_dead == false)
        {
            if (sym->sym_addr->kind.tag == KOOPA_RVT_FUNC_ARG_REF)
                continue; // 参数单独拿出来处理
            if (get_value_type(sym->sym_addr) == USE_INT_REG)
            {
                free_intervals_i.push_back(&(sym->lr));
            }
            else
            {
                free_intervals_f.push_back(&(sym->lr));
            }
        }
    }
    /// 全局变量
    for (auto &pair : global_symtable)
    {
        auto sym = pair.second.get();
        if (sym->lr.is_dead == false)
        {
            if (get_value_type(sym->sym_addr) == USE_INT_REG)
            {
                free_intervals_i.push_back(&(sym->lr));
            }
            else
            {
                free_intervals_f.push_back(&(sym->lr));
            }
        }
    }
    /// 参数
    int i_cnt = 0;
    int f_cnt = 0;
    for (auto &param : func->params)
    {
        if (get_value_type(param->sym_addr) == USE_INT_REG)
        {
            if (i_cnt < w_regs_for_arg)
            {
                // TODO8: 传参寄存器也用来分配
                // reg_alloc_map.insert({param->sym_addr, "r" + std::to_string(i_cnt)});
                i_cnt++;
            }
            else
            {
                free_intervals_i.push_back(&(param->lr));
            }
        }
        else
        {
            if (f_cnt < s_regs_for_arg)
            {
                // TODO8: 传参寄存器也用来分配
                // reg_alloc_map.insert({param->sym_addr, "s" + std::to_string(f_cnt)});
                f_cnt++;
            }
            else
            {
                free_intervals_f.push_back(&(param->lr));
            }
        }
    }

    // 对未分配区间进行排序
    std::sort(free_intervals_i.begin(), free_intervals_i.end(),
              [](const LiveRange *lhs, const LiveRange *rhs)
              { return *lhs < *rhs; });
    std::sort(free_intervals_f.begin(), free_intervals_f.end(),
              [](const LiveRange *lhs, const LiveRange *rhs)
              { return *lhs < *rhs; });
}

// 对单个类型的变量进行线性扫描
void linear_scan_for_one(std::vector<LiveRange *> &free_intervals, std::priority_queue<LiveRange *, std::vector<LiveRange *>> &occupied_intervals, std::stack<std::string> &free_regs)
{
    for (auto lr : free_intervals)
    {
        eviction_for_one(occupied_intervals, lr->global_range.l);
        if (free_regs.empty())
        {
            try_spill(lr, occupied_intervals);
        }
        else
        {
            std::string reg = free_regs.top();
            if (reg.empty())
            {
                std::cerr << "reg is empty in linear scan for one" << std::endl;
                assert(false);
            }
            free_regs.pop();
            if (reg[0] == 'x')
            {
                if (w_regs_alloced[std::stoi(reg.substr(1))])
                {
                    std::cerr << "x" << std::stoi(reg.substr(1)) << " has been alloced" << std::endl;
                    assert(false);
                }
                else
                    w_regs_alloced[std::stoi(reg.substr(1))] = true;
            }
            else if (reg[0] == 's')
            {
                if (s_regs_alloced[std::stoi(reg.substr(1))])
                {
                    std::cerr << "s" << std::stoi(reg.substr(1)) << " has been alloced" << std::endl;
                    assert(false);
                }
                else
                    s_regs_alloced[std::stoi(reg.substr(1))] = true;
            }
            else
            {
                std::cerr << "reg is not w or s in linear scan for one" << std::endl;
                assert(false);
            }
            reg_alloc_map.insert({lr->sym->sym_addr, reg});
            occupied_intervals.push(lr);
        }
    }
}

// 已经被放进寄存器的参数要从未分配区间中删除
void linear_scan(FuncInfo *func)
{
    memset(w_regs_alloced, 0, sizeof(w_regs_alloced));
    memset(s_regs_alloced, 0, sizeof(s_regs_alloced));
    if (func->bb_list.size() == 0)
        return;
    linear_scan_for_one(free_intervals_i, occupied_intervals_i, free_w_regs);
    linear_scan_for_one(free_intervals_f, occupied_intervals_f, free_s_regs);
}

// 对单个函数进行寄存器分配
void reg_alloc(FuncInfo *func)
{
    // 初始化可用寄存器
    free_regs_init();

    // 初始化分配表
    reg_alloc_init(func);

    // 计算活跃区间
    interval_analysis(func);

    // 初始化未分配区间，初始化后的未分配区间是有序的
    init_free_intervals(func);

    if (PRINT_DEBUG)
    {
        for (auto lr : free_intervals_i)
        {
            std::cerr << "live range of " << lr->sym->sym_name << " : [" << lr->global_range.l << ", " << lr->global_range.h << ']' << std::endl;
        }
    }
    // 线性扫描
    linear_scan(func);

    arrange_win_seq();
    if (PRINT_DEBUG)
    {
        print_win_seq(func);
    }
}

void eviction_for_one(std::priority_queue<LiveRange *, std::vector<LiveRange *>> &occupied_intervals, int l)
{
    std::stack<LiveRange *> tmp_stack;
    std::string reg;
    LiveRange *tmp;
    while (occupied_intervals.empty() == 0)
    {
        tmp = occupied_intervals.top();
        occupied_intervals.pop();
        if (tmp->global_range.h > l)
        {
            tmp_stack.push(tmp);
        }
        else // h_x <= l
        {
            auto it = reg_alloc_map.find(tmp->sym->sym_addr);
            /*异常处理*/
            if (it == reg_alloc_map.end())
            {
                std::cerr << "interval_reg_map does not find interval: " << tmp->global_range.l << ',' << tmp->global_range.h << std::endl;
                assert(false);
            }
            /*异常处理*/

            reg = (*it).second;

            /*异常处理*/
            if (reg == "")
            {
                std::cerr << "reg is empty in eviction for one " << tmp->global_range.l << ',' << tmp->global_range.h << std::endl;
                assert(false);
            }
            /*异常处理*/

            if (reg[0] == 'x')
            {
                if (w_regs_alloced[std::stoi(reg.substr(1))])
                {
                    w_regs_alloced[std::stoi(reg.substr(1))] = false;
                }
                else
                {
                    std::cerr << reg << " has not been alloced" << std::endl;
                    assert(false);
                }
                free_w_regs.push(reg);
            }
            else
            {
                if (s_regs_alloced[std::stoi(reg.substr(1))])
                {
                    s_regs_alloced[std::stoi(reg.substr(1))] = false;
                }
                else
                {
                    std::cerr << reg << " has not been alloced" << std::endl;
                    assert(false);
                }
                free_s_regs.push(reg);
            }
            update_win(reg, tmp);
        }
    }
    while (tmp_stack.empty() == 0)
    {
        tmp = tmp_stack.top();
        tmp_stack.pop();
        occupied_intervals.push(tmp);
    }
}

void eviction(int l)
{
    eviction_for_one(occupied_intervals_i, l);
    eviction_for_one(occupied_intervals_f, l);
}

void spill(LiveRange *lr)
{
    if (reg_alloc_map.find(lr->sym->sym_addr) != reg_alloc_map.end())
    {
        reg_alloc_map.erase(lr->sym->sym_addr);
    }
    // 分配栈帧也可以在这里做
}

void try_spill(LiveRange *lr, std::priority_queue<LiveRange *, std::vector<LiveRange *>> &occupied_intervals)
{
    LiveRange *top = occupied_intervals.top();
    std::string reg;
    if (top->global_range.h > lr->global_range.h)
    {
        auto it = reg_alloc_map.find(top->sym->sym_addr);
        /*异常处理*/
        if (it == reg_alloc_map.end())
        {
            std::cerr << "value: " << top->sym->sym_name << " in occupied_intervals does not find in reg_alloc_map" << std::endl;
            assert(false);
        }
        /*异常处理*/
        reg = (*it).second;
        /*异常处理*/
        if (reg.empty())
        {
            std::cerr << "reg is empty in tyr_spill" << std::endl;
            assert(false);
        }
        /*异常处理*/
        spill(top);
        occupied_intervals.pop();
        occupied_intervals.push(lr);
        reg_alloc_map.insert({lr->sym->sym_addr, reg});
    }
    else
    {
        spill(lr);
    }
}

void update_win(std::string &reg, LiveRange *lr)
{
    if (reg.empty())
    {
        std::cerr << "reg is empty in update_win" << std::endl;
        assert(false);
    }
    if (reg[0] == 'x')
    {
        win_w_lrs[std::stoi(reg.substr(1))].push_back(lr);
    }
    else if (reg[0] == 's')
    {
        win_s_lrs[std::stoi(reg.substr(1))].push_back(lr);
    }
    else
    {
        std::cerr << "wrong reg name!" << std::endl;
        assert(false);
    }
}

void arrange_win_seq()
{
    while (occupied_intervals_f.empty() == 0)
    {
        auto lr = occupied_intervals_f.top();
        occupied_intervals_f.pop();
        auto it = reg_alloc_map.find(lr->sym->sym_addr);
        if (it == reg_alloc_map.end())
        {
            std::cerr << "value: " << lr->sym->sym_name << " in occupied_intervals does not find in reg_alloc_map" << std::endl;
            assert(false);
        }
        std::string reg = (*it).second;
        update_win(reg, lr);
    }
    while (occupied_intervals_i.empty() == 0)
    {
        auto lr = occupied_intervals_i.top();
        occupied_intervals_i.pop();
        auto it = reg_alloc_map.find(lr->sym->sym_addr);
        if (it == reg_alloc_map.end())
        {
            std::cerr << "value: " << lr->sym->sym_name << " in occupied_intervals does not find in reg_alloc_map" << std::endl;
            assert(false);
        }
        std::string reg = (*it).second;
        update_win(reg, lr);
    }
    for (int i = w_regs_for_arg + w_regs_reserved + zero_w_reg_num; i < w_regs_num; i++)
    {
        std::sort(win_w_lrs[i].begin(), win_w_lrs[i].end(),
                  [](const LiveRange *lhs, const LiveRange *rhs)
                  { return *lhs < *rhs; });
    }
    for (int i = s_regs_for_arg + s_regs_reserved + zero_s_reg_num; i < s_regs_num; i++)
    {
        std::sort(win_s_lrs[i].begin(), win_s_lrs[i].end(),
                  [](const LiveRange *lhs, const LiveRange *rhs)
                  { return *lhs < *rhs; });
    }
}

void print_win_seq(FuncInfo *func)
{
    std::cerr << "func: " << func->func_name << std::endl;
    for (int i = w_regs_for_arg + w_regs_reserved + zero_w_reg_num; i < w_regs_num; i++)
    {
        if (win_w_lrs[i].size() == 0)
            continue;
        std::cerr << "  x" << i << ": " << std::endl;
        for (auto lr : win_w_lrs[i])
        {
            std::cerr << "    " << lr->sym->sym_name << ": [" << lr->global_range.l << ", " << lr->global_range.h << ']' << std::endl;
        }
        std::cerr << std::endl;
    }
    for (int i = s_regs_for_arg + s_regs_reserved + zero_s_reg_num; i < s_regs_num; i++)
    {
        if (win_s_lrs[i].size() == 0)
            continue;
        std::cerr << "  s" << i << ": " << std::endl;
        for (auto lr : win_s_lrs[i])
        {
            std::cerr << "    " << lr->sym->sym_name << ": [" << lr->global_range.l << ", " << lr->global_range.h << ']' << std::endl;
        }
        std::cerr << std::endl;
    }
}
