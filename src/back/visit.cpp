
#include <cmath> // 包含 std::abs
#include "../../include/ir/koopa.h"
#include "../../include/back/IRtoAsm.hpp"
#include "../../include/back/koopa_print.hpp"
#include "../../include/back/regalloc.hpp"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define PRINT_DEBUG 0
#define PRINT_REG_ALLOC 0
#define REG_ALLOC 1
#define INT_TYPE 1
#define FLOAT_TYPE 2
#define SIZE_OF_STACK_UNIT 8
#define SYSY_VALUE_SIZE 4


std::string next_bb;
std::map<koopa_raw_binary_t *, std::string> reg_map;
std::map<koopa_raw_value_t, int> stack_map; // 栈值以及偏移量
std::map<std::string, int> reg_count;
std::stack<std::string> reg_stack;
std::stack<std::string> sreg_stack;

int size_of_stack_frame = 0;
std::string func_name = "";
std::string sp_reg = "sp";
// 零寄存器
const std::string zero_reg = "x19";
const std::string zero_reg_f = "s16";
// s12会被库函数修改
// 返回值寄存器
const std::string ret_reg = "x0";
const std::string ret_reg_f = "s0";
const int in_r0_r7 = 9;
const int in_s0_s7 = 54;
const int PARAM_INT_TYPE = 1;
const int PARAM_FLOAT_TYPE = 2;
const int FRAME_SIZE_OF_R_REGS=512;  
const int FRAME_SIZE_OF_S_REGS=512;
bool call_father;     // 指示当前父函数是不是调用了子函数
bool call_son = true; // 指示当前子函数是不是被调用了
bool zero_reg_init = false;
uint16_t fimm_low, fimm_high;
uint32_t fimm_32;
int cout_len = 6; // cout setw()中的参数
int total_ldr_times = 0, total_str_times = 0, total_mov_times = 0, total_add_times = 0;
bool dest_reg_used = false;
// 记录遍历到了基本块的第几条指令，方便调试
int inst_cnt = 0;

// 记录寄存器的使用情况
bool w_reg_used[w_regs_num];
bool s_reg_used[s_regs_num];
int w_reg_stack_base_offset;
int s_reg_stack_base_offset;
koopa_raw_value_t w_reg_value_map[w_regs_num];
koopa_raw_value_t s_reg_value_map[s_regs_num];

koopa_raw_value_t last_inst; // 用来做窥孔优化等
koopa_raw_value_t next_inst; // 用来做binary_for_br
bool binary_for_br = false;  // 如果上一条binary仅仅是为了下一条br计算cond则为真
koopa_raw_binary_op_t binary_for_br_op;
bool binary_for_br_next = false; // 用来指示br上一条是不是符号条件的bianry

// binary操作符对应的汇编指令 还要扩充float版本，不过可以在打印时分情况再输出
const char *ir_asm_binaryop[] = {
    [KOOPA_RBO_NOT_EQ] = "snez",
    [KOOPA_RBO_EQ] = "seqz",
    [KOOPA_RBO_GT] = "sgt",
    [KOOPA_RBO_LT] = "slt",
    [KOOPA_RBO_GE] = "sle",
    [KOOPA_RBO_LE] = "sge",
    [KOOPA_RBO_ADD] = "add",
    [KOOPA_RBO_SUB] = "sub",
    [KOOPA_RBO_MUL] = "mul",
    [KOOPA_RBO_DIV] = "sdiv",
    [KOOPA_RBO_MOD] = "mod",
    [KOOPA_RBO_AND] = "and",
    [KOOPA_RBO_OR] = "orr",
    [KOOPA_RBO_XOR] = "eor",
    [KOOPA_RBO_SHL] = "lsl",
    [KOOPA_RBO_SHR] = "lsr",
    [KOOPA_RBO_SAR] = "asr"};

//TODO 一股脑保存了所有寄存器，后续优化时可能要保存已使用的寄存器

// 访问 raw program
void Visit_top_program(const koopa_raw_program_t &top_program)
{
    // 初始化临时寄存器栈
    for (int i = w_regs_reserved + w_regs_for_arg - 1; i >= w_regs_for_arg; --i)
    {
        std::string str = "x" + std::to_string(i);
        reg_stack.push(str);
    }
    for (int i = s_regs_reserved + s_regs_for_arg - 1; i >= s_regs_for_arg; --i)
    {
        std::string str = "s" + std::to_string(i);
        sreg_stack.push(str);
    }

    // 输出汇编文件头部
    std::cout << std::left;
    std::cout << ".cpu cortex-a53" << std::endl;
    std::cout << ".data" << std::endl;
    // 访问所有全局变量（ARM把全局变量声明放在后面）
    Visit_raw_slice(top_program.values);

    std::cout << std::left;
    std::cout << ".text" << std::endl;

    // 访问所有函数
    std::cout << std::left;
    Visit_raw_slice(top_program.funcs);

}

// 访问 raw slice
void Visit_raw_slice(const koopa_raw_slice_t &slice)
{
    for (int i = 0; i < slice.len; ++i)
    {
        auto ptr = slice.buffer[i];
        // 根据 slice 的 kind 决定将 ptr 视作何种元素
        switch (slice.kind)
        {
        case KOOPA_RSIK_FUNCTION:
            // 访问函数
            Visit_func(reinterpret_cast<koopa_raw_function_t>(ptr));
            break;
        case KOOPA_RSIK_BASIC_BLOCK:
            // 访问基本块
            if (i + 1 < slice.len)
            {
                next_bb = reinterpret_cast<koopa_raw_basic_block_t>(slice.buffer[i + 1])->name;
            }
            Visit_bb(reinterpret_cast<koopa_raw_basic_block_t>(ptr));
            break;
        case KOOPA_RSIK_VALUE:
            // 访问指令
            {
                auto inst = reinterpret_cast<koopa_raw_value_t>(ptr);
                auto ptr_last = slice.buffer[MAX(0, i - 1)];
                auto ptr_next = slice.buffer[MIN(slice.len - 1, i + 1)];
                last_inst = reinterpret_cast<koopa_raw_value_t>(ptr_last);
                next_inst = reinterpret_cast<koopa_raw_value_t>(ptr_next);
                Visit_inst(inst);
                break;
            }
        case KOOPA_RSIK_TYPE:
            std::cout << "type" << std::endl;
            assert(false);
            break;
        case KOOPA_RSIK_UNKNOWN:
            std::cout << "unknown" << std::endl;
            assert(false);
            break;
        default:
            assert(false);
        }
    }
}

std::string ty2str(const koopa_raw_type_t &ty)
{
    switch (ty->tag)
    {
    case KOOPA_RTT_UNIT:
        return "KOOPA_RTT_UNIT";
    case KOOPA_RTT_INT32:
        return "KOOPA_RTT_INT32";
    case KOOPA_RTT_FLOAT32:
        return "KOOPA_RTT_FLOAT32";
    case KOOPA_RTT_POINTER:
        return "KOOPA_RTT_POINTER";
    case KOOPA_RTT_ARRAY:
        return "KOOPA_RTT_ARRAY";
    case KOOPA_RTT_FUNCTION:
        return "KOOPA_RTT_FUNCTION";
    default:
        return "unknown";
    }
}

// 访问函数  分配栈空间
void Visit_func(const koopa_raw_function_t &func)
{
    // print_symtable_for_func(func);
    if (0 == func->bbs.len)
    {
        return;
    }

    // 初始化寄存器使用情况
    for (int i = 0; i < w_regs_num; ++i)
    {
        w_reg_value_map[i] = nullptr;
    }
    for (int i = 0; i < s_regs_num; ++i)
    {
        s_reg_value_map[i] = nullptr;
    }

    // 进行寄存器分配
    auto func_info = functable[func->name].get();
    if (REG_ALLOC)
    {
        reg_alloc(func_info);
    }

    stack_map.clear();
    func_name = func->name + 1;
    std::cout << std::setw(6) << ".align 3" << std::endl; 
    std::cout << std::setw(6) << ".global" << "   " << func_name << std::endl;
    std::cout << std::setw(6) << ".type" << "   " << func_name << ", %function" << std::endl;
    std::cout << func_name << ":" << std::endl;

    // 初始化零寄存器
    if (func_name == "main" && zero_reg_init == false)
    {
        mov_print(zero_reg, 0, "", true, "");
        fmov_print(zero_reg_f, 0.0f, "", true, "");
        call_son = false;
        zero_reg_init = true; // 要是没有两个main函数的情况，就不需要考虑这个
    }

    /*float arguments*/
    int max_int_arg_count = 0;
    int max_float_arg_count = 0;
    int int_arg_count = 0;
    int float_arg_count = 0;
    /*float arguments*/

    size_of_stack_frame = 0;

    call_father = false;
    // 统计一个函数中需要在栈帧中分配内存的指令数
    for (size_t i = 0; i < func->bbs.len; i++)
    {
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)(func->bbs.buffer[i]);
        for (size_t j = 0; j < bb->insts.len; ++j)
        {
            auto ptr = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            if (ptr->ty->tag != KOOPA_RTT_UNIT && ptr->kind.tag != KOOPA_RVT_RETURN) 
            {
                size_of_stack_frame += 8; // TODO: 做完寄存器分配后，栈帧的大小可以优化
            }
            if (ptr->kind.tag == KOOPA_RVT_CALL)
            {
                call_father = true;
                koopa_raw_call_t call = ptr->kind.data.call;
                int_arg_count = 0;
                float_arg_count = 0;
                for (int i = 0; i < call.args.len; i++)
                {
                    int param_type = get_param_type(static_cast<koopa_raw_value_t>(call.args.buffer[i])->ty);
                    if (param_type == 2)
                        float_arg_count += 1;
                    else if (param_type == 1)
                        int_arg_count += 1;
                    else
                    {
                        std::cout << "参数类型错误" << std::endl;
                    }
                }
                if (int_arg_count > max_int_arg_count)
                {
                    max_int_arg_count = int_arg_count;
                }
                if (float_arg_count > max_float_arg_count)
                {
                    max_float_arg_count = float_arg_count;
                }
                /*float arguments*/
            }
        }
    }
    /*float arguments*/
    if (call_father)
    {
        if (max_int_arg_count > w_regs_for_arg)
        {
            size_of_stack_frame += 8 * (max_int_arg_count - w_regs_for_arg);
        }
        if (max_float_arg_count > s_regs_for_arg)
        {
            size_of_stack_frame += 8 * (max_float_arg_count - s_regs_for_arg);
        }
        // size_of_stack_frame += 4;//这是为ra寄存器分配的栈空间
    }
    // max_arg_count -> max_int_arg_count + max_float_arg_count
    int sp_offset = 8 * MAX(max_int_arg_count - w_regs_for_arg, 0) + 8 * MAX(max_float_arg_count - s_regs_for_arg, 0) - 8; // 减4是因为下面+=4
    // sp_offset是给参数分配了栈空间过后，最后一个参数的偏移量，也就是第一个局部变量/临时变量相对于当前函数sp的偏移量 减4

    int array_length = 0;
    /*float arguments*/

    // 将栈帧分配给临时变量
    for (size_t i = 0; i < func->bbs.len; i++)
    {
        koopa_raw_basic_block_t bb = (koopa_raw_basic_block_t)(func->bbs.buffer[i]);
        for (size_t j = 0; j < bb->insts.len; ++j)
        {
            auto ptr = reinterpret_cast<koopa_raw_value_t>(bb->insts.buffer[j]);
            if (ptr->ty->tag != KOOPA_RTT_UNIT && ptr->kind.tag != KOOPA_RVT_RETURN) // 函数返回值不是空类型
            {
                stack_map.insert({(koopa_raw_value_t)ptr, sp_offset += 8});
                if (ptr->kind.tag == KOOPA_RVT_ALLOC)
                {
                    if (ptr->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY)
                    {
                        array_length = cal_array_length(*ptr->ty->data.pointer.base);
                        sp_offset += SYSY_VALUE_SIZE * array_length;
                        size_of_stack_frame += SYSY_VALUE_SIZE * array_length;
                    }
                    else // TODO: ALLOC需不需要分配空间
                    {
                        size_of_stack_frame += 8 - 8;
                        sp_offset += 8 - 8;
                    }
                }
            }
        }
    }
    // 没有为多出来的函数参数分配位置
    // 实际上预留了空间，到call指令是再装载值即可
    // 分别统计int/float参数个数
    size_of_stack_frame += FRAME_SIZE_OF_R_REGS + FRAME_SIZE_OF_S_REGS;

    // 四字节对齐
    size_of_stack_frame = -((size_of_stack_frame + 15) / 16) * 16;

    // 下述放入stack_map的都是放到寄存器里面的参数
    int param_offset = 0;
    int now_int_num = -1, now_float_num = -1;
    for (int i = 0; i < func->params.len; i++)
    {
        koopa_raw_value_t value = (koopa_raw_value_t)(func->params.buffer[i]);
        int param_type = get_param_type(value->ty);
        if (param_type != PARAM_FLOAT_TYPE)
            now_int_num += 1;
        else
            now_float_num += 1;
        if (param_type != PARAM_FLOAT_TYPE && (now_int_num < w_regs_for_arg))
            stack_map.insert({value, now_int_num - in_r0_r7});
        else if ((param_type == PARAM_FLOAT_TYPE) && (now_float_num < s_regs_for_arg))
            stack_map.insert({value, now_float_num - in_s0_s7}); // 第几个参数就放在第几个寄存器里
        if ((now_int_num >= w_regs_for_arg &&
             param_type == PARAM_INT_TYPE) ||
            (now_float_num >= s_regs_for_arg &&
             param_type == PARAM_FLOAT_TYPE))
        {
            // std::cout<<"param_offset:"<<param_offset<<std::endl;
            stack_map.insert({value, param_offset - size_of_stack_frame});
            param_offset += 8;
        }
    }
    // print_stack_map(stack_map);

    // 为函数分配栈空间
    add_print(sp_reg, sp_reg, size_of_stack_frame, "", true,"");
    // add_print(sp_reg,sp_reg,0,"",true);

    // 保存lr  假设lr在栈底第一个元素
    if (call_father)
        str_print("x30", sp_reg, -size_of_stack_frame - 8, "", 1);

    /// 参数
    for (int i = 0; i < func->params.len; ++i)
    {
        koopa_raw_value_t param = (koopa_raw_value_t)(func->params.buffer[i]);
        // 通过寄存器传递的参数不需要再次加载
        if (stack_map[param] < 0)
            continue;
        if (reg_alloc_map.find(param) != reg_alloc_map.end())
        {
            std::string reg = reg_alloc_map[param];
            update_reg_value_map(param, reg);
            ldr_print(reg, sp_reg, stack_map[param], "", 1);
        }
    }

    /// 全局变量
    auto &global_symtable = symtable["Global"];
    SymInfo *sym;
    for (auto &pair : global_symtable)
    {
        sym = pair.second.get();
        if (reg_alloc_map.find(sym->sym_addr) != reg_alloc_map.end())
        {
            std::string reg = reg_alloc_map[sym->sym_addr];
            update_reg_value_map(sym->sym_addr, reg);
            gloabal_var_load(reg, sym->sym_addr->name + 1);
        }
    }

    // 遍历指令
    Visit_raw_slice(func->bbs);
    std::cout << std::endl;
    // 恢复被分配的寄存器放在处理ret指令的函数里面
}

// 访问基本块  floatdone
void Visit_bb(const koopa_raw_basic_block_t &bb)
{
    // 访问所有指令
    std::cout << func_name << '_' << bb->name + 1 << ":" << std::endl;
    if (PRINT_DEBUG)
    {
        inst_cnt = 0;
    }
    Visit_raw_slice(bb->insts);
    std::cout << std::endl;
}

// 访问指令  floatdone
void Visit_inst(const koopa_raw_value_t &value)
{
    // 根据指令类型判断后续需要如何访问
    const auto &kind = value->kind;
    if (binary_for_br_next)
        binary_for_br = true;
    else
        binary_for_br = false;
    binary_for_br_next = false;
    switch (kind.tag)
    {
    case KOOPA_RVT_RETURN:
        // 访问 return 指令
        Visit_ret(kind.data.ret, value);
        break;
    case KOOPA_RVT_FLOATNUM:
        // 访问 integer 指令  注：有问题，没有出现过这条指令
        std::cerr << "Visit_const_float during Visit_inst" << std::endl;
        assert(false);
        Visit_const_float(kind.data.floatnum);
        break;
    case KOOPA_RVT_INTEGER:
        // 访问 integer 指令  注：有问题，没有出现过这条指令
        std::cerr << "Visit_const_int during Visit_inst" << std::endl;
        assert(false);
        Visit_const_int(kind.data.integer);
        break;
    case KOOPA_RVT_BINARY:
        // 访问 binary 指令
        {
            if (next_inst->kind.tag == KOOPA_RVT_BRANCH && strcmp(next_inst->kind.data.branch.cond->name, value->name) == 0)
            {
                koopa_raw_binary_op_t tmp = value->kind.data.binary.op;
                // 注，目前只优化了大小比较，等于不等于；其它运算类型未优化
                if (value->used_by.len == 1 &&
                    get_value_type(kind.data.binary.lhs->ty) != FLOAT_TYPE &&
                    get_value_type(kind.data.binary.rhs->ty) != FLOAT_TYPE &&
                    (tmp == KOOPA_RBO_EQ || tmp == KOOPA_RBO_NOT_EQ || tmp == KOOPA_RBO_GT || tmp == KOOPA_RBO_LT || tmp == KOOPA_RBO_GE || tmp == KOOPA_RBO_LE))
                {
                    auto ptr = value->used_by.buffer[0];
                    auto inst = reinterpret_cast<koopa_raw_value_t>(ptr);
                    if (inst->kind.tag == KOOPA_RVT_BRANCH && strcmp(inst->kind.data.branch.cond->name, next_inst->kind.data.branch.cond->name) == 0)
                    {
                        binary_for_br = true;
                        binary_for_br_next = true;
                        binary_for_br_op = kind.data.binary.op;
                    }
                }
            }
            Visit_binary(kind.data.binary, value, value->used_by.len);
            break;
        }
    case KOOPA_RVT_ALLOC:
        // 访问 alloc 指令  i32和f32都是4字节，但是寄存器用的不一样所以要区分开来
        Visit_alloc(value);
        break;
    case KOOPA_RVT_LOAD:
        // 访问 load 指令
        Visit_load(kind.data.load, value);
        break;
    case KOOPA_RVT_STORE:
        // 访问 store 指令
        Visit_store(kind.data.store);
        break;
    case KOOPA_RVT_BRANCH:
        // 访问 branch 指令
        Visit_branch(kind.data.branch);
        break;
    case KOOPA_RVT_JUMP:
        // 访问 jump 指令
        Visit_jump(kind.data.jump);
        break;
    case KOOPA_RVT_CALL:
        // 访问 call 指令
        Visit_call(kind.data.call, value);
        break;
    case KOOPA_RVT_GLOBAL_ALLOC:
        // 访问 global_alloc 指令
        Visit_global_alloc(kind.data.global_alloc, value);
        break;
    case KOOPA_RVT_GET_ELEM_PTR:
        // 访问 get_elem_ptr 指令
        Visit_elem_ptr(kind.data.get_elem_ptr, value);
        break;
    case KOOPA_RVT_GET_PTR:
        // 访问 get_ptr 指令
        Visit_ptr(kind.data.get_ptr, value);
        break;
    default:
        printf("this is %d", kind.tag);
        std::cerr << kind.tag << std::endl;
        assert(false);
    }

    // 在输出指令后，也就是对应变量正式被def，更新寄存器的使用情况
    if (value->kind.tag != KOOPA_RVT_RETURN && value->ty->tag != KOOPA_RTT_UNIT)
    {
        if (reg_alloc_map.find(value) != reg_alloc_map.end())
        {
            std::string reg = reg_alloc_map[value];
            update_reg_value_map(value, reg);
        }
    }
    // 清空临时寄存器栈
    while (!reg_stack.empty())
    {
        reg_stack.pop();
    }
    while (!sreg_stack.empty())
    {
        sreg_stack.pop();
    }
    // 初始化临时寄存器栈
    for (int i = w_regs_reserved + w_regs_for_arg - 1; i >= w_regs_for_arg; --i)
    {
        std::string str = "x" + std::to_string(i);
        reg_stack.push(str);
    }
    for (int i = s_regs_reserved + s_regs_for_arg - 1; i >= s_regs_for_arg; --i)
    {
        std::string str = "s" + std::to_string(i);
        sreg_stack.push(str);
    }

    std::cout << std::endl;
}

// 访问返回指令  ret floatdone  已重构
void Visit_ret(const koopa_raw_return_t &ret, const koopa_raw_value_t &value)
{
    if (ret.value)
    {
        int true_type = get_value_type(value->ty);
        std::string r0 = "x0";
        if (true_type == FLOAT_TYPE)
            r0 = "s0";
        bool src_is_param = (ret.value->kind.tag == KOOPA_RVT_FUNC_ARG_REF);
        dest_reg_used = false;
        auto it = reg_alloc_map.find(ret.value);
        std::string src_reg;

        // 返回值存在寄存器中，因为是ret语句，所以即使被分配了寄存器，也可以直接修改寄存器的值
        if (it != reg_alloc_map.end())
        {
            src_reg = (*it).second;
            /// mov操作
            mov_between_any_regs(r0, src_reg);
        }
        // 返回值是整型立即数
        else if (ret.value->kind.tag == KOOPA_RVT_INTEGER)
        {
            if (r0[0] == 's')
            {
                fmov_print(r0, ret.value->kind.data.integer.value, "", true, "");
            }
            else
            {
                mov_print(r0, ret.value->kind.data.integer.value, "", true, "");
            }
        }
        // 返回值是浮点型立即数
        else if (ret.value->kind.tag == KOOPA_RVT_FLOATNUM)
        {
            if (r0[0] == 'x')
            {
                mov_print(r0, ret.value->kind.data.floatnum.value, "", true, "");
            }
            else
            {
                fmov_print(r0, ret.value->kind.data.floatnum.value, "", true, "");
            }
        }
        // 返回值存在栈中
        else if (stack_map[ret.value] >= 0)
        {
            int value_type = true_type;
            int mem_type = get_value_type(ret.value->ty);
            if (value_type != mem_type)
            {
                if (value_type == INT_TYPE)
                {
                    std::string tmp_reg = regstack_pop(mem_type);
                    ldr_print(tmp_reg, sp_reg, stack_map[ret.value], "", 1);
                    fcvt_print(tmp_reg, tmp_reg, true);
                    fmov_print(r0, 0, tmp_reg, false, "");
                    regstack_push(tmp_reg);
                }
                else // value_type = FLOAT_TYPE, mem_type = INT_TYPE
                {
                    ldr_print(r0, sp_reg, stack_map[ret.value], "", 1);
                    fcvt_print(r0, r0, false);
                }
            }
            else
            {
                ldr_print(r0, sp_reg, stack_map[ret.value], "", 1);
            }
        }
        // 返回值是用寄存器传递的参数
        else if (stack_map[ret.value] < 0)
        {
            if (src_is_param)
            {
                /// 得到传参寄存器
                if (ret.value->ty->tag == KOOPA_RTT_INT32)
                {
                    src_reg = "x" + std::to_string(stack_map[ret.value] + in_r0_r7);
                }
                else if (ret.value->ty->tag == KOOPA_RTT_FLOAT32)
                {
                    src_reg = "s" + std::to_string(stack_map[ret.value] + in_s0_s7);
                }
                else
                {
                    std::cerr << "type of ret value is not int or float" << std::endl;
                    assert(false);
                }
                /// mov操作
                mov_between_any_regs(r0, src_reg);
            }
            else
            {
                std::cerr << "stack_map[ret.value] < 0, but ret.value is not a param" << std::endl;
                assert(false);
            }
        }
        else
        {
            std::cerr << "type of ret value you have not considered" << std::endl;
            assert(false);
        }
    }

    // 恢复lr
    if (call_father) // 没有调用其它函数的函数不需要恢复lr
        ldr_print("x30", sp_reg, -size_of_stack_frame - 8, "", 1);

    // 切换栈帧
    add_print(sp_reg, sp_reg, -size_of_stack_frame, "", true,"");
    std::cout << std::setw(6) << "ret" << std::endl;
}

// 访问整数指令 //注：实际上需要  floatdone
std::string Visit_const_int(const koopa_raw_integer_t &integer)
{
    std::string reg;
    reg = regstack_pop(USE_INT_REG);
    mov_print(reg, integer.value, "", true, "");
    // regstack_push(reg);
    return reg;
}

std::string Visit_const_float(const koopa_raw_floatnum_t &floatnum)
{
    // 注：实际上可以只传一个数值进来
    std::string reg;
    reg = regstack_pop(USE_FLOAT_REG);
    fmov_print(reg, floatnum.value, "", true, "");
    return reg;
}

// 访问二元运算指令 未检查溢出  done floatdone?
void Visit_binary(const koopa_raw_binary_t &binary, const koopa_raw_value_t &value, const uint32_t use_count = 0)
{
    // 判断lhs/rhs类型是int/float
    int lhs_type = get_value_type(binary.lhs->ty);
    int rhs_type = get_value_type(binary.rhs->ty);

    // 如果操作数中有一个为浮点数，那么为浮点数运算
    bool float_cal = false;
    if (lhs_type == FLOAT_TYPE || rhs_type == FLOAT_TYPE)
    {
        float_cal = true;
    }


    // 这四个变量存lhs/rhs为立即数时的值
    int lhs_int = binary.lhs->kind.data.integer.value;
    int rhs_int = binary.rhs->kind.data.integer.value;
    float lhs_float = binary.lhs->kind.data.floatnum.value;
    float rhs_float = binary.rhs->kind.data.floatnum.value;

    // 立即数类型转换
    if (lhs_type == INT_TYPE && float_cal)
    {
        lhs_float = (float)lhs_int;
    }
    if (rhs_type == INT_TYPE && float_cal)
    {
        rhs_float = (float)rhs_int;
    }

    // 得到目的寄存器
    std::string reg;
    // TODO: 是否存在float_cal为真，但是value的类型为整数的情况？ 比如 %1 = div 1, 2
    if (reg_alloc_map.find(value) != reg_alloc_map.end())
    {
        reg = reg_alloc_map[value];
    }
    else if (float_cal) // TODO: 到底应该用float_cal还是value.type.tag来判断
        reg = regstack_pop(USE_FLOAT_REG);
    else
        reg = regstack_pop(USE_INT_REG);

    // 得到两个操作数的寄存器
    dest_reg_used = false;
    bool lhs_is_imm = (binary.lhs->kind.tag == KOOPA_RVT_INTEGER || binary.lhs->kind.tag == KOOPA_RVT_FLOATNUM);
    bool rhs_is_imm = (binary.rhs->kind.tag == KOOPA_RVT_INTEGER || binary.rhs->kind.tag == KOOPA_RVT_FLOATNUM);
    std::string lhs_reg = "";
    std::string rhs_reg = "";
    if (!lhs_is_imm || !rhs_is_imm) // TODO: 没考虑到I型指令 done
    {
        lhs_reg = prep_operand(binary.lhs, reg, value);
        rhs_reg = prep_operand(binary.rhs, reg, value);
    }

    // 整型运算
    if (!float_cal)
    {
        // 异常处理
        if ((lhs_reg == "" || rhs_reg == "") && (!lhs_is_imm || !rhs_is_imm))
        {
            std::cerr << "lhs_reg or rhs_reg is empty" << std::endl;
            assert(false);
        }
        // 现在只考虑二元运算的左右值是BINARY还是INTEGER, 在这儿得到左右值在转换为asm形式时对应的字符串
        bool lhs_is_integer = binary.lhs->kind.tag == KOOPA_RVT_INTEGER;
        bool rhs_is_integer = binary.rhs->kind.tag == KOOPA_RVT_INTEGER;
        bool lhs_is_zero = binary.lhs->kind.data.integer.value == 0;
        bool rhs_is_zero = binary.rhs->kind.data.integer.value == 0;
        switch (binary.op)
        {
        case KOOPA_RBO_EQ:
        case KOOPA_RBO_NOT_EQ:
            if (!lhs_is_integer)
            {
                // DEBUG6: 是cmp_set_print的问题
                if (!rhs_is_integer)
                    cmp_set_print(reg, lhs_reg, 0, rhs_reg, false, binary.op);
                else if (rhs_int >= 0 && rhs_int <= 0xff)
                {
                    cmp_set_print(reg, lhs_reg, rhs_int, "", true, binary.op);
                }
                else
                {
                    imm2reg(binary.rhs, rhs_reg);
                    cmp_set_print(reg, lhs_reg, 0, rhs_reg, false, binary.op);
                }
            }
            else // lhs是立即数
            {
                if (!rhs_is_integer)
                {
                    if (lhs_int >= 0 && lhs_int <= 0xff)
                    {
                        cmp_set_print(reg, rhs_reg, lhs_int, "", true, binary.op);
                    }
                    else
                    {
                        imm2reg(binary.lhs, lhs_reg);
                        cmp_set_print(reg, lhs_reg, 0, rhs_reg, false, binary.op);
                    }
                }
                else
                {
                    int32_t result;
                    if (binary.op == KOOPA_RBO_EQ)
                        result = binary.lhs->kind.data.integer.value == binary.rhs->kind.data.integer.value;
                    if (binary.op == KOOPA_RBO_NOT_EQ)
                        result = binary.lhs->kind.data.integer.value != binary.rhs->kind.data.integer.value;
                    mov_print(reg, result, "", true, "");
                }
            }
            break;
        // 注：以下指令操作数一个为立即数，一个为寄存器时未细分，而是统一全部mov到寄存器中处理，可优化
        case KOOPA_RBO_MUL:
        case KOOPA_RBO_DIV:
        case KOOPA_RBO_MOD:
        case KOOPA_RBO_GT:
        case KOOPA_RBO_LT:
        case KOOPA_RBO_LE:
        case KOOPA_RBO_GE:
        case KOOPA_RBO_SUB: // 注：需要实现RSB减法吗？
        case KOOPA_RBO_SHL: // 注：感觉移位指令移位量为寄存器时有古怪
        case KOOPA_RBO_SHR:
        case KOOPA_RBO_SAR:
            if ((binary.op == KOOPA_RBO_DIV && rhs_is_zero) || (binary.op == KOOPA_RBO_MOD && rhs_is_zero))
                assert(false);
            if (lhs_is_integer && rhs_is_integer)
            {
                int32_t result;
                if (binary.op == KOOPA_RBO_MUL)
                    result = binary.lhs->kind.data.integer.value * binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_SUB)
                    result = binary.lhs->kind.data.integer.value - binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_DIV)
                    result = binary.lhs->kind.data.integer.value / binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_MOD)
                    result = binary.lhs->kind.data.integer.value % binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_GT)
                    result = binary.lhs->kind.data.integer.value > binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_LT)
                    result = binary.lhs->kind.data.integer.value < binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_GE)
                    result = binary.lhs->kind.data.integer.value >= binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_LE)
                    result = binary.lhs->kind.data.integer.value <= binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_SHL)
                    result = binary.lhs->kind.data.integer.value << binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_SHR)
                    result = (u_int32_t)binary.lhs->kind.data.integer.value >> binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_SAR)
                    result = binary.lhs->kind.data.integer.value >> binary.rhs->kind.data.integer.value;
                // std::cout << std::setw(6) << "li" << reg << ", " << result << std::endl;
                mov_print(reg, result, "", true, "");
                break;
            }
            // 把立即数转移到寄存器
            imm2reg(binary.lhs, lhs_reg);
            imm2reg(binary.rhs, rhs_reg);
            if (binary.op == KOOPA_RBO_DIV) // 注：默认都用带符号除法
            {
                // 被除数为0
                if (lhs_is_zero)
                {
                    mov_print(reg, 0, "", true, "");
                    break;
                }
                // AArch64 ASR rounds negative values toward minus infinity, while
                // SysY signed division must truncate toward zero.
                std::cout << std::setw(6) << "sdiv" << reg << ", " << lhs_reg << ", " << rhs_reg << std::endl;
            }
            else if (binary.op == KOOPA_RBO_MOD)
            {
                // 被除数为0
                if (lhs_is_zero)
                {
                    mov_print(reg, 0, "", true, "");
                    break;
                }
                // AND is not a valid signed remainder lowering for negative
                // dividends.  Compute lhs - trunc(lhs / rhs) * rhs instead.
                std::string tmp_reg;
                tmp_reg = regstack_pop(USE_INT_REG);
                std::cout << std::setw(6) << "sdiv" << tmp_reg << ", " << lhs_reg << ", " << rhs_reg << std::endl;
                std::cout << std::setw(6) << "mul" << tmp_reg << ", " << tmp_reg << ", " << rhs_reg << std::endl;
                std::cout << std::setw(6) << "sub" << reg << ", " << lhs_reg << ", " << tmp_reg << std::endl;
                reg_stack.push(tmp_reg);
            }
            else if (binary.op == KOOPA_RBO_MUL && rhs_is_integer && getPowerOfTwo(binary.rhs->kind.data.integer.value) != -1)
            {
                // 乘法削弱成左移
                int power2 = getPowerOfTwo(binary.rhs->kind.data.integer.value);
                std::cout << std::setw(6) << "lsl" << reg << ", " << lhs_reg << ", #" << power2 << std::endl;
            }
            else if (binary.op == KOOPA_RBO_MUL && lhs_is_integer && getPowerOfTwo(binary.lhs->kind.data.integer.value) != -1)
            {
                // 乘法削弱成左移
                int power2 = getPowerOfTwo(binary.lhs->kind.data.integer.value);
                std::cout << std::setw(6) << "lsl" << reg << ", " << rhs_reg << ", #" << power2 << std::endl;
            }
            else if (binary.op == KOOPA_RBO_LE || binary.op == KOOPA_RBO_GE || binary.op == KOOPA_RBO_GT || binary.op == KOOPA_RBO_LT)
                cmp_set_print(reg, lhs_reg, 0, rhs_reg, false, binary.op);
            else
                std::cout << std::setw(6) << ir_asm_binaryop[binary.op] << reg << ", " << lhs_reg << ", " << rhs_reg << std::endl;
            break;
        case KOOPA_RBO_AND:
        case KOOPA_RBO_OR:
        case KOOPA_RBO_XOR:
        case KOOPA_RBO_ADD:
            if (lhs_is_integer && rhs_is_integer)
            {
                int32_t result;
                if (binary.op == KOOPA_RBO_ADD)
                    result = binary.lhs->kind.data.integer.value + binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_XOR)
                    result = binary.lhs->kind.data.integer.value ^ binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_AND)
                    result = binary.lhs->kind.data.integer.value & binary.rhs->kind.data.integer.value;
                if (binary.op == KOOPA_RBO_OR)
                    result = binary.lhs->kind.data.integer.value | binary.rhs->kind.data.integer.value;
                mov_print(reg, result, "", true, "");
            }
            if (!lhs_is_integer)
            {
                if (!rhs_is_integer)
                    std::cout << std::setw(6) << ir_asm_binaryop[binary.op] << reg << ", " << lhs_reg << ", " << rhs_reg << std::endl;
                else if (binary.op == KOOPA_RBO_ADD)
                {
                    add_print(reg, lhs_reg, rhs_int, "", true,"");
                }
                else
                {
                    bit_cal_print(reg, lhs_reg, rhs_int, "", true, binary.op);
                }
            }
            else
            {
                if (!rhs_is_integer) // lhs是立即数，rhs是寄存器数
                {
                    if (binary.op == KOOPA_RBO_ADD)
                        add_print(reg, rhs_reg, lhs_int, "", true,"");
                    else
                        bit_cal_print(reg, rhs_reg, lhs_int, "", true, binary.op);
                }
            }
            break;
        };
        // 写入内存同时释放寄存器
        if (is_temp_reg(reg))
        {
            if (!binary_for_br)
                str_print(reg, sp_reg, stack_map[value], "", 1);
            regstack_push(reg);
        }
        // 释放寄存器
        if (is_temp_reg(lhs_reg))
        {
            regstack_push(lhs_reg);
        }
        if (is_temp_reg(rhs_reg))
        {
            regstack_push(rhs_reg);
        }
        return;
    }

    // 浮点运算
    /// 为操作数分配寄存器
    // now
    std::string lhs, rhs;
    if (!lhs_is_imm || !rhs_is_imm)
    {
        lhs = int2float_reg(lhs_type, lhs_is_imm, lhs_float, lhs_reg);
        rhs = int2float_reg(rhs_type, rhs_is_imm, rhs_float, rhs_reg);
    }
    bool lhs_is_zero = (lhs_float == 0.0f);
    bool rhs_is_zero = (rhs_float == 0.0f);
    /// 输出汇编代码
    if (float_cal)
    {
        switch (binary.op)
        {
        case KOOPA_RBO_MUL: // 有float
        case KOOPA_RBO_DIV: // 有float
        case KOOPA_RBO_SUB: // 有float
        case KOOPA_RBO_ADD: // 有float
            if (binary.op == KOOPA_RBO_DIV && rhs_is_zero)
                assert(false);
            // 操作数都是立即数
            if (lhs_is_imm && rhs_is_imm)
            {
                float result;
                if (binary.op == KOOPA_RBO_MUL)
                    result = lhs_float * rhs_float;
                if (binary.op == KOOPA_RBO_SUB)
                    result = lhs_float - rhs_float;
                if (binary.op == KOOPA_RBO_DIV)
                    result = lhs_float / rhs_float;
                if (binary.op == KOOPA_RBO_ADD)
                    result = lhs_float + rhs_float;
                fmov_print(reg, result, "", true, "");
                break;
            }
            if (lhs_is_zero)
            {
                if (binary.op == KOOPA_RBO_MUL)
                    fmov_print(reg, 0.0f, "", true, "");
                if (binary.op == KOOPA_RBO_DIV)
                    fmov_print(reg, 0.0f, "", true, "");
                if (binary.op == KOOPA_RBO_ADD)
                    fmov_print(reg, 0, rhs, false, "");
                if (binary.op == KOOPA_RBO_SUB)
                {
                    std::cout << std::setw(6) << "fsub " << reg << ", " << lhs << ", " << rhs << std::endl;
                }
                break;
            }
            if (rhs_is_zero)
            {
                if (binary.op == KOOPA_RBO_MUL)
                    fmov_print(reg, 0.0f, "", true, "");
                if (binary.op == KOOPA_RBO_ADD)
                    fmov_print(reg, 0, lhs, false, "");
                if (binary.op == KOOPA_RBO_SUB)
                    fmov_print(reg, 0, lhs, false, "");
                if (binary.op == KOOPA_RBO_DIV)
                {
                    std::cerr << "div by zero for float" << std::endl;
                    assert(false);
                }
                break;
            }
            if (binary.op == KOOPA_RBO_DIV) // 注：默认都用带符号除法
                std::cout << std::setw(6) << "fdiv " << reg << ", " << lhs << ", " << rhs << std::endl;
            else if (binary.op == KOOPA_RBO_SUB) // 注：默认都用带符号除法
                std::cout << std::setw(6) << "fsub " << reg << ", " << lhs << ", " << rhs << std::endl;
            else if (binary.op == KOOPA_RBO_MUL) // 注：默认都用带符号除法
                std::cout << std::setw(6) << "fmul " << reg << ", " << lhs << ", " << rhs << std::endl;
            else if (binary.op == KOOPA_RBO_ADD) // 注：默认都用带符号除法
                std::cout << std::setw(6) << "fadd " << reg << ", " << lhs << ", " << rhs << std::endl;
            break;
        case KOOPA_RBO_GT: // 有float
        case KOOPA_RBO_LT: // 有float
        case KOOPA_RBO_LE: // 有float
        case KOOPA_RBO_GE: // 有float
        case KOOPA_RBO_EQ:
        case KOOPA_RBO_NOT_EQ:
            if (is_temp_reg(reg))
            {
                regstack_push(reg);
                reg = regstack_pop(USE_INT_REG);
            }
            if (lhs_is_imm && rhs_is_imm)
            {
                int result;
                if (binary.op == KOOPA_RBO_GT)
                    result = lhs_float > rhs_float;
                if (binary.op == KOOPA_RBO_LT)
                    result = lhs_float < rhs_float;
                if (binary.op == KOOPA_RBO_GE)
                    result = lhs_float >= rhs_float;
                if (binary.op == KOOPA_RBO_LE)
                    result = lhs_float <= rhs_float;
                if (binary.op == KOOPA_RBO_EQ)
                    result = lhs_float == rhs_float;
                if (binary.op == KOOPA_RBO_NOT_EQ)
                    result = lhs_float != rhs_float;
                mov_print(reg, result, "", true, "");
                break;
            }
            fcmp_set_print(reg, lhs, 0, rhs, false, binary.op);
            break;
        default:
            std::cout << "float 不支持该类型运算" << std::endl;
            break;
        };
        // 写入内存
        if (is_temp_reg(reg))
        {
            if (!binary_for_br)
                str_print(reg, sp_reg, stack_map[value], "", 1);
            regstack_push(reg);
        }
        // 释放寄存器
        if (is_temp_reg(lhs))
            regstack_push(lhs);
        if (is_temp_reg(rhs))
            regstack_push(rhs);
    }
    // lhs_reg, rhs_reg, lhs, rhs都要判断是否释放
}

// alloc指令  floatdone
void Visit_alloc(const koopa_raw_value_t &value)
{
    // 为value分配寄存器
    std::string reg;
    if (reg_alloc_map.find(value) != reg_alloc_map.end())
    {
        reg = reg_alloc_map[value];
    }
    else
    {
        reg = regstack_pop(USE_INT_REG);
    }

    // alloc的类型是数组
    if (value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY)
    {
        // 定义整形立即数
        int imm = stack_map[value] + 8;
        add_print(reg, sp_reg, imm, "", true,"");
        // store的原因是get_ptr和get_elem_ptr的时候需要用到
        if (is_temp_reg(reg))
        {
            str_print(reg, sp_reg, stack_map[value], "", 1);
        }
        // str_print(reg, sp_reg, stack_map[value], "", 1);
        if (is_temp_reg(reg))
        {
            regstack_push(reg);
        }
        // else {
        //     add_print(reg, reg, -4, "", true);
        // }
        return;
    }

    // alloc的类型是整形或浮点型
    if (value->ty->data.pointer.base->tag == KOOPA_RTT_INT32 || value->ty->data.pointer.base->tag == KOOPA_RTT_FLOAT32)
    {
        // 如果没被分配寄存器就不需要把地址放入寄存器，在load的时候会把地址放入寄存器
        if (is_temp_reg(reg))
        {
            regstack_push(reg);
            return;
        }
        add_print(reg, sp_reg, stack_map[value], "", true,"");
    }
}

// store指令  floatdone  已重构
void Visit_store(const koopa_raw_store_t &store)
{
    // 数组初始化
    if (store.value->kind.tag == KOOPA_RVT_AGGREGATE)
    {
        store_array(store.value, store.dest);
        return;
    }

    // value
    // 指定寄存器
    dest_reg_used = true;
    std::string reg;      // 存store value的寄存器
    std::string dest_reg; // 存store dest的寄存器
    int src_type = get_param_type(store.value->ty);
    int dest_type = get_value_type(store.dest->ty);
    // 注：需要改前中端来配合
    if (store.value->kind.tag == KOOPA_RVT_UNDEF)
        return;
    // 0初始化
    if (store.value->kind.tag == KOOPA_RVT_ZERO_INIT)
    {
        // 数组0初始化，TODO，可以优化 done
        if (store.dest->ty->tag == KOOPA_RTT_POINTER && store.dest->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY)
        {
            /// 数组地址前保留一个 8 字节槽，用于保存数组基址。
            int offset = stack_map[store.dest];

            if (reg_stack.empty())
                assert(false);
            std::string cur_reg = regstack_pop(USE_INT_REG);
            add_print(cur_reg, sp_reg, offset, "", true,"");

            /// 数组零初始化，用预索引的方式来实现
            offset = SIZE_OF_STACK_UNIT;
            for (size_t i = 0; i < cal_array_length(*store.dest->ty->data.pointer.base); i++)
            {
                str_print(zero_reg, cur_reg, offset, "", 4);
                offset = SYSY_VALUE_SIZE;
            }
            regstack_push(cur_reg);
            return;
        }
        // 普通变量0初始化
        else
        {
            reg = zero_reg;
        }
    }
    // 非0初始化
    else
    {
        // 确定value_reg
        reg = prep_operand(store.value, "", nullptr);
        imm2reg(store.value, reg);
    }

    // 上面是把value放入寄存器，下面是把value放入内存

    /// 确定dest_reg
    if (store.dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        if (reg_alloc_map.find(store.dest) != reg_alloc_map.end())
        {
            dest_reg = reg_alloc_map[store.dest];
        }
        else
        {
            dest_reg = regstack_pop(USE_INT_REG);
            gloabal_var_load(dest_reg, store.dest->name + 1);
        }
        str_print(dest_type, src_type, dest_reg, reg);
    }
    else // 注：是不是没有考虑 目的为寄存器中的参数（a0-a7）这种情况：这种情况不可能
    // 注：可以考虑立即数范围，两条语句变一条
    {
        // DEBUG 3
        // dest_reg = prep_operand(store.dest, "", nullptr);
        if (reg_alloc_map.find(store.dest) != reg_alloc_map.end())
        {
            dest_reg = reg_alloc_map[store.dest];
        }
        else
        {
            dest_reg = regstack_pop(USE_INT_REG);
        }
        // store.dest不可能是常量和参数
        if (store.dest->name[0] == '%' && (store.dest->kind.tag == KOOPA_RVT_GET_ELEM_PTR || store.dest->kind.tag == KOOPA_RVT_GET_PTR)) // 如果是数组变量需要先加载地址
        {
            if (is_temp_reg(dest_reg))
            {
                ldr_print(dest_reg, sp_reg, stack_map[store.dest], "", 1);
            }
            // str_print(reg, dest_reg, 0, "", 1);
            str_print(dest_type, src_type, dest_reg, reg);
        }
        else // store.dest是普通局部变量
        {
            if (!is_temp_reg(dest_reg))
            {
                // str_print(reg, dest_reg, 0, "", 1);
                str_print(dest_type, src_type, dest_reg, reg);
            }
            else
            {
                if (src_type != dest_type)
                {
                    std::string tmp_reg;
                    if (dest_type == INT_TYPE)
                    { // dest是int型，src是float型，会被改变，需要进行保护
                        if (is_temp_reg(reg))
                        {
                            tmp_reg = reg;
                        }
                        else
                        {
                            tmp_reg = regstack_pop(src_type);
                            mov_between_any_regs(tmp_reg, reg);
                        }
                        fcvt_print(tmp_reg, tmp_reg, true);
                    }
                    else
                    { // dest是float型，src是int型，不会被改变
                        tmp_reg = regstack_pop(dest_type);
                        fmov_print(tmp_reg, 0, reg, false, "");
                        fcvt_print(tmp_reg, tmp_reg, false);
                    }
                    str_print(tmp_reg, sp_reg, stack_map[store.dest], "", 1);
                    if (is_temp_reg(tmp_reg))
                    {
                        regstack_push(tmp_reg);
                    }
                }
                else
                {
                    str_print(reg, sp_reg, stack_map[store.dest], "", 1);
                }
                // str_print(reg, sp_reg, stack_map[store.dest], "", 1);
            }
        }
    }

    if (is_temp_reg(reg))
    {
        regstack_push(reg);
    }
    if (is_temp_reg(dest_reg))
    {
        regstack_push(dest_reg);
    }
}

// load指令  floatdone 已做寄存器分配
void Visit_load(const koopa_raw_load_t &load, const koopa_raw_value_t &value)
{
    std::string reg;          // 存load的结果
    std::string src_reg = ""; // 存load的src
    int value_type = get_param_type(value->ty);
    int mem_type = get_value_type(load.src->ty);
    // 得到相应寄存器
    /// 得到reg
    if (reg_alloc_map.find(value) != reg_alloc_map.end())
    {
        reg = reg_alloc_map[value];
    }
    else
    {
        reg = regstack_pop(value_type);
    }
    /// 得到src_reg, src_reg肯定是INT型
    dest_reg_used = false;

    // 从内存中加载
    /// 从全局变量中加载
    if (load.src->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
    {
        src_reg = prep_operand(load.src, reg, value);
        // ldr_print(reg, src_reg, 0, "", 1);
        ldr_print(value_type, mem_type, src_reg, reg);
    }
    /// 从局部变量中加载
    else
    {
        //// %ptr
        if (load.src->name[0] == '%' && (load.src->kind.tag == KOOPA_RVT_GET_ELEM_PTR || load.src->kind.tag == KOOPA_RVT_GET_PTR))
        {
            src_reg = prep_operand(load.src, reg, value);
            // ldr_print(reg, src_reg, 0, "", 1);
            ldr_print(value_type, mem_type, src_reg, reg);
        }
        //// %c = alloc i32 , @ c = alloc i32
        else
        {
            if (reg_alloc_map.find(load.src) != reg_alloc_map.end())
            {
                src_reg = reg_alloc_map[load.src];
                // ldr_print(reg, src_reg, 0, "", 1);
                ldr_print(value_type, mem_type, src_reg, reg);
            }
            else // 直接用栈帧偏移量load
            {
                if (value_type != mem_type)
                {
                    if (value_type == INT_TYPE)
                    {
                        std::string tmp_reg = regstack_pop(mem_type);
                        ldr_print(tmp_reg, sp_reg, stack_map[load.src], "", 1);
                        fcvt_print(tmp_reg, tmp_reg, true);
                        fmov_print(reg, 0, tmp_reg, false, "");
                        regstack_push(tmp_reg);
                    }
                    else // value_type = FLOAT_TYPE, mem_type = INT_TYPE
                    {
                        ldr_print(reg, sp_reg, stack_map[load.src], "", 1);
                        fcvt_print(reg, reg, false);
                    }
                }
                else
                {
                    ldr_print(reg, sp_reg, stack_map[load.src], "", 1);
                }
            }
        }
    }
    // 写回内存并释放寄存器
    if (is_temp_reg(reg))
    {
        str_print(reg, sp_reg, stack_map[value], "", 1);
        regstack_push(reg);
    }
    if (is_temp_reg(src_reg))
    {
        regstack_push(src_reg);
    }
}

// TODO9
//  branch指令  floatdone  已重构 已做寄存器分配
void Visit_branch(const koopa_raw_branch_t &branch)
{
    if (binary_for_br == true)
    {
        switch (binary_for_br_op)
        {
        case KOOPA_RBO_GT: //>
            std::cout << std::setw(6) << "b.gt" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
            std::cout << std::setw(6) << "b.le" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
            break;
        case KOOPA_RBO_LT: //<
            std::cout << std::setw(6) << "b.lt" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
            std::cout << std::setw(6) << "b.ge" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
            break;
        case KOOPA_RBO_LE: //<=
            std::cout << std::setw(6) << "b.le" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
            std::cout << std::setw(6) << "b.gt" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
            break;
        case KOOPA_RBO_GE: //>=
            std::cout << std::setw(6) << "b.ge" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
            std::cout << std::setw(6) << "b.lt" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
            break;
        case KOOPA_RBO_EQ: //==
            std::cout << std::setw(6) << "b.eq" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
            std::cout << std::setw(6) << "b.ne" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
            break;
        case KOOPA_RBO_NOT_EQ: //!=
            std::cout << std::setw(6) << "b.ne" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
            std::cout << std::setw(6) << "b.eq" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
            break;
        default:
            break;
        }
        return;
    }
    int cond_type = get_value_type(branch.cond->ty);
    // 获得存储条件变量的寄存器
    dest_reg_used = true;
    std::string reg = prep_operand(branch.cond, "", nullptr);
    imm2reg(branch.cond, reg);

    // 输出汇编代码
    if (cond_type == FLOAT_TYPE) // 注：float判断 修改标志位的方法不同
    {
        std::string x=reg_stack.top();
        reg_stack.pop();
        std::string xx='w'+x.substr(1);
        std::cout << std::setw(cout_len) << "cmp " << xx << ", " << zero_reg << std::endl;
        reg_stack.push(x);
        /// 不等于0跳转到true_bb
        std::cout << std::setw(6) << "b.ne" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
        /// 等于0跳转到false_bb
        std::cout << std::setw(6) << "b.eq" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
    }
    else
    {
        cmp_print(reg, 0, zero_reg, false);
        /// 不等于0跳转到true_bb
        std::cout << std::setw(6) << "b.ne" << func_name + '_' << branch.true_bb->name + 1 << std::endl;
        /// 等于0跳转到false_bb
        std::cout << std::setw(6) << "b.eq" << func_name + '_' << branch.false_bb->name + 1 << std::endl;
    }

    // 释放寄存器
    if (is_temp_reg(reg))
    {
        regstack_push(reg);
    }
}

// TODO9
//   jump指令 已做寄存器分配
void Visit_jump(const koopa_raw_jump_t &jump)
{
    if (jump.target->name == next_bb)
        return;
    std::cout << std::setw(6) << "b" << func_name + '_' << jump.target->name + 1 << std::endl;
}

// TODO: get_value_type函数有问题，前面涉及到这个函数的地方要检查一下 done

// 访问 call 指令  函数调用 floatdone 已做寄存器分配
void Visit_call(const koopa_raw_call_t &call, const koopa_raw_value_t &fa_value)
{
    // 分别统计int/float形式参数个数
    int float_param_num = 0;
    int int_param_num = 0;
    for (int i = 0; i < call.args.len; i++)
    {
        int param_type = get_param_type(reinterpret_cast<koopa_raw_type_t>(call.callee->ty->data.function.params.buffer[i]));
        if (param_type == FLOAT_TYPE)
            float_param_num += 1;
        else if (param_type == INT_TYPE)
            int_param_num += 1;
        else
        {
            std::cout << "参数类型错误" << std::endl;
        }
    }

    // TODO8
    // 保护寄存器
    std::string rr=reg_stack.top();
    reg_stack.pop();
    add_print(rr, sp_reg, -size_of_stack_frame - FRAME_SIZE_OF_R_REGS - FRAME_SIZE_OF_S_REGS, "", true,"");
    for (int i = 0; i < 8; i += 2) 
    {
         std::cout << "stp x" << i << ", x" << i+1 << ", [" << rr << "], #16" << std::endl;
    }
    for (int i = 19; i < 30; i += 2) 
    {
        std::cout << "stp x" << i << ", x" << i+1 << ", [" << rr << "], #16" << std::endl;
    }

    for (int i = 0; i < 8; i += 2) 
    {
         std::cout << "stp s" << i << ", s" << i+1 << ", [" << rr << "], #8" << std::endl;
    }
    for (int i = 16; i <= 30; i += 2) 
    {
        std::cout << "stp s" << i << ", s" << i+1 << ", [" << rr << "], #8" << std::endl;
    }

    // 传递实参
    int now_int_num = -1, now_float_num = -1, sp_ofsset = 0;
    for (int i = 0; i < call.args.len; i++)
    {
        // 取实参value
        koopa_raw_value_t value = (koopa_raw_value_t)(call.args.buffer[i]);
        int param_type = get_param_type(reinterpret_cast<koopa_raw_type_t>(call.callee->ty->data.function.params.buffer[i]));
        if (param_type == INT_TYPE)
            now_int_num += 1;
        else if (param_type == FLOAT_TYPE)
            now_float_num += 1;

        std::string dest_reg;
        if ((param_type == INT_TYPE && (now_int_num < w_regs_for_arg))) // 实参传入整型寄存器
        {
            dest_reg = "x" + std::to_string(now_int_num);

            // 如果value已经分配了寄存器，call语句可以改变寄存器的值吗？不能！！！// CUR BUG
            if (reg_alloc_map.find(value) != reg_alloc_map.end())
            {
                std::string src_reg = reg_alloc_map[value];
                std::string tmp_reg;
                if (src_reg[0] == 's')
                {
                    tmp_reg = regstack_pop(USE_FLOAT_REG);
                    fmov_print(tmp_reg, 0, src_reg, false, "");
                    mov_between_any_regs(dest_reg, tmp_reg); // mov_between_any_regs可能会修改src_reg的值，所以在寄存器中的值要用tmp_reg存储，防止被覆盖
                    regstack_push(tmp_reg);
                }
                else
                {
                    mov_print(dest_reg, 0, src_reg, false, "");
                }
            }
            else // 如果value没有分配寄存器
            {
                dest_reg_used = false;
                // DONE DEBUG7：原因：prep_operand里面因为reg_value_map[stoi(dest_reg.substr(1))] == value这里只考虑了被分配寄存器，
                //  而此处传入的dest_reg是传参寄存器，相应的reg_value_map的值是nullptr所以prep_operand直接用一个临时寄存器存了实参，
                //  而没有用传参寄存器，解决方法是value处传入nullptr，这样prep_operand就会用传参寄存器，同时update_reg_value_map也要改
                std::string src_reg = prep_operand(value, dest_reg, nullptr);
                /// 如果value是立即数
                if (value->kind.tag == KOOPA_RVT_INTEGER)
                {
                    mov_print(dest_reg, value->kind.data.integer.value, "", true, "");
                }
                else if (value->kind.tag == KOOPA_RVT_FLOATNUM)
                {
                    mov_print(dest_reg, value->kind.data.floatnum.value, "", true, "");
                }
                else if (is_temp_reg(src_reg)) // 没考虑src_reg是传参寄存器的情况：由于IR会有相应的临时变量把形参存起来，所以不会出现这种情况
                {
                    regstack_push(src_reg);
                    // 如果src_reg和dest_reg_used是同类型寄存器，那么在prep_operand里面就已经把值load 到 dest_reg_used中了
                    if (dest_reg[0] != src_reg[0])
                    {
                        mov_between_any_regs(dest_reg, src_reg);
                    }
                }
            }
        }
        else if (param_type == FLOAT_TYPE && (now_float_num < s_regs_for_arg)) // 实参传入浮点寄存器
        {
            dest_reg = "s" + std::to_string(now_float_num);

            // 如果value已经分配了寄存器
            if (reg_alloc_map.find(value) != reg_alloc_map.end())
            {
                std::string src_reg = reg_alloc_map[value];
                fmov_print(dest_reg, 0, src_reg, false, "");
                if (src_reg[0] == 'x')
                {
                    fcvt_print(dest_reg, dest_reg, false);
                }
            }
            else // 如果value没有分配寄存器
            {
                dest_reg_used = false;
                std::string src_reg = prep_operand(value, dest_reg, nullptr);
                /// 如果value是立即数
                if (value->kind.tag == KOOPA_RVT_FLOATNUM)
                {
                    fmov_print(dest_reg, value->kind.data.floatnum.value, "", true, "");
                }
                else if (value->kind.tag == KOOPA_RVT_INTEGER)
                {
                    fmov_print(dest_reg, value->kind.data.integer.value, "", true, "");
                }
                else if (is_temp_reg(src_reg))
                {
                    regstack_push(src_reg);
                    // 如果src_reg和dest_reg是同类型的寄存器，那么prep_operand中就已经把值load进去了
                    if (src_reg[0] == 'x')
                    {
                        fmov_print(dest_reg, 0, src_reg, false, "");
                        fcvt_print(dest_reg, dest_reg, false);
                    }
                }
            }
        }
        else // 把实参传到栈中
        {
            dest_reg_used = true;
            std::string reg = prep_operand(value, "", nullptr);
            imm2reg(value, reg);
            str_print(reg, sp_reg, sp_ofsset, "", 1);
            sp_ofsset += 8;
            if (is_temp_reg(reg))
            {
                regstack_push(reg);
            }
        }
    }
    std::cout << std::setw(6) << "bl" << call.callee->name + 1 << std::endl;

    // 恢复前暂存返回值寄存器
    std::string tmp_ret_reg;
    if (call.callee->ty->data.function.ret->tag == KOOPA_RTT_FLOAT32)
    {
        tmp_ret_reg = regstack_pop(USE_FLOAT_REG);
        mov_between_any_regs(tmp_ret_reg, "s0");
    }
    else if (call.callee->ty->data.function.ret->tag == KOOPA_RTT_INT32)
    {
        tmp_ret_reg = regstack_pop(USE_INT_REG);
        mov_between_any_regs(tmp_ret_reg, "x0");
    }
    // 恢复传参寄存器
    add_print(rr, sp_reg, -size_of_stack_frame - FRAME_SIZE_OF_R_REGS - FRAME_SIZE_OF_S_REGS, "", true,"");
    for (int i = 0; i < 8; i += 2) 
    {
         std::cout << "ldp x" << i << ", x" << i+1 << ", [" << rr << "], #16" << std::endl;
    }
    for (int i = 19; i < 30; i += 2) 
    {
        std::cout << "ldp x" << i << ", x" << i+1 << ", [" << rr << "], #16" << std::endl;
    }

    for (int i = 0; i < 8; i += 2) 
    {
         std::cout << "ldp s" << i << ", s" << i+1 << ", [" << rr << "], #8" << std::endl;
    }
    for (int i = 16; i <= 30; i += 2) 
    {
        std::cout << "ldp s" << i << ", s" << i+1 << ", [" << rr << "], #8" << std::endl;
    }
    reg_stack.push(rr);
    
    // 保存返回值
    if (call.callee->ty->data.function.ret->tag == KOOPA_RTT_INT32)
    {
        auto it = reg_alloc_map.find(fa_value);
        if (it != reg_alloc_map.end())
        {
            std::string reg = (*it).second;
            update_reg_value_map(fa_value, reg);
            mov_print(reg, 0, tmp_ret_reg, false, "");
        }
        else
        {
            str_print(tmp_ret_reg, sp_reg, stack_map[fa_value], "", 1);
        }
        regstack_push(tmp_ret_reg);
    }
    else if (call.callee->ty->data.function.ret->tag == KOOPA_RTT_FLOAT32)
    {
        auto it = reg_alloc_map.find(fa_value);
        if (it != reg_alloc_map.end())
        {
            std::string reg = (*it).second;
            update_reg_value_map(fa_value, reg);
            fmov_print(reg, 0, tmp_ret_reg, false, "");
        }
        else
        {
            str_print(tmp_ret_reg, sp_reg, stack_map[fa_value], "", 1);
        }
        regstack_push(tmp_ret_reg);
    }
}

void Visit_global_alloc(const koopa_raw_global_alloc_t &global_alloc, const koopa_raw_value_t &value)
{
    // 非零初始化的全局变量需要声明全局可见性并定义标签
    if (global_alloc.init->kind.tag != KOOPA_RVT_ZERO_INIT)
    {
        std::cout << ".global " << value->name + 1 << std::endl;  // 声明全局可见
        std::cout << value->name + 1 << ':' << std::endl;         // 定义变量标签
    }

    switch (global_alloc.init->kind.tag)
    {
    // SysY 的 int/float 均为 32 位对象。
    case KOOPA_RVT_INTEGER: 
        std::cout << ".word " << global_alloc.init->kind.data.integer.value << std::endl;
        break;

    case KOOPA_RVT_FLOATNUM: 
        float2uint32(global_alloc.init->kind.data.floatnum.value);
        std::cout << ".word " << fimm_32 << std::endl;
        break;

    // 零初始化（全局变量默认值为0）
    case KOOPA_RVT_ZERO_INIT:
        // 全局数组和标量均按其 32 位元素宽度分配。
        if (value->ty->data.pointer.base->tag == KOOPA_RTT_ARRAY)
        {
            std::cout << ".comm " << value->name + 1 
                      << ',' << cal_array_length(*value->ty->data.pointer.base) * SYSY_VALUE_SIZE
                      << ',' << SYSY_VALUE_SIZE << std::endl;
        }
        else
        {
            std::cout << ".comm " << value->name + 1 
                      << ',' << SYSY_VALUE_SIZE
                      << ',' << SYSY_VALUE_SIZE
                      << std::endl;
        }
        break;

    // 聚合类型初始化（结构体/数组等复合类型）
    case KOOPA_RVT_AGGREGATE:
    {
        int init_type = get_value_type(global_alloc.init->ty);
        int zero_count = 0;
        bool zero_flag = false;

        // 整数类型聚合（如int32数组）
        if (init_type != 2)
        {
            std::list<int32_t> init_list;
            koopa_raw_aggregate_t _aggregate = global_alloc.init->kind.data.aggregate;
            analysis_aggregate(_aggregate, init_list);  // 展开嵌套的聚合初始化器

            for (auto i : init_list)
            {
                if (i == 0)
                {
                    // 连续零元素，累计计数
                    zero_flag = true;
                    zero_count++;
                }
                else
                {
                    if (zero_flag)
                    {
                        std::cout << ".zero " << zero_count * SYSY_VALUE_SIZE << std::endl;
                        zero_flag = false;
                        zero_count = 0;
                    }
                    std::cout << ".word " << i << std::endl;
                }
            }
        }
        // 浮点类型聚合（如float32数组）
        else
        {
            std::list<float> init_list;
            koopa_raw_aggregate_t _aggregate = global_alloc.init->kind.data.aggregate;
            analysis_float_aggregate(_aggregate, init_list);  // 展开浮点初始化器

            for (auto i : init_list)
            {
                if (i == 0.0f)
                {
                    // 连续零元素，累计计数
                    zero_flag = true;
                    zero_count++;
                }
                else
                {
                    if (zero_flag)
                    {
                        std::cout << ".zero " << zero_count * SYSY_VALUE_SIZE << std::endl;
                        zero_flag = false;
                        zero_count = 0;
                    }
                    float2uint32(i);
                    std::cout << ".word " << fimm_32 << std::endl;
                }
            }
        }

        // 处理剩余的连续零元素
        if (zero_flag)
        {
            std::cout << ".zero " << zero_count * SYSY_VALUE_SIZE << std::endl;
        }
        break;
    }

    default:
        std::cerr << "Unsupported global init type: " << global_alloc.init->kind.tag << std::endl;
        assert(false);
    }
}

// 访问 get_elem_ptr 指令  zy %0 = getelemptr @a, 2， %1 = getelemptr %0, 3 已做寄存器分配
void Visit_elem_ptr(const koopa_raw_get_elem_ptr_t &get_elem_ptr, const koopa_raw_value_t &value)
{
    std::string reg;
    std::string src_reg;
    std::string index_reg;
    // 确定寄存器
    /// 确定reg
    if (reg_alloc_map.find(value) != reg_alloc_map.end())
    {
        reg = reg_alloc_map[value];
    }
    else
    {
        reg = regstack_pop(USE_INT_REG);
    }

    dest_reg_used = false;

    /// 确定src_reg
    src_reg = prep_operand(get_elem_ptr.src, reg, value);
    // IR规范：get_elem_ptr的src不可能是常量
    // imm2reg(get_elem_ptr.src, src_reg); TODO:这里可能有问题

    /// 确定index_reg，并计算偏移量
    index_reg = prep_operand(get_elem_ptr.index, reg, value);
    if (get_elem_ptr.index->kind.tag == KOOPA_RVT_INTEGER)
    {
        int index_num = get_elem_ptr.index->kind.data.integer.value * cal_array_length(*get_elem_ptr.src->ty->data.pointer.base->data.array.base) * SYSY_VALUE_SIZE;
        add_print(reg, src_reg, index_num, "", true,"");
    }
    else // index不是立即数
    {
        // 此时index_reg可能是分配寄存器中的一个
        std::string tmp_reg = regstack_pop(USE_INT_REG);
        mov_print(tmp_reg, cal_array_length(*get_elem_ptr.src->ty->data.pointer.base->data.array.base) * SYSY_VALUE_SIZE, "", true, "");
        std::cout << std::setw(6) << "madd" << reg << ", " << index_reg << ", " << tmp_reg << ", " << src_reg << std::endl;
        // std::cout << std::setw(6) << "mul" << tmp_reg << ", " << index_reg << ", " << tmp_reg << std::endl;
        if (is_temp_reg(index_reg))
        {
            regstack_push(index_reg);
        }
        index_reg = tmp_reg;
    }

    // 写回内存并释放寄存器
    if (is_temp_reg(reg))
    {
        str_print(reg, sp_reg, stack_map[value], "", 1);
        regstack_push(reg);
    }
    if (is_temp_reg(src_reg))
    {
        regstack_push(src_reg);
    }
    if (is_temp_reg(index_reg))
    {
        regstack_push(index_reg);
    }
}

// 访问 get_ptr 指令  zy %0 = getptr @a, 2， %1 = getptr %0, 3
void Visit_ptr(const koopa_raw_get_ptr_t &get_ptr, const koopa_raw_value_t &value)
{
    std::string reg;
    std::string src_reg;
    std::string index_reg;
    // 确定寄存器
    /// 确定reg
    if (reg_alloc_map.find(value) != reg_alloc_map.end())
    {
        reg = reg_alloc_map[value];
    }
    else
        reg = regstack_pop(USE_INT_REG);
    dest_reg_used = false;

    /// 确定src_reg
    src_reg = prep_operand(get_ptr.src, reg, value);
    // IR规范：get_elem_ptr的src不可能是常量

    /// 确定index_reg，并计算偏移量
    index_reg = prep_operand(get_ptr.index, reg, value);
    if (get_ptr.index->kind.tag == KOOPA_RVT_INTEGER)
    {
        mov_print(index_reg, get_ptr.index->kind.data.integer.value * cal_array_length(*get_ptr.src->ty->data.pointer.base), "", true, "");
    }
    else
    {
        std::string tmp_reg = regstack_pop(USE_INT_REG);
        mov_print(tmp_reg, cal_array_length(*get_ptr.src->ty->data.pointer.base), "", true, "");
        std::cout << std::setw(6) << "mul" << tmp_reg << ", " << index_reg << ", " << tmp_reg << std::endl;
        if (is_temp_reg(index_reg))
        {
            regstack_push(index_reg);
        }
        index_reg = tmp_reg;
    }
    std::cout << std::setw(6) << "lsl" << index_reg << ", " << index_reg << ", #2" << std::endl;

    // 计算地址
    add_print(reg, src_reg, 0, index_reg, false,"");

    // 写回内存并释放寄存器
    if (is_temp_reg(reg))
    {
        str_print(reg, sp_reg, stack_map[value], "", 1);
        regstack_push(reg);
    }
    if (is_temp_reg(src_reg))
    {
        regstack_push(src_reg);
    }
    if (is_temp_reg(index_reg))
    {
        regstack_push(index_reg);
    }
}

int cal_array_length(const koopa_raw_type_kind &base)
{
    if (base.tag == KOOPA_RTT_INT32 || base.tag == KOOPA_RTT_FLOAT32)
    {
        return 1;
    }
    else
    {
        int length = 0;
        length = cal_array_length(*base.data.array.base) * base.data.array.len;
        return length;
    }
}

int cal_aggregate_length(const koopa_raw_value_t &value)
{
    if (value->kind.tag == KOOPA_RVT_AGGREGATE)
    {
        int length = 0;
        for (size_t i = 0; i < value->kind.data.aggregate.elems.len; i++)
        {
            length += cal_aggregate_length((koopa_raw_value_t)(value->kind.data.aggregate.elems.buffer[i]));
        }
        return length;
    }
    return SYSY_VALUE_SIZE;
}
// needs more instur

void analysis_aggregate(const koopa_raw_aggregate_t &aggregate, std::list<int32_t> &init_list)
{
    for (size_t i = 0; i < aggregate.elems.len; i++)
    {
        koopa_raw_value_t value = (koopa_raw_value_t)(aggregate.elems.buffer[i]);
        if (value->kind.tag == KOOPA_RVT_INTEGER)
        {
            init_list.push_back(value->kind.data.integer.value);
        }
        else
        {
            analysis_aggregate(value->kind.data.aggregate, init_list);
        }
    }
}
void analysis_float_aggregate(const koopa_raw_aggregate_t &aggregate, std::list<float> &init_list)
{
    for (size_t i = 0; i < aggregate.elems.len; i++)
    {
        koopa_raw_value_t value = (koopa_raw_value_t)(aggregate.elems.buffer[i]);
        if (value->kind.tag == KOOPA_RVT_FLOATNUM)
        {
            init_list.push_back(value->kind.data.floatnum.value);
        }
        else
        {
            analysis_float_aggregate(value->kind.data.aggregate, init_list);
        }
    }
}

void store_array(const koopa_raw_value_t &value, std::string &base_reg, int array_type, int available_regs)
{
    int uses_of_reserved_regs = 0;
    int elem_type;
    for (int i = 0; i < value->kind.data.aggregate.elems.len; i++)
    {
        const koopa_raw_value_t &elem = (koopa_raw_value_t)(value->kind.data.aggregate.elems.buffer[i]);
        if (elem->kind.tag != KOOPA_RVT_AGGREGATE) // 说明当前AGGREGATE已经是一维了
        {
            /*异常处理*/
            elem_type = get_value_type(elem->ty);
            /*异常处理*/
            if (elem->kind.tag == KOOPA_RVT_INTEGER)
            {
                if (array_type == INT_TYPE)
                {
                    mov_print('x' + std::to_string(uses_of_reserved_regs++ + w_regs_for_arg), elem->kind.data.integer.value, "", true, "");
                }
                else
                {
                    fmov_print('s' + std::to_string(uses_of_reserved_regs++ + s_regs_for_arg), elem->kind.data.integer.value, "", true, "");
                }
            }
            else if (elem->kind.tag == KOOPA_RVT_FLOATNUM)
            {
                if (array_type == FLOAT_TYPE)
                {
                    fmov_print('s' + std::to_string(uses_of_reserved_regs++ + s_regs_for_arg), elem->kind.data.floatnum.value, "", true, "");
                }
                else
                {
                    mov_print('x' + std::to_string(uses_of_reserved_regs++ + w_regs_for_arg), elem->kind.data.floatnum.value, "", true, "");
                }
            }
            else // elem是变量
            {
                if (elem_type != array_type)
                {
                    std::cerr << "array type is not match" << std::endl;
                    assert(false);
                }
                std::string src_reg;
                std::string tmp_reg;
                if (array_type == INT_TYPE)
                {
                    tmp_reg = 'x' + std::to_string(uses_of_reserved_regs++ + w_regs_for_arg);
                }
                else
                {
                    tmp_reg = 's' + std::to_string(uses_of_reserved_regs++ + s_regs_for_arg);
                }
                // 由于前面的异常检测，此处elem的类型和数组类型必然一致
                // elem被分配了寄存器
                auto it = reg_alloc_map.find(elem);
                // 把值存入临时寄存器
                if (it != reg_alloc_map.end())
                {
                    src_reg = (*it).second;
                    mov_between_any_regs(tmp_reg, src_reg);
                }
                else if (stack_map[elem] >= 0) // elem是存储在栈中的变量
                {
                    ldr_print(tmp_reg, sp_reg, stack_map[elem], "", 1);
                }
                else // elem是用寄存器传递的参数
                {
                    if (elem_type == INT_TYPE)
                    {
                        src_reg = 'x' + std::to_string(stack_map[elem] + in_r0_r7);
                    }
                    else
                    {
                        src_reg = 's' + std::to_string(stack_map[elem] + in_s0_s7);
                    }
                    mov_between_any_regs(tmp_reg, src_reg);
                }
            }
            if (uses_of_reserved_regs == available_regs)
            {
                uses_of_reserved_regs = 0;
                if (array_type == INT_TYPE)
                {
                    // 获取需要保存的寄存器范围
                    int start_reg = w_regs_for_arg;
                    int end_reg = available_regs + w_regs_for_arg - 1;
                    int reg_count = end_reg - start_reg + 1;

                    // 按对保存寄存器（stp指令）
                    for (int i = start_reg; i < end_reg; i += 2) {
                        std::cout << "stp x" << i << ", x" << i+1 << ", [" << base_reg << "], #16" << std::endl;
                    }

                    // 处理可能的单数剩余寄存器（最后一个）
                    if (reg_count % 2 != 0) {
                        std::cout << "str x" << end_reg << ", [" << base_reg << "], #8" << std::endl;
                    }
                }
                else
                {
                    // 获取需要保存的寄存器范围
                    int start_reg = s_regs_for_arg;
                    int end_reg = available_regs + s_regs_for_arg - 1;
                    int reg_count = end_reg - start_reg + 1;

                    // 按对保存寄存器（stp指令）
                    for (int i = start_reg; i < end_reg; i += 2) {
                        std::cout << "stp s" << i << ", s" << i+1 << ", [" << base_reg << "], #16" << std::endl;
                    }

                    // 处理可能的单数剩余寄存器（最后一个）
                    if (reg_count % 2 != 0) {
                        std::cout << "str s" << end_reg << ", [" << base_reg << "], #8" << std::endl;
                    }
                }
            }
        }
        else
        {
            store_array(elem, base_reg, array_type, available_regs);
        }
    }
    if (uses_of_reserved_regs > 1)
    {
        if (array_type == INT_TYPE)
        {
            // 获取需要保存的寄存器范围
            int start_reg = w_regs_for_arg;
            int end_reg = uses_of_reserved_regs + w_regs_for_arg - 1;
            int reg_count = end_reg - start_reg + 1;

            // 按对保存寄存器（stp指令）
            for (int i = start_reg; i < end_reg; i += 2) {
                std::cout << "stp x" << i << ", x" << i+1 << ", [" << base_reg << "], #16" << std::endl;
            }

            // 处理可能的单数剩余寄存器（最后一个）
            if (reg_count % 2 != 0) {
                std::cout << "str x" << end_reg << ", [" << base_reg << "], #8" << std::endl;
            }
        }
        else
        {
            // 获取需要保存的寄存器范围
            int start_reg = s_regs_for_arg;
            int end_reg = uses_of_reserved_regs + s_regs_for_arg - 1;
            int reg_count = end_reg - start_reg + 1;
            // 按对保存寄存器（stp指令）
            for (int i = start_reg; i < end_reg; i += 2) {
                std::cout << "stp s" << i << ", s" << i+1 << ", [" << base_reg << "], #16" << std::endl;
            }

            // 处理可能的单数剩余寄存器（最后一个）
            if (reg_count % 2 != 0) {
                std::cout << "str s" << end_reg << ", [" << base_reg << "], #8" << std::endl;
            }
        }
    }
    else if (uses_of_reserved_regs == 1)
    {
        if (array_type == INT_TYPE)
        {
            std::cout << "str x" << w_regs_for_arg + uses_of_reserved_regs - 1 << ", [" << base_reg << "], #8" << std::endl;
        }
        else
        {
            std::cout << "str s" << s_regs_for_arg + uses_of_reserved_regs - 1 << ", [" << base_reg << "], #8" << std::endl;
        }
    }
}

void store_array(const koopa_raw_value_t &value, const koopa_raw_value_t &dest)
{
    std::string base_reg = 'x' + std::to_string(w_regs_for_arg + w_regs_reserved - 1);
    int array_type = get_value_type(dest->ty);
    int available_regs;
    if (array_type == INT_TYPE)
    {
        available_regs = w_regs_reserved - 1;
    }
    else
    {
        available_regs = s_regs_reserved;
    }
    auto it = reg_alloc_map.find(dest);
    if (it != reg_alloc_map.end())
    {
        mov_print(base_reg, 0, (*it).second, false, "");
    }
    else if (dest->kind.tag == KOOPA_RVT_GLOBAL_ALLOC || stack_map[dest] < 0)
    {
        std::cerr << "try to store array to a global variable or a parameter" << std::endl;
        assert(false);
    }
    else if (stack_map[dest] >= 0)
    {

        add_print(base_reg, sp_reg, stack_map[dest] + 8, "", true,"");
    }
    // store_array的dest不可能是参数或全局变量
    store_array(value, base_reg, array_type, available_regs);
}
// 返回value的type,1是int，2是float，0是例外
int get_value_type(const koopa_raw_type_t &type)
{
    if (type->tag == KOOPA_RTT_INT32)
        return INT_TYPE;
    if (type->tag == KOOPA_RTT_FLOAT32)
        return FLOAT_TYPE;
    if (type->tag == KOOPA_RTT_ARRAY || type->tag == KOOPA_RTT_POINTER)
        return get_value_type(type->data.array.base);
    return 0;
}

int get_param_type(const koopa_raw_type_t &type)
{
    if (type->tag == KOOPA_RTT_INT32)
        return PARAM_INT_TYPE;
    if (type->tag == KOOPA_RTT_FLOAT32)
        return PARAM_FLOAT_TYPE;
    if (type->tag == KOOPA_RTT_POINTER || type->tag == KOOPA_RTT_ARRAY)
        return PARAM_INT_TYPE;
    // std::cout<<"参数类型无法识别"<<std::endl;
    return 1;
}

void regstack_push(std::string reg)
{
    if (reg[0] == 's')
        sreg_stack.push(reg);
    if (reg[0] == 'x')
        reg_stack.push(reg);
}
// 如果flag=2就弹出浮点寄存器，否则弹出通用寄存器
std::string regstack_pop(int flag)
{
    std::string reg;
    if (flag == USE_FLOAT_REG)
    {
        if (sreg_stack.empty())
            assert(false);
        reg = sreg_stack.top();
        sreg_stack.pop();
    }
    else
    {
        if (reg_stack.empty())
            assert(false);
        reg = reg_stack.top();
        reg_stack.pop();
    }
    return reg;
}

// 把一个 立即数/变量 value传入 目的寄存器（整形/浮点型寄存器）
void value_to_reg(std::string dest_reg, const koopa_raw_value_t &src, bool src_is_param)
{
    int src_type;
    src_is_param = (src->kind.tag == KOOPA_RVT_FUNC_ARG_REF);
    if (src_is_param)
        src_type = get_param_type(src->ty);
    else
        src_type = get_value_type(src->ty);
    bool is_const = (src->kind.tag == KOOPA_RVT_INTEGER || src->kind.tag == KOOPA_RVT_FLOATNUM);
    if (dest_reg[0] == 's') // 目的类型是浮点型
    {
        if (is_const) // src是常量
            if (src_type != 2)
            {
                std::string rr=reg_stack.top();
                reg_stack.pop();
                mov_print(rr, src->kind.data.integer.value, "", true, "");
                fmov_print(dest_reg, 0, rr, false, "");
                fcvt_print(dest_reg, dest_reg, false);
                // vmov_print(dest_reg, float(src->kind.data.integer.value), "", true, "");
                // vcvt_print(dest_reg, dest_reg, false);
                reg_stack.push(rr);
            }
            else
            {
                fmov_print(dest_reg, src->kind.data.floatnum.value, "", true, "");
                // vcvt_print(dest_reg, dest_reg, false);
            }
        else // src是变量
        {
            if (src_type != 2)
            {
                if (stack_map[src] >= 0) // src在栈帧中
                {
                    std::string rr=reg_stack.top();
                    reg_stack.pop();
                    ldr_print(rr, sp_reg, stack_map[src], "", 1);
                    fmov_print(dest_reg, 0, rr, false, "");
                    fcvt_print(dest_reg, dest_reg, false);
                    reg_stack.push(rr);
                }
                else
                {
                    fmov_print(dest_reg, 0, "x" + std::to_string(stack_map[src] + in_r0_r7), false, "");
                    fcvt_print(dest_reg, dest_reg, false);
                }
            }
            else // 浮点变量到dest
            {
                if (stack_map[src] >= 0) // src在栈帧中
                    ldr_print(dest_reg, sp_reg, stack_map[src], "", 1);
                else
                    fmov_print(dest_reg, 0, "s" + std::to_string(stack_map[src] + in_s0_s7), false, "");
            }
        }
    }
    else if (dest_reg[0] == 'x') // dest是整形
    {
        if (is_const) // src是常量
            if (src_type != 2)
                mov_print(dest_reg, src->kind.data.integer.value, "", true, "");
            else
                mov_print(dest_reg, int(src->kind.data.floatnum.value), "", true, "");
        else // src是变量
        {
            if (src_type != 2) // int变量到dest_reg
            {
                if (stack_map[src] >= 0) // src在栈帧中
                    ldr_print(dest_reg, sp_reg, stack_map[src], "", 1);
                else
                    mov_print(dest_reg, 0, "x" + std::to_string(stack_map[src] + in_r0_r7), false, "");
            }
            else // 浮点变量到dest
            {
                if (stack_map[src] >= 0) // src在栈帧中
                {
                    std::string rr=sreg_stack.top();
                    sreg_stack.pop();
                    ldr_print(rr, sp_reg, stack_map[src], "", 1);
                    fcvt_print(rr, rr, true);
                    fmov_print(dest_reg, 0, rr, false, "");
                    sreg_stack.push(rr);
                }
                else
                {
                    fcvt_print("s" + std::to_string(stack_map[src] + in_s0_s7), "s" + std::to_string(stack_map[src] + in_s0_s7), true);
                    fmov_print(dest_reg, 0, "s" + std::to_string(stack_map[src] + in_s0_s7), false, "");
                }
            }
        }
    }
}

bool is_temp_reg(const std::string &reg)
{
    if (reg.empty())
        return false;
    int number = std::stoi(reg.substr(1));
    if (reg[0] == 'x' && number >= w_regs_for_arg && number < w_regs_for_arg + w_regs_reserved)
        return true;
    if (reg[0] == 's' && number >= s_regs_for_arg && number < s_regs_for_arg + s_regs_reserved)
        return true;
    return false;
}

// 处理操作数，NOT CHECKED
std::string prep_operand(const koopa_raw_value_t &operand, std::string dest_reg, const koopa_raw_value_t &value)
{
    std::string reg;
    int type = (operand->ty->tag == KOOPA_RTT_FLOAT32) ? FLOAT_TYPE : INT_TYPE;
    // std::cerr << "type: " << type << std::endl;
    bool is_imm = (operand->kind.tag == KOOPA_RVT_INTEGER || operand->kind.tag == KOOPA_RVT_FLOATNUM);
    // 不是立即数
    if (!is_imm)
    {
        /// 已被分配寄存器
        if (reg_alloc_map.find(operand) != reg_alloc_map.end())
        {
            std::string reg = reg_alloc_map[operand];
            update_reg_value_map(operand, reg);
            return reg;
        }

        /// 全局变量
        if (operand->kind.tag == KOOPA_RVT_GLOBAL_ALLOC)
        {
            // 三个条件成立才能直接用dest_reg来存GLOBAL_ALLOC：
            // 1. dest_reg是r型寄存器
            // 2. dest_reg_used为false，即没有用来存前一个操作数
            // 3. dest_reg已经被用于存储结果，可以用来存储当前操作数
            if (dest_reg[0] == 'x' &&
                !dest_reg_used &&
                w_reg_value_map[std::stoi(dest_reg.substr(1))] == value)
            {
                reg = dest_reg;
                dest_reg_used = true;
            }
            else
                reg = regstack_pop(USE_INT_REG);
            gloabal_var_load(reg, operand->name + 1);
            return reg;
        }

        /// 未被分配寄存器的变量 或 不是用寄存器传递的参数
        int offset = stack_map[operand];
        if (offset >= 0)
        {
            // 能够使用dest_reg的条件：
            //  1. dest_reg_used为false，即没有用来存前一个操作数
            //  2. dest_reg的寄存器类型和操作数类型一致
            //  3. dest_reg已经被用于存储结果，可以用来存储当前操作数
            if (!dest_reg_used)
            {
                if (dest_reg[0] == 'x' && type == INT_TYPE &&
                    (w_reg_value_map[std::stoi(dest_reg.substr(1))] == value))
                {
                    reg = dest_reg;
                    dest_reg_used = true;
                }
                else if (dest_reg[0] == 's' && type == FLOAT_TYPE &&
                         (s_reg_value_map[std::stoi(dest_reg.substr(1))] == value))
                {
                    reg = dest_reg;
                    dest_reg_used = true;
                }
                else
                {
                    reg = regstack_pop(type);
                }
            }
            else
                reg = regstack_pop(type);
            ldr_print(reg, sp_reg, offset, "", 1);
        }

        /// 用寄存器传递的参数
        else
        {
            if (type == INT_TYPE)
                reg = "x" + std::to_string(offset + in_r0_r7);
            else
                reg = "s" + std::to_string(offset + in_s0_s7);
        }
    }

    // 是立即数，只为操作数分配临时寄存器，但不进行mov操作
    else
    {
        if (type == INT_TYPE)
        {
            if (operand->kind.data.integer.value == 0)
                return zero_reg;
            // 操作数非0
            // 能够使用dest_reg的条件：
            //  1. dest_reg_used为false，即没有用来存前一个操作数
            //  2. dest_reg的寄存器类型和操作数类型一致
            //  3. dest_reg已经被用于存储结果，可以用来存储当前操作数
            if (!dest_reg_used &&
                dest_reg[0] == 'x' &&
                w_reg_value_map[std::stoi(dest_reg.substr(1))] == value)
            {
                reg = dest_reg;
                dest_reg_used = true;
            }
            else
                reg = regstack_pop(type);
        }
        else
        {
            if (operand->kind.data.floatnum.value == 0.0f)
                return zero_reg_f;
            // 操作数非0
            // 能够使用dest_reg的条件：
            //  1. dest_reg_used为false，即没有用来存前一个操作数
            //  2. dest_reg的寄存器类型和操作数类型一致
            //  3. dest_reg已经被用于存储结果，可以用来存储当前操作数
            if (!dest_reg_used &&
                dest_reg[0] == 's' &&
                s_reg_value_map[std::stoi(dest_reg.substr(1))] == value)
            {
                reg = dest_reg;
                dest_reg_used = true;
            }
            else
                reg = regstack_pop(type);
        }
    }
    return reg;
}

// 整型寄存器到浮点寄存器的转换
std::string int2float_reg(int type, bool is_imm, float float_num, std::string &i_reg)
{
    std::string f_reg;
    if (type == INT_TYPE) // lhs_reg是一个整型寄存器
    {
        f_reg = regstack_pop(USE_FLOAT_REG);
        if (is_temp_reg(i_reg))
        {
            regstack_push(i_reg);
        }
        if (is_imm)
        {
            if (float_num == 0.0f)
            {
                if (is_temp_reg(f_reg))
                {
                    regstack_push(f_reg);
                }
                return zero_reg_f;
            }
            fmov_print(f_reg, float_num, "", true, "");
        }
        else
        { // lhs_reg中存了lhs的值
            fmov_print(f_reg, 0, i_reg, false, "");
            fcvt_print(f_reg, f_reg, false);
        }
    }
    else
    {
        if (is_imm)
        {
            if (float_num == 0.0f)
            {
                if (is_temp_reg(i_reg))
                {
                    regstack_push(i_reg);
                }
                return zero_reg_f;
            }
            fmov_print(i_reg, float_num, "", true, "");
        }
        return i_reg;
    }
    return f_reg;
}

// 把立即数放到寄存器里面
void imm2reg(const koopa_raw_value_t &value, std::string &dest_reg)
{
    if (value->kind.tag == KOOPA_RVT_INTEGER && dest_reg != zero_reg)
    {
        mov_print(dest_reg, value->kind.data.integer.value, "", true, "");
    }
    else if (value->kind.tag == KOOPA_RVT_FLOATNUM && dest_reg != zero_reg_f)
    {
        fmov_print(dest_reg, value->kind.data.floatnum.value, "", true, "");
    }
}

void store_regs()
{
    w_reg_stack_base_offset = -size_of_stack_frame - FRAME_SIZE_OF_R_REGS;
    s_reg_stack_base_offset = -size_of_stack_frame - FRAME_SIZE_OF_S_REGS - FRAME_SIZE_OF_R_REGS;
    memset(w_reg_used, 0, sizeof(w_reg_used));
    memset(s_reg_used, 0, sizeof(s_reg_used));
    bool w_used = false;
    bool s_used = false;
    /// 记录哪些寄存器被分配
    int cnt = 0;
    for (auto &pair : reg_alloc_map)
    {
        std::string reg = pair.second;
        if (reg.empty())
        {
            // std::cerr<<"reg is empty"<<std::endl;
            // assert(false);
            cnt++;
            continue;
        }
        if (reg[0] == 'x')
        {
            w_reg_used[std::stoi(reg.substr(1))] = true;
            w_used = true;
        }
        else
        {
            s_reg_used[std::stoi(reg.substr(1))] = true;
            s_used = true;
        }
    }

    /// 保存被分配的寄存器
    std::string base_reg = reg_stack.top();
    reg_stack.pop();
    add_print(base_reg, sp_reg, w_reg_stack_base_offset, "", true,"");
    for (int i = w_regs_for_arg + w_regs_reserved + zero_w_reg_num; i < w_regs_num; ++i)
    {
        if (w_reg_used[i])
        {
            str_print("x" + std::to_string(i), base_reg, 8 * i, "", 1);
        }
    }
    if (s_used)
    {
        add_print(base_reg, base_reg, -FRAME_SIZE_OF_S_REGS, "", true,"");
        for (int i = s_regs_for_arg + s_regs_reserved + zero_s_reg_num; i < s_regs_num; ++i)
        {
            if (s_reg_used[i])
            {
                str_print("s" + std::to_string(i), base_reg, 8 * i, "", 1);
            }
        }
    }
    reg_stack.push(base_reg);
}

void restore_regs()
{
    std::string base_reg = reg_stack.top();
    reg_stack.pop();
    add_print(base_reg, sp_reg, w_reg_stack_base_offset, "", true,"");
    for (int i = w_regs_for_arg + w_regs_reserved + zero_w_reg_num; i < w_regs_num; ++i)
    {
        if (w_reg_used[i])
        {
            ldr_print("x" + std::to_string(i), base_reg, 8 * i, "", 1);
        }
    }
    add_print(base_reg, base_reg, -FRAME_SIZE_OF_S_REGS, "", true,"");
    for (int i = s_regs_for_arg + s_regs_reserved + zero_s_reg_num; i < s_regs_num; ++i)
    {
        if (s_reg_used[i])
        {
            ldr_print("s" + std::to_string(i), base_reg, 8 * i, "", 1);
        }
    }
    reg_stack.push(base_reg); 
}

void update_reg_value_map(const koopa_raw_value_t &value, std::string reg)
{
    if (reg.empty())
    {
        std::cerr << "reg is empty" << std::endl;
        assert(false);
    }
    int reg_num = std::stoi(reg.substr(1));
    // 只更新用于分配的寄存器
    if (reg[0] == 'x')
    {
        if (reg_num >= w_regs_for_arg + w_regs_reserved + zero_w_reg_num && reg_num < w_regs_num)
        {
            w_reg_value_map[reg_num] = value;
        }
    }
    else
    {
        if (reg_num >= s_regs_for_arg + s_regs_reserved + zero_s_reg_num && reg_num < s_regs_num)
        {
            s_reg_value_map[reg_num] = value;
        }
    }
}

void mov_between_any_regs(std::string dest_reg, std::string src_reg)
{
    if (src_reg.empty() || dest_reg.empty())
    {
        std::cerr << "reg is empty string" << std::endl;
        assert(false);
    }
    if (src_reg == dest_reg)
    {
        return;
    }
    if (src_reg[0] == 'x' && dest_reg[0] == 'x')
    {
        mov_print(dest_reg, 0, src_reg, false, "");
    }
    else if (src_reg[0] == 's' && dest_reg[0] == 's')
    {
        fmov_print(dest_reg, 0, src_reg, false, "");
    }
    else if (src_reg[0] == 's' && dest_reg[0] == 'x')
    {
        fcvt_print(src_reg, src_reg, true);
        fmov_print(dest_reg, 0, src_reg, false, "");
    }
    else if (src_reg[0] == 'x' && dest_reg[0] == 's')
    {
        fmov_print(dest_reg, 0, src_reg, false, "");
        fcvt_print(dest_reg, dest_reg, false);
    }
}

void str_print(int dest_type, int src_type, std::string dest_reg, std::string reg)
{
    // 如果value和dest的类型不同，需要类型转换
    if (src_type != dest_type)
    {
        std::string tmp_reg;
        if (dest_type == INT_TYPE)
        { // dest是int型，src是float型，会被改变，需要进行保护
            if (is_temp_reg(reg))
            {
                tmp_reg = reg;
            }
            else
            {
                tmp_reg = regstack_pop(src_type);
                mov_between_any_regs(tmp_reg, reg);
            }
            fcvt_print(tmp_reg, tmp_reg, true);
        }
        else
        { // dest是float型，src是int型，不会被改变
            tmp_reg = regstack_pop(dest_type);
            fmov_print(tmp_reg, 0, reg, false, "");
            fcvt_print(tmp_reg, tmp_reg, false);
        }
        str_print(tmp_reg, dest_reg, 0, "", 1);
        if (is_temp_reg(tmp_reg))
        {
            regstack_push(tmp_reg);
        }
    }
    else
    {
        str_print(reg, dest_reg, 0, "", 1);
    }
}

void ldr_print(int value_type, int mem_type, std::string src_reg, std::string dest_reg)
{
    // src_reg是地址量
    // dest_reg是存放load结果的寄存器
    if (value_type != mem_type)
    {
        std::string tmp_reg;

        if (value_type == INT_TYPE) // mem_type = FLOAT_TYPE
        {
            //%1 = load @a，%1是整型，@a存的是浮点型
            /// 先把值load到临时寄存器
            tmp_reg = regstack_pop(mem_type);
            ldr_print(tmp_reg, src_reg, 0, "", 1);
            /// 类型转换
            fcvt_print(tmp_reg, tmp_reg, true);
            /// 把类型转换的结果放入dest_reg
            fmov_print(dest_reg, 0, tmp_reg, false, "");
            regstack_push(tmp_reg);
        }
        else // value_type = FLOAT_TYPE, mem_type = INT_TYPE
        {
            // dest_reg是S型寄存器，可以直接vcvt，就不需要临时寄存器
            /// 把值load到dest_reg
            ldr_print(dest_reg, src_reg, 0, "", 1);
            /// 类型转换
            fcvt_print(dest_reg, dest_reg, false);
        }
    }
    else
    {
        ldr_print(dest_reg, src_reg, 0, "", 1);
    }
}
int getPowerOfTwo(int n)
{
    if (n > 0 && (n & (n - 1)) == 0)
    {
        // __builtin_ctz 返回 n 的二进制表示中从右边开始连续的 0 的个数
        return __builtin_ctz(n);
        // 或者使用 log2
        // return static_cast<int>(std::log2(n));
    }
    // 如果不是 2 的幂次，返回 -1
    return -1;
}
//判断是否符合算术运算的立即数标准
bool isopimm(int64_t imm, bool is64bit) {
    // 立即数的绝对值
    uint64_t abs_imm = imm < 0 ? static_cast<uint64_t>(-imm) : static_cast<uint64_t>(imm);
    
    // 检查是否在0-4095范围内 
    if (abs_imm <= 0xFFF) return true;
    
    // 检查是否在移位后的范围内 (imm << 12)
    if (is64bit) {
        // 64位支持移位后的立即数
        if ((abs_imm & 0xFFF) == 0) {
            uint64_t shifted = abs_imm >> 12;
            if (shifted <= 0xFFF) return true;
        }
    }
    
    return false;
}
//是否符合逻辑运算的立即数标准
bool islogicimm(uint64_t imm) {
    const uint64_t target_nums[] = {0x00000000000ffff0, 0x01fffffffffe0000, 0x7000000000000000, 0xfffffffffffe1fff, 0x0000000ffe000000, 0x7ffffffffffe0000, 0xffff80001fffffff, 0x0000000000002000, 0xffffffc000000003, 0x1f001f001f001f00, 0x000000007f000000, 0x0000000fffe00000, 0x0000000000000ffe, 0x007f0000007f0000, 0x0020000000000000, 0x7fffe00000000000, 0xf0000007ffffffff, 0x03fffffc00000000, 0xfffffffffffff000, 0xfffffff9ffffffff, 0xffffffffc000007f, 0xffffffffffffe01f, 0x0000001ff0000000, 0x000ffffffff80000, 0x7ffffffffffff800, 0x0007fffffff00000, 0xfff0000000000001, 0x000001fffffc0000, 0xf00000000001ffff, 0x0202020202020202, 0xfffffffff00007ff, 0xfffc000000000001, 0x0000003fffffffc0, 0xfc1ffffffc1fffff, 0xff00001fffffffff, 0x0007ffc00007ffc0, 0xffffe000000003ff, 0xff0000000001ffff, 0xffffffffe003ffff, 0xfffe000000000fff, 0xffffc0000000003f, 0xfff001ffffffffff, 0x3f0000003f000000, 0x0000000000fffe00, 0x000000007ffc0000, 0x0000000000040000, 0x003fc000003fc000, 0xffffffffe000ffff, 0xffffe00000000001, 0xffffff00000001ff, 0xfe00000000000fff, 0xc00000000003ffff, 0xfff1fffffff1ffff, 0x000003ff80000000, 0x00ffffffffffffc0, 0x003ffffffffe0000, 0x0fffffffffffffff, 0x3000000030000000, 0x07f007f007f007f0, 0x00003ff000000000, 0x0000000000000038, 0xc7ffffffffffffff, 0x0007fffffffff000, 0x1c1c1c1c1c1c1c1c, 0xffffffff000007ff, 0x0000007ffffff000, 0x01ffffe001ffffe0, 0x0000000000000700, 0xfffffffc00003fff, 0x0007fffffff80000, 0xffffffff00000000, 0xffffff80000000ff, 0xfffffffffc00001f, 0x00000001f0000000, 0xfff8ffffffffffff, 0xf0000000000001ff, 0x00003fffffffc000, 0x000000fc00000000, 0xe0003fffffffffff, 0xfffffffe0001ffff, 0x0000fff800000000, 0xfe3fffffffffffff, 0x1818181818181818, 0x07ffffff00000000, 0x000000000007fffe, 0xfff80003ffffffff, 0xe1ffffffe1ffffff, 0x03ffffffff000000, 0x00000001fffffff0, 0x0000000000300000, 0x0000000003ffff80, 0x7ffffffffffff000, 0xc000000000ffffff, 0xfff0fffffff0ffff, 0x0000001f80000000, 0xff8000000001ffff, 0xfffff7ffffffffff, 0xfff803fffff803ff, 0xfe000000003fffff, 0x07ffffffffffc000, 0x3fff3fff3fff3fff, 0x07ffffffffffffe0, 0x0000000007fffff0, 0x000000000ff00000, 0x0200000000000000, 0x000001ffffffff00, 0x7ffc00007ffc0000, 0x0000000200000002, 0xfffffffbfffffffb, 0xffffff800007ffff, 0xfdffffffffffffff, 0x8007800780078007, 0x01f001f001f001f0, 0x0003fffc00000000, 0xfff00000000007ff, 0x000007ffffffffe0, 0x0007e0000007e000, 0xffffffffffc001ff, 0x1f8000001f800000, 0x00001fc000001fc0, 0x00001ffffffffff8, 0x001ffffffc000000, 0xfffc00000001ffff, 0xc0000000000fffff, 0xffffffffffc0ffff, 0x4000000000000000, 0x03fe000003fe0000, 0x01fffffff0000000, 0xffffff8000000001, 0x0001ff0000000000, 0xe00000000000000f, 0x001ffc00001ffc00, 0xfff0000000003fff, 0x07fffe0007fffe00, 0xffe007ffffffffff, 0x0f00000000000000, 0x007ff800007ff800, 0xdddddddddddddddd, 0x0000001800000018, 0xfff800000000ffff, 0x00000fffffff0000, 0x007f000000000000, 0x07fffe0000000000, 0x7ffffc007ffffc00, 0x0000001fffffffe0, 0x001ffffffffe0000, 0x000000ff00000000, 0xfffffc0000000007, 0x000000000fc00000, 0xfffc00000000ffff, 0xffffc000007fffff, 0x00000001ffffc000, 0xfffffefffffffeff, 0x000000000001c000, 0x00000007f8000000, 0xfffffffffc0007ff, 0xffffff80003fffff, 0xffff8003ffffffff, 0x03fffff000000000, 0x0fffffc00fffffc0, 0xc01fffffffffffff, 0xfffffffffffbffff, 0x07fffffffffc0000, 0xffffe003ffffffff, 0xfffffffffff807ff, 0x00000000fff80000, 0xfffffffffffc0003, 0xffffffe00000ffff, 0xfffffffffdffffff, 0xfff00007fff00007, 0x00003fffffffe000, 0xfffc00000007ffff, 0xffffff0003ffffff, 0x0000000ffc000000, 0x0003fffffffc0000, 0x0000003ff0000000, 0xf000000000007fff, 0x7fffffff00000000, 0x0fffffc000000000, 0xfffffffff8000001, 0xfffff00000000003, 0x1fffffffe0000000, 0xff03ff03ff03ff03, 0x00001fffffff0000, 0x000000000001fffc, 0xfffffffffffcffff, 0x00000f0000000f00, 0x000007fffffe0000, 0xffffffffffffffc1, 0x000000003f000000, 0xfff00003ffffffff, 0x7fc000007fc00000, 0xffffffe0000fffff, 0xfffffffff01fffff, 0x003fffffffffff00, 0x000003fff0000000, 0xffe0007fffe0007f, 0xffffc1ffffffc1ff, 0x0000003c0000003c, 0x0ffffffffc000000, 0xfffffffffff0ffff, 0x000001fffffffff8, 0x00000000003fff80, 0xffff000000000000, 0x0007ffffffffffc0, 0xf03ffffff03fffff, 0x00e0000000e00000, 0x000001ffff000000, 0xfffff80000001fff, 0xfffff807fffff807, 0xfffffffff800007f, 0xffffc0ffffffffff, 0x0000000000000ff8, 0x6000000060000000, 0xff0000003fffffff, 0x000ffffffc000000, 0xfffc00000003ffff, 0xffff8001ffffffff, 0x07ffffe000000000, 0x0380000003800000, 0x07ffe00007ffe000, 0x3f8000003f800000, 0x00000000000ffc00, 0x0000000fffffffe0, 0xfe00003ffe00003f, 0xfffffff83fffffff, 0x0000000600000000, 0x1111111111111111, 0xffdfffdfffdfffdf, 0x3fffc0003fffc000, 0x0001ff8000000000, 0xf80000000fffffff, 0xfffe0ffffffe0fff, 0xfff8001fffffffff, 0xffffffe01fffffff, 0xffc00007ffffffff, 0xfffff00000000001, 0xfffffffffe007fff, 0x00fffffe00000000, 0x000000000000000e, 0x8000000000ffffff, 0xff80001fff80001f, 0x00fffffc00fffffc, 0x001fffffe0000000, 0xfffffffffc1fffff, 0x00000000fe000000, 0x000000000003f800, 0x0007fffffffffffc, 0x0000000c00000000, 0xfffffffff80001ff, 0x07ffc00000000000, 0x3fff80003fff8000, 0x00001ffc00000000, 0x03fffffffffc0000, 0x1ffffff01ffffff0, 0xffffffffc0007fff, 0x0007ffffffe00000, 0xf00000000003ffff, 0x07ffff8000000000, 0xfffffffffffffc01, 0xffc00000000fffff, 0xffffffffffffbfff, 0xfffffc0003ffffff, 0x000007fc00000000, 0x80007fff80007fff, 0x01e0000001e00000, 0x000001fc00000000, 0xfffffffffffffe0f, 0x000000001ffff000, 0xfffbfffbfffbfffb, 0x0000003800000038, 0xffffffffff01ffff, 0xfffffffff87fffff, 0x7fff00007fff0000, 0x000007fffffffff0, 0xffffe00000007fff, 0xe00000000001ffff, 0xfffffffffe00007f, 0xffffe0000003ffff, 0xfffffe07fffffe07, 0x00001c0000000000, 0xff800000000fffff, 0x00000000ffffff00, 0x00007fffffc00000, 0xc0000001ffffffff, 0x000000000001e000, 0xfc0007ffffffffff, 0xfe001ffffe001fff, 0x003ffff800000000, 0xf800000000000001, 0xfffe000fffffffff, 0xffffff80000007ff, 0xffe00000000000ff, 0x0000000000060000, 0x0000007fffffff00, 0x3ffffc003ffffc00, 0xffff80000000000f, 0xdfdfdfdfdfdfdfdf, 0x01ffffffff800000, 0x01c0000000000000, 0x0003ffffffffc000, 0xf83ffffff83fffff, 0x0ffffffffffffc00, 0xf807ffffffffffff, 0xffffe001ffffe001, 0x01c001c001c001c0, 0x0000007fffffffe0, 0xff80007fffffffff, 0x000000007ffe0000, 0x007ffff0007ffff0, 0xfffffffffff0001f, 0x1ff81ff81ff81ff8, 0xffffe00001ffffff, 0xfffc07ffffffffff, 0xfffff80000000000, 0x003fe00000000000, 0xfffffffff0ffffff, 0x001ffffe00000000, 0x003fffffffffc000, 0x00000001ffffe000, 0x003ff000003ff000, 0xf8000007ffffffff, 0x00000001ffffffff, 0x0000000040000000, 0x07fffffffff80000, 0xf80000000003ffff, 0xfe0001fffe0001ff, 0xfffffff80007ffff, 0x0000000003c00000, 0x00007e0000000000, 0xfff8000007ffffff, 0xffe7ffffffe7ffff, 0xffffff80000003ff, 0xffff000000000001, 0x0000000000fff000, 0xffffc3ffffffc3ff, 0xc03fffffc03fffff, 0xff07ffffffffffff, 0x00ffffffff000000, 0x0000000000000010, 0x00ffffffffffff00, 0x1ffffe001ffffe00, 0x807fffff807fffff, 0x4040404040404040, 0xfffffff8000007ff, 0xffffffffffffc3ff, 0x00001ff800001ff8, 0xe0000000e0000000, 0xffffffffff80000f, 0xffe0000000000000, 0xffffffffbfffffff, 0xffff001fffff001f, 0x0000040000000000, 0xffffffff00000fff, 0x0000700000000000, 0xff80001fffffffff, 0xe0000000001fffff, 0x000000000ffff000, 0x80000000000001ff, 0xc007ffffffffffff, 0x001ffc0000000000, 0x00ffffffffffff80, 0xfffffff00000003f, 0x00000003fe000000, 0xcfffffffffffffff, 0xffffffff000001ff, 0x00000000000003f8, 0xfffffff800ffffff, 0x003fffffffe00000, 0x7fffffe07fffffe0, 0x0000006000000000, 0x000000001fff8000, 0x0001ffff0001ffff, 0xff001fffff001fff, 0x0000000001fff000, 0x00000fe000000000, 0xffff00000000ffff, 0x00fffff800000000, 0xff3fff3fff3fff3f, 0x1ff01ff01ff01ff0, 0xffffffffff0000ff, 0x03fffffff8000000, 0x0007ffffffffff00, 0x00000ffffffc0000, 0x000ff00000000000, 0x000000000007ffc0, 0x0003800000000000, 0xff00000000007fff, 0xffff800003ffffff, 0x7fffffffffff8000, 0x7f0000007f000000, 0xffe000000fffffff, 0xfffffffffc3fffff, 0xffc007ffffffffff, 0xf000000001ffffff, 0xfffffffffffe00ff, 0x000fffffff800000, 0x000000003c000000, 0x0003ff8000000000, 0x03ffff8003ffff80, 0xff00000000000003, 0x000000f000000000, 0xfff81ffffff81fff, 0x000000f8000000f8, 0xfe00003fffffffff, 0xffffffffffe007ff, 0xffffffff83ffffff, 0x000000fffe000000, 0x0e000e000e000e00, 0x7fe07fe07fe07fe0, 0xffffffffffffff7f, 0xfffffffffe000000, 0xffffff000003ffff, 0xfff00fffffffffff, 0x3ffc000000000000, 0xffe001ffffe001ff, 0xffffffe00001ffff, 0xf800001fffffffff, 0x00000000007fc000, 0x0000007000000000, 0x00003ffffffffff8, 0x3fffffffff800000, 0xfc3ffc3ffc3ffc3f, 0x0000000000ffe000, 0x1ffc000000000000, 0xfff80000000fffff, 0xfffffffffffe0003, 0xc01fc01fc01fc01f, 0x0003f8000003f800, 0x00000000003f0000, 0xffffffc000003fff, 0xffffffffffffffcf, 0x0003fe0000000000, 0xffffffffffffffbf, 0xe01fe01fe01fe01f, 0x001fffffffffc000, 0x0000000007ffff00, 0x1fffffffff000000, 0xfffffffff803ffff, 0xfffffff3fffffff3, 0xffff8fffffff8fff, 0xffffffffffff001f, 0xffffff9fffffffff, 0x3fffe00000000000, 0xffffffffe07fffff, 0xfffffff03fffffff, 0x0000000000003e00, 0x00003ffffffffffe, 0xfffffff8001fffff, 0xffc0000000007fff, 0x000001ff000001ff, 0x00000000fffffff0, 0xe1ffffffffffffff, 0x000ffffffff00000, 0xfe7fffffffffffff, 0x0000001fffe00000, 0xff000001ff000001, 0xc00007ffc00007ff, 0x800003ffffffffff, 0x0003ffffffffff00, 0xffffffffffffff3f, 0xf00ffffff00fffff, 0x0600060006000600, 0x003c000000000000, 0x0000fffffffffc00, 0xfffffffffc01ffff, 0x01fffc0001fffc00, 0xff01ffffffffffff, 0xfff3ffffffffffff, 0x0000003fffffffe0, 0xf800000000003fff, 0x00000000001fff80, 0xfffc0000000003ff, 0x000000fffc000000, 0xffffff00000000ff, 0xffffffe3ffffffff, 0x00003fc000000000, 0x0000ffffffffff00, 0x0fe0000000000000, 0xf0000000000007ff, 0x7ffffff800000000, 0x000003fffffffffc, 0x00ff000000000000, 0x000000007fc00000, 0xfffffffffffff003, 0x03fff00000000000, 0x01ff000000000000, 0x001fc00000000000, 0xfc0ffc0ffc0ffc0f, 0x1800180018001800, 0xfffffe03ffffffff, 0x0780000000000000, 0x003fffffffff8000, 0x00007ff000007ff0, 0xf800003ff800003f, 0xfffffffffe000001, 0x1fffffffffffe000, 0xfffffffffffff1ff, 0xfffff000003fffff, 0x3333333333333333, 0x000000003fc00000, 0xffffffff03ffffff, 0xe1e1e1e1e1e1e1e1, 0xdfffffffdfffffff, 0xfffffffe000007ff, 0x3fe000003fe00000, 0x0001c00000000000, 0xfffffffff000000f, 0xbfbfbfbfbfbfbfbf, 0xffffff000000007f, 0x00000fffffffff80, 0xffff800000000007, 0xfffffffffc007fff, 0x000003fffff00000, 0x0000001c00000000, 0xffe0000000ffffff, 0xffe0003fffe0003f, 0x00000ffffffffe00, 0x0003fff00003fff0, 0xffc007ffffc007ff, 0xfffff8000001ffff, 0x1ffffffffffe0000, 0xffc0ffffffc0ffff, 0x00000000003fff00, 0x00000007ffffffff, 0x00fffe0000000000, 0x0006000000060000, 0x0001ffffffffc000, 0x001ffffe001ffffe, 0xfffffc000007ffff, 0xffff00003fffffff, 0xfff000000000ffff, 0xe3ffe3ffe3ffe3ff, 0xfffffffe000fffff, 0x0007ffffff000000, 0x000001ffc0000000, 0x0000780000000000, 0x00001e0000001e00, 0x00000ff800000ff8, 0x03fffe0003fffe00, 0xff80000fff80000f, 0x00000000003ffffe, 0xfffffffffe3fffff, 0x1fffffc01fffffc0, 0x000000000000001c, 0xffffffffffff8003, 0x000fffffc0000000, 0x01f801f801f801f8, 0xf000f000f000f000, 0x0000000003fff000, 0xffffffffe00003ff, 0x00001ffffffff000, 0x000000000003fff0, 0x0001fffffffffff8, 0x0000007ffe000000, 0xf803ffffffffffff, 0x003ffff000000000, 0x00000000001ffffc, 0x000fffffffe00000, 0xc3ffffffc3ffffff, 0x000003f000000000, 0x00007ffff8000000, 0xfe000001ffffffff, 0x0ff80ff80ff80ff8, 0xffffffff8003ffff, 0x1ffffffffffffe00, 0xf800007fffffffff, 0xfffffffffffffc0f, 0x0000000000000fe0, 0x00007fc000007fc0, 0xffffffe3ffffffe3, 0x000000003ffc0000, 0xffffc001ffffffff, 0x00ffffc000000000, 0xfe00007ffe00007f, 0xfe07fffffe07ffff, 0x000000fffffffc00, 0xfffffff800001fff, 0xffff3fffffff3fff, 0x000000000001f800, 0xff03ffffffffffff, 0x0003fffffe000000, 0x0ffffffffffffff8, 0x00fffffffffffffc, 0xfe0007fffe0007ff, 0xfffe0000000007ff, 0x07fffffffe000000, 0x0000fffc00000000, 0x1f0000001f000000, 0xffffff03ffffff03, 0x000007ffe0000000, 0x0000000ffffffffc, 0x00000ffffc000000, 0xffe000000001ffff, 0xffffffffffff0000, 0x0007ffe00007ffe0, 0x1ffff8001ffff800, 0x000000003fe00000, 0xf800000007ffffff, 0xffffc003ffffc003, 0x001fffffffff0000, 0x3fffc00000000000, 0xc000c000c000c000, 0x000000003ffffff8, 0x00000fc000000fc0, 0xffff1fffffff1fff, 0x0c000c000c000c00, 0x7070707070707070, 0xffffffffc00000ff, 0x0001fffffffffff0, 0xc00000003fffffff, 0x0000000007ff0000, 0xffffe0000001ffff, 0xfffffffffffffe3f, 0x0000000e00000000, 0x03f003f003f003f0, 0x0000000000001e00, 0x000001ffffff0000, 0xffc0000fffc0000f, 0x0000000ffffffffe, 0x001ffffffffff800, 0xfffffffffffc00ff, 0x0000003fc0000000, 0x001f8000001f8000, 0xfffffc000000007f, 0xfff0000000007fff, 0x00007fe000000000, 0x01e001e001e001e0, 0xffffffffffc00001, 0xffc000000000000f, 0xffffffffffffcfff, 0xffff8007ffff8007, 0xf0000001ffffffff, 0x07ff800000000000, 0x000001ffffff8000, 0x80ff80ff80ff80ff, 0xfffffffc00000000, 0xfff00000003fffff, 0xff001fffffffffff, 0x00003fffe0000000, 0x000007ffffffff80, 0x000001c0000001c0, 0xfffff00ffffff00f, 0x00000000000001c0, 0x00000003fffff800, 0xffff3fffffffffff, 0x00007ffe00000000, 0xff800000ffffffff, 0x00000001fff00000, 0x0007fc0000000000, 0x007fffe0007fffe0, 0xffffffc00000ffff, 0xffff8000ffffffff, 0x000000001c000000, 0x00000000000003e0, 0xffc0000000ffffff, 0x1ff0000000000000, 0xfe00000007ffffff, 0xfe0000000007ffff, 0x00000000007fffc0, 0xfffcfffcfffcfffc, 0x80000000001fffff, 0xffe007ffffe007ff, 0x7ff800007ff80000, 0x0fffe0000fffe000, 0x00007ffff0000000, 0xe000000000000007, 0x0000f8000000f800, 0x000000000007ff00, 0xfc03ffffffffffff, 0xfffffffffff800ff, 0xffefffffffffffff, 0xfffffffffff81fff, 0x800000003fffffff, 0x8000000001ffffff, 0x3c0000003c000000, 0x0000ffffffe00000, 0x001ffffffe000000, 0xffffffffff003fff, 0xfffff0000007ffff, 0x0001ffc000000000, 0xffffffff80001fff, 0xe00000000000003f, 0xe007ffffe007ffff, 0xfffffff000000003, 0x00000007fffffffe, 0xfffff8000000003f, 0x0000003fffffe000, 0xfffc0000000001ff, 0xffffffffff000fff, 0x00000fe000000fe0, 0xfffffffff1ffffff, 0x00003ffc00000000, 0x2000000020000000, 0xffffffffffbfffff, 0x81ff81ff81ff81ff, 0x000000000e000000, 0xe03fffffe03fffff, 0xffffffff8000003f, 0x000007f8000007f8, 0xfffffffffc001fff, 0x7fffffffffffff00, 0x01fffffffffffff0, 0xfffc003ffffc003f, 0xfffffffe00000003, 0x0000000000780000, 0x000f800000000000, 0xfffffffff800000f, 0x0000000fff000000, 0xff07ff07ff07ff07, 0xfc3ffffffc3fffff, 0xfffffffffffe0000, 0xffff003fffffffff, 0xc000000001ffffff, 0x8003ffff8003ffff, 0xffffffffff81ffff, 0xfff7ffffffffffff, 0x00000000000000ff, 0xf03ff03ff03ff03f, 0xfffc0fffffffffff, 0xffffffffffffc0ff, 0x3ffffff000000000, 0xc00fffffffffffff, 0xfffffffc0000001f, 0x0000007800000000, 0xfc0000000007ffff, 0xfc00003fffffffff, 0xf00000000000003f, 0x00fffffffffff800, 0xfc00000000ffffff, 0x0000ffff80000000, 0x00000000000007ff, 0x000001ff00000000, 0x0000000000f00000, 0xc00000000000000f, 0x0ffe000000000000, 0x00001ff000001ff0, 0x0003000300030003, 0xfcfcfcfcfcfcfcfc, 0x0000000000003c00, 0x003f000000000000, 0x03fffffffffffc00, 0xfffffffffe1fffff, 0x0000003e00000000, 0x000000000000fe00, 0xfffe000000000007, 0xfffffffffff80007, 0x00003fff80000000, 0xffffe0000007ffff, 0x7ffc000000000000, 0xffffffffffffc003, 0x0000007f80000000, 0x3fffffffffff0000, 0x1ffffc0000000000, 0x0003fffffffffffc, 0xffff07ffffff07ff, 0xf800f800f800f800, 0x07ffff0007ffff00, 0xffffc00000007fff, 0x000ffffffe000000, 0xffffffffffffff07, 0x01f8000001f80000, 0x0001fff800000000, 0xfffe007fffffffff, 0x000007e000000000, 0x0003fffffffff000, 0x000007e0000007e0, 0x000000ffffffffff, 0x3fc000003fc00000, 0xffe00000ffe00000, 0xfffffff000001fff, 0xe00fffffe00fffff, 0xc0000007c0000007, 0xfffffff000007fff, 0x03ffe00000000000, 0xfffe0000001fffff, 0x0001fc000001fc00, 0xffffffffff87ffff, 0x800000000003ffff, 0x000000000007ffe0, 0xfff80000003fffff, 0xffff8003ffff8003, 0x07ffffff80000000, 0x0000000007c00000, 0x0000000000003fe0, 0xfffffc0000001fff, 0x0000000000007000, 0x000ffffe00000000, 0x00000003fffffff8, 0x8fffffff8fffffff, 0xffffffffff80003f, 0x01ffffffffffff00, 0xfffffdfffffffdff, 0x000000007ffffffc, 0x3fff00003fff0000, 0xf001ffffffffffff, 0x00000000003fffc0, 0x007ffffc00000000, 0xffbfffffffffffff, 0x8000000080000000, 0x03fffffff0000000, 0xfffffffff80fffff, 0xc003ffffc003ffff, 0xfffff0000fffffff, 0xf80007ffffffffff, 0xffffc01fffffffff, 0x000007fffff80000, 0xffff00000003ffff, 0x0000fffffe000000, 0xcfffcfffcfffcfff, 0x0400000000000000, 0x003c003c003c003c, 0xff80000007ffffff, 0x0fe000000fe00000, 0xffffffffff80ffff, 0x0001ffff00000000, 0x007e0000007e0000, 0xf800003fffffffff, 0xfe0000fffe0000ff, 0xf80000fff80000ff, 0x0040000000400000, 0x03ffffc003ffffc0, 0xffffdfffffffffff, 0xf80ff80ff80ff80f, 0x03ffffff80000000, 0x0001f80000000000, 0x00000001ff800000, 0x003ffffffffffff8, 0x1ff8000000000000, 0x00007fffffffe000, 0x000003ffffff8000, 0xfffff00001ffffff, 0x00ffffffffffe000, 0xffe000000000000f, 0x3fffffff3fffffff, 0xe00003ffffffffff, 0x000ffffffffffffc, 0x0000007ffffff800, 0xffffe00000001fff, 0x0000000000003ff0, 0xfffffffffffff801, 0x7ffffffe7ffffffe, 0xfffbffffffffffff, 0x000000000007c000, 0x0000001fffffff80, 0x0000000000001f80, 0x0000003fe0000000, 0x03fffffffc000000, 0xfffffffffffff83f, 0x0000100000000000, 0xffffffffffff8007, 0x0003ffffffffffc0, 0x03ffffffffffffe0, 0x0000000400000000, 0xe000007fffffffff, 0xfffff00000000fff, 0x000ffffe000ffffe, 0x8888888888888888, 0x000000fffffff000, 0xfffffffffff001ff, 0x0007fffffffffff0, 0x000fffff80000000, 0xfffffff9fffffff9, 0x007fff0000000000, 0xf9fff9fff9fff9ff, 0xfff1fff1fff1fff1, 0xbfffffffffffffff, 0xff007fffffffffff, 0xfffff00000007fff, 0x8787878787878787, 0xfffffffe0fffffff, 0xffff00001fffffff, 0xff800000000003ff, 0x00001ffffc000000, 0x7fc07fc07fc07fc0, 0x001fffffffffff00, 0x00000000000003fe, 0xfffffffe0000000f, 0x000000fffffffe00, 0x007ffffffe000000, 0x03ffffffffc00000, 0xf000000000000001, 0xffffe00fffffe00f, 0x0fffffffffffff00, 0x00007fffffffffff, 0x0000003ffffffffc, 0x0f0000000f000000, 0x0003f00000000000, 0xfe003fffffffffff, 0x00000003e0000000, 0x0000000010000000, 0xffffffffff800003, 0x0000003fff000000, 0xfff007fffff007ff, 0x2000000000000000, 0xfffffffffffe000f, 0xffffff000001ffff, 0xfffe00007fffffff, 0xfffffffffffffff3, 0xfffff001ffffffff, 0xfffffffffff7ffff, 0x003fff00003fff00, 0x003ffffffc000000, 0x0007fffc0007fffc, 0x007ffffffffffe00, 0xffc3ffffffc3ffff, 0x03ffffe003ffffe0, 0xfffffffff80007ff, 0xffffc0000000ffff, 0x000fffff000fffff, 0xffffe1ffffffe1ff, 0x0000ffffff800000, 0xff000000007fffff, 0x000000fffffff800, 0xc1ffffffc1ffffff, 0x00003ffffe000000, 0xfffdfffdfffdfffd, 0xfffffff000ffffff, 0x00003ffffff00000, 0xffff000000003fff, 0xffffffffc000ffff, 0xf0001ffff0001fff, 0x3ffffff83ffffff8, 0xfffffffffe00003f, 0xffc0000003ffffff, 0xffff0000003fffff, 0xffffffffffffc00f, 0xfffffffffffe03ff, 0xfe0000001fffffff, 0x000000000001ffff, 0x07fffff800000000, 0xfffffc0007ffffff, 0xfffff003ffffffff, 0x00003ffff8000000, 0x003fffc000000000, 0xfffffc03fffffc03, 0x0000f80000000000, 0x803fffffffffffff, 0x07ffffe007ffffe0, 0xffffffff8007ffff, 0xffffe000000007ff, 0xe000000001ffffff, 0x01fffffc00000000, 0xfffffffffff00000, 0xfffffffffffffff9, 0x0003fffffffffff8, 0x7ffc7ffc7ffc7ffc, 0x0000000ffff80000, 0x00000000003f8000, 0x0001fffffffff800, 0xfffc0001fffc0001, 0x00c000c000c000c0, 0x800000001fffffff, 0xfffffffffffe07ff, 0x000000fffffffffc, 0x000ffe0000000000, 0xfffff000ffffffff, 0xfffffffc001fffff, 0xe0000007e0000007, 0x800f800f800f800f, 0x07f8000000000000, 0xffffffffffe0007f, 0x0000003ffffffffe, 0xfffffffc003fffff, 0xe000e000e000e000, 0x000000000fffff00, 0x000003fffe000000, 0x000003ffe0000000, 0x07fff80007fff800, 0x0000000300000000, 0xfffffe0001ffffff, 0xfffffff80fffffff, 0x03c003c003c003c0, 0x0fffffff80000000, 0xff07ffffff07ffff, 0x01fffc0000000000, 0xfffffe01ffffffff, 0x1fffffc000000000, 0x00000000fff00000, 0x1ffff0001ffff000, 0xfffffff1ffffffff, 0xffffffe0000003ff, 0x0000003fffff0000, 0xfffffe01fffffe01, 0x07ffffff07ffffff, 0xff9fff9fff9fff9f, 0xffffefffffffffff, 0xffffffffff80001f, 0x00000f8000000f80, 0xffffffe000000001, 0xf87ffffff87fffff, 0xf800000001ffffff, 0x0ffffc0000000000, 0x007fffffffc00000, 0x000000003ff00000, 0x0000007ffc000000, 0xffffffffffe0001f, 0x00001fff80000000, 0xc0c0c0c0c0c0c0c0, 0x7ffe000000000000, 0xffffff81ffffffff, 0xfffff000000fffff, 0x000000000fe00000, 0xf8007ffff8007fff, 0x0fffffffffffff80, 0xffc7ffffffc7ffff, 0xfffffffffffffcff, 0xffffffc0ffffffff, 0xffffffffff8007ff, 0xffc000000000ffff, 0xffffffffff00007f, 0xffe03fffffffffff, 0xffffffc7ffffffff, 0x0000000001ff0000, 0xfffffffffff87fff, 0xffff80000000ffff, 0x0000000000001ff0, 0x0001800000018000, 0xfffffffffffe01ff, 0x0000001800000000, 0xfc00001fffffffff, 0xf000001ff000001f, 0x0000000000400000, 0x0000003e0000003e, 0x0ffffffff0000000, 0x00007ffffffffff8, 0xc000000000000007, 0x0003ffffffffff80, 0x0000300000000000, 0xfff80000000001ff, 0xf80000007fffffff, 0xffffff3fffffff3f, 0xffffff83ffffff83, 0x00000007ffff8000, 0x000000f800000000, 0xffff801fffffffff, 0xcfcfcfcfcfcfcfcf, 0xfffffff00fffffff, 0x000007ff000007ff, 0xe00fffffffffffff, 0xfffe00fffffe00ff, 0x0000000003ffe000, 0xfffffe0000000007, 0xc0000007ffffffff, 0xffffe000001fffff, 0xffffffffffffffe7, 0x0000000000007f80, 0xfffffff800000007, 0xff000003ff000003, 0x01f0000000000000, 0xffffe0ffffffe0ff, 0xffffffffffc01fff, 0xf0fff0fff0fff0ff, 0x0300000000000000, 0x007ff00000000000, 0x0000000001fffe00, 0x00000000000ffe00, 0xffffffffff0003ff, 0x0000700000007000, 0x000000000ffffe00, 0xffffffffffc0000f, 0xff0001ffff0001ff, 0xffffffff800003ff, 0xfffe000000007fff, 0x001fff8000000000, 0x003ffffc00000000, 0x00000003ffff8000, 0x0000fffffc000000, 0xffffff01ffffff01, 0x7c00000000000000, 0x7fffffffffff0000, 0xfffc0000001fffff, 0xfffffff8fffffff8, 0xf000007fffffffff, 0xfff000ffffffffff, 0xff83ff83ff83ff83, 0xffffffff803fffff, 0x000000000001ffe0, 0x4444444444444444, 0x00003c0000000000, 0xfffffffffffc001f, 0x000000007fffffe0, 0x00ff800000ff8000, 0xfff9ffffffffffff, 0x000001f000000000, 0xfff807ffffffffff, 0x03fffc0000000000, 0xfff800001fffffff, 0x0000000007fffffe, 0xfffffffff0000fff, 0x001ffffffff00000, 0xffffffffe000000f, 0x07ffffffffff8000, 0x07fffffffffffc00, 0x0000003000000000, 0xffe00001ffffffff, 0xffffffffffc003ff, 0x007fffffff800000, 0xffff800fffff800f, 0xffc0003fffffffff, 0x0000000000007e00, 0xf80007fff80007ff, 0x0400040004000400, 0xe0000fffffffffff, 0x07fffff807fffff8, 0xfffc0003fffc0003, 0x0300030003000300, 0x00000003ffffffc0, 0x0000003fffffff00, 0xf801f801f801f801, 0xe001ffffffffffff, 0x00007f0000000000, 0xffffffe003ffffff, 0x07fffffffff00000, 0xfffffe000000007f, 0x0000ffffffc00000, 0xfff80000000003ff, 0xffffffffff8003ff, 0xff000000000000ff, 0x0000000000000003, 0xffffffffe03fffff, 0xfffc000000000fff, 0x0f80000000000000, 0xffffffc000001fff, 0x00003f0000003f00, 0xffffffffffff80ff, 0xfffffffc00000fff, 0xcccccccccccccccc, 0x007ff80000000000, 0xe0ffffffffffffff, 0xfff03fffffffffff, 0xffffffffe00000ff, 0x0003ffffffffffe0, 0x000007fffffffc00, 0xff80000000000003, 0xffc1ffc1ffc1ffc1, 0x000000000001ffc0, 0x800000000007ffff, 0xfe0ffffffe0fffff, 0xff83ffffff83ffff, 0xffffe007ffffe007, 0x0ffe0ffe0ffe0ffe, 0xe0000000000001ff, 0xf0000003f0000003, 0x000000007ffff800, 0xffc00fffffc00fff, 0x0001ffffe0000000, 0xff800000000000ff, 0xfffc000000000007, 0x003ffc0000000000, 0x000003ffff800000, 0xfffff8000003ffff, 0x0003800000038000, 0x0fc00fc00fc00fc0, 0x00000000007ffe00, 0xfc7ffc7ffc7ffc7f, 0x001ffffc001ffffc, 0x0000000200000000, 0xfffffffffffff803, 0xfe00000000000007, 0xf8000007f8000007, 0xfffff00000001fff, 0x0007fff80007fff8, 0x0ffc0ffc0ffc0ffc, 0xfffffffff8000003, 0x0007fffe0007fffe, 0xffffffffffc00007, 0xfff3fff3fff3fff3, 0x00fffc0000fffc00, 0xfff8000000000001, 0x000000000003ff00, 0x07fffc0000000000, 0xfff000000007ffff, 0x007c007c007c007c, 0x0003ffffffffe000, 0x0000000007ffffff, 0x0001000100010001, 0xfc0001fffc0001ff, 0xf1ffffffffffffff, 0x0001ffffff000000, 0xc000000000000001, 0xfff8007ffff8007f, 0xf8000000007fffff, 0xffc0007fffffffff, 0x0000001f00000000, 0xfe0ffe0ffe0ffe0f, 0xfff7fff7fff7fff7, 0x03fffffc03fffffc, 0x0007ffff0007ffff, 0xffffffffffc00003, 0xfffffffffffff0ff, 0xfff00000ffffffff, 0xffffc0000000000f, 0xffc0000000000000, 0x00ffffffffff8000, 0x007fffff80000000, 0x000000fff8000000, 0x8003ffffffffffff, 0x0000000000000002, 0xfffc003fffffffff, 0x00000000007ffff0, 0x0000000000fffffc, 0xffffffc003ffffff, 0x00001ffffe000000, 0xf00001ffffffffff, 0x00007fffff000000, 0x00fe000000000000, 0x01ffffff80000000, 0x00000000ffffc000, 0x00000001ffc00000, 0x00000000000fe000, 0xffc003ffffffffff, 0x7fffffffffffe000, 0x07c007c007c007c0, 0x2000200020002000, 0x000001fffc000000, 0x00ffe00000000000, 0x00007ffe00007ffe, 0xfffe0000000003ff, 0x000fc00000000000, 0x01ffc00001ffc000, 0xfc00fc00fc00fc00, 0x000000003fffc000, 0x80000000000fffff, 0x000000001e000000, 0x000c000000000000, 0x9999999999999999, 0x001fffc0001fffc0, 0x0000fffff0000000, 0xfffff000007fffff, 0x003ff00000000000, 0xc000007fc000007f, 0x003fffffff800000, 0xffe001ffffffffff, 0x3800000038000000, 0x000000000001fffe, 0x000001fc000001fc, 0x000ffffffffe0000, 0xf8000000f8000000, 0xe00000ffe00000ff, 0xff000001ffffffff, 0xffffffffffffff81, 0xfffffffc0000007f, 0xffffffffc0000001, 0x000fff0000000000, 0xffffe00fffffffff, 0xfffe0001fffe0001, 0xffff8fffffffffff, 0x00000000003ffff0, 0x000007c0000007c0, 0x00000007ffff0000, 0xffffff07ffffffff, 0xe003ffffffffffff, 0xfff800000000001f, 0x0003fffffffffff0, 0xf000000007ffffff, 0xfffff03ffffff03f, 0xffffffffffffff03, 0xffffffffff001fff, 0x80003fffffffffff, 0x1ffc00001ffc0000, 0x01ffffff01ffffff, 0x0000007ffffe0000, 0xfffff8000000001f, 0x0001ffffffffffe0, 0x0030000000000000, 0x007fc000007fc000, 0x0070007000700070, 0x00000fff80000000, 0x07e0000007e00000, 0xffffffff007fffff, 0xfffff00003ffffff, 0xe7e7e7e7e7e7e7e7, 0xfffe000001ffffff, 0xffe0003fffffffff, 0x00000ffffff00000, 0x1000100010001000, 0x000003fffffffc00, 0xffff801fffff801f, 0x000fffc000000000, 0xfffffffe00000007, 0xff1fff1fff1fff1f, 0x00007fffe0000000, 0x0606060606060606, 0xfc001ffffc001fff, 0x80007fffffffffff, 0x00000000f8000000, 0x000000000003fc00, 0xfff007ffffffffff, 0xff003fffff003fff, 0xc0000003ffffffff, 0x8000000f8000000f, 0x1fffc0001fffc000, 0x0000fffffffffff8, 0x003ffffffffffffe, 0x03ffffffffffc000, 0x00000ffffffffff8, 0xffffffffffff81ff, 0x000000000000fff0, 0xe000000000000003, 0xfffffffc000007ff, 0x0000000007fe0000, 0xfffffffe07ffffff, 0x01fffe0000000000, 0x01fe000001fe0000, 0x0600000000000000, 0x03fc000000000000, 0x00001ffe00000000, 0xffff03ffffff03ff, 0x3ffffffffffffff8, 0x000000000007fc00, 0x001ffffffffffff0, 0x0002000000020000, 0x000001fffffe0000, 0xfffffff00007ffff, 0xffffffffffffffc3, 0x000001e000000000, 0x0000007fffffe000, 0x00ff000000ff0000, 0x0000000000007ff0, 0xfffffc0000000000, 0x0200000002000000, 0x0007fff800000000, 0xffffdfffffffdfff, 0x3ffffffe00000000, 0xffffffffe7ffffff, 0x0000003ffffffff8, 0xfe001fffffffffff, 0x0000000000000780, 0x0007f80000000000, 0x000000000fffff80, 0xfffff800fffff800, 0x00000000000000f8, 0x0ffffe000ffffe00, 0xffffffe000007fff, 0xffffff8fffffff8f, 0x3fffffffffffffe0, 0x01ffffffffe00000, 0x00001fffff800000, 0xfffffe3fffffffff, 0x0000000fe0000000, 0x0000000000000100, 0xffc7ffffffffffff, 0x0038000000380000, 0x8001ffff8001ffff, 0x0fffc00000000000, 0x9fff9fff9fff9fff, 0xf3fff3fff3fff3ff, 0x3ffffffffc000000, 0xfffffffe3fffffff, 0xfe000000000003ff, 0x0007fffffc000000, 0xfffffffffffffe00, 0x0fffffff0fffffff, 0xff8001ffff8001ff, 0x7fffffffe0000000, 0x8000007fffffffff, 0xffff80000000003f, 0xffffffffc0000003, 0xfc001fffffffffff, 0x0000380000000000, 0x8000003f8000003f, 0x000000007ffff000, 0x9fffffff9fffffff, 0x000000007fff0000, 0xfff003ffffffffff, 0x00000007fe000000, 0x0000000000000380, 0x007fffffffffffe0, 0x0000000000003f80, 0xfffff007fffff007, 0x00000007ff000000, 0x01ffffffff000000, 0xfff0000000000000, 0xfff0003ffff0003f, 0xffffffffffc0007f, 0x0000000000000020, 0xfffffff7fffffff7, 0xc00000000000ffff, 0xdfffffffffffffff, 0x00000000000ffffc, 0x0000ffffc0000000, 0x000003ffc0000000, 0x0000ffffffffffff, 0xffe7ffe7ffe7ffe7, 0x00000000007c0000, 0x03f8000003f80000, 0xfffff80000007fff, 0x0007ffffffc00000, 0xffffffff81ffffff, 0x0080000000800000, 0xc0000000000001ff, 0xff80000000003fff, 0xffff000000000007, 0xf001f001f001f001, 0xffff0000000001ff, 0xffffffffffffff87, 0x0007ff000007ff00, 0xf80000001fffffff, 0x03fffe0000000000, 0x0000000fffff0000, 0x00001ffffffff800, 0xfff8000000ffffff, 0x000001fffffffff0, 0xfffffffe0000007f, 0x1ffffffffc000000, 0x03ffffffffffff00, 0x000000000001fc00, 0x0000003ffffff000, 0x001fe000001fe000, 0xfc000000000007ff, 0xfe0000003fffffff, 0x0ffffe0000000000, 0x01ffffffffff8000, 0xf8001ffff8001fff, 0xff80000000ffffff, 0xc000003fc000003f, 0xffc000007fffffff, 0xe07fe07fe07fe07f, 0xc003c003c003c003, 0xfff801fffff801ff, 0xffffff83ffffffff, 0xffffffffffe3ffff, 0xfe03ffffffffffff, 0xffffff000000ffff, 0x00fffffffe000000, 0x800000ff800000ff, 0x0000000007ffff80, 0x0000000001ffff80, 0xffc000000007ffff, 0xffff000001ffffff, 0xc0ffc0ffc0ffc0ff, 0x01fc01fc01fc01fc, 0x07e007e007e007e0, 0xffffe00000000000, 0x0007ffffffff0000, 0xff000000ffffffff, 0xe007e007e007e007, 0xffffff800000003f, 0xfffff80000003fff, 0xe00000000003ffff, 0x03ffffffffe00000, 0xfffff800001fffff, 0xff80003fffffffff, 0x03fffffe00000000, 0x000f8000000f8000, 0x0030000000300000, 0xffffe000007fffff, 0x000000003fff0000, 0x000003ffffffffe0, 0xfff8000000007fff, 0xfff001fffff001ff, 0x007fff8000000000, 0x0000000000000600, 0xf8000000001fffff, 0x3ffffe0000000000, 0x000003ffffe00000, 0xffffffff00000003, 0xffffff00003fffff, 0xffffffff00000001, 0x0000000000ffc000, 0xfc000001ffffffff, 0x00000001fffffffe, 0xf80000000000003f, 0x0001fffffe000000, 0x3ffff8003ffff800, 0xfffffff00001ffff, 0x000000003fff8000, 0x0000000000001fc0, 0xff00000000000001, 0x00000007fffffff0, 0x000000ffff000000, 0x01ffff0000000000, 0x0000000007fffff8, 0x0000000003fe0000, 0x0000000000030000, 0x000fffffffc00000, 0xffffe000ffffffff, 0x00003ff000003ff0, 0xf80000000000000f, 0xfffffffffffffff0, 0x00003ffffc000000, 0xffe0000000001fff, 0x3ffff0003ffff000, 0x0000000000fe0000, 0xffffffffffff00ff, 0x00fc00fc00fc00fc, 0x0000000000007800, 0xfffffffffffe7fff, 0x03fffffffffe0000, 0xc000001fffffffff, 0xfffffffffffc03ff, 0x0000000007800000, 0x000ffffffffff000, 0xffffffff0000003f, 0xfc000000fc000000, 0xf00007ffffffffff, 0xf0000000007fffff, 0x003ffffe003ffffe, 0x00001fffffffc000, 0x00ffff8000000000, 0x3ffffffff0000000, 0x001ff000001ff000, 0x1ffffffff0000000, 0x07f0000007f00000, 0xeeeeeeeeeeeeeeee, 0xf0000007f0000007, 0xe001e001e001e001, 0x00000007fffc0000, 0x00000000003e0000, 0x00000000003fc000, 0x000000007fff8000, 0xfffffffffff00fff, 0xff8000ffffffffff, 0x000000000000fffc, 0x00001fffffff8000, 0xffffffffc3ffffff, 0x0000001ffffe0000, 0x00007fffffffff80, 0xffffffc01fffffff, 0x0000000001800000, 0x000000000f800000, 0x03ffffc000000000, 0xfffffff807ffffff, 0x0780000007800000, 0xff7fffffffffffff, 0xfffe7ffffffe7fff, 0x1000000010000000, 0xffffff80000fffff, 0xf0000000000fffff, 0xffffc00000001fff, 0xfffffffff00fffff, 0xc000000000000000, 0xfe0003ffffffffff, 0x000001fffff00000, 0x0001ffffffffe000, 0xfffc007fffffffff, 0x8000003fffffffff, 0x01fffffffff80000, 0xffc7ffc7ffc7ffc7, 0x0ffffffffff80000, 0x000fe000000fe000, 0xffffc00000003fff, 0xfffffe00001fffff, 0x00000007ffffe000, 0xfff000007fffffff, 0xfffffe3ffffffe3f, 0xf00000000000ffff, 0x00007ff800000000, 0x00000000007fe000, 0x0018000000000000, 0x0000003fffc00000, 0xfffffffffc003fff, 0xffffffe0007fffff, 0xf807fffff807ffff, 0xc3c3c3c3c3c3c3c3, 0x00000000003c0000, 0x0000001ffffc0000, 0xfe7ffe7ffe7ffe7f, 0xffffe0000000003f, 0xffffffffffe00007, 0x01fffff800000000, 0x003ffffffffffffc, 0xfffffffc3fffffff, 0xfffe000000001fff, 0x00000000003ffe00, 0x000000003ffffff0, 0x0000000ffffffc00, 0x000000001fe00000, 0x0000200000000000, 0xff0fffffffffffff, 0xfff00001fff00001, 0x000000000000003e, 0xfffffffff000003f, 0xffffffe0ffffffe0, 0x000000001fffff80, 0xfff000000000007f, 0x7ff07ff07ff07ff0, 0x800000007fffffff, 0xfffffffffff003ff, 0x0100000000000000, 0xffff8001ffff8001, 0x007fc00000000000, 0xfe0000007fffffff, 0xffffffffff8000ff, 0xfffff07ffffff07f, 0x000007ffff000000, 0x0000ff800000ff80, 0x01ffff0001ffff00, 0xfe000000000000ff, 0xffffe00000000003, 0x0018001800180018, 0xe3e3e3e3e3e3e3e3, 0x00001e0000000000, 0x03e003e003e003e0, 0x000f000000000000, 0x00000001ffffffc0, 0x8000ffff8000ffff, 0x0000003ffc000000, 0xfffffffffffc007f, 0x007fffc000000000, 0x000000000003c000, 0x000000ff80000000, 0xffffffff000000ff, 0xf007f007f007f007, 0xfffe00000fffffff, 0xfffffc07fffffc07, 0x00000fffffffe000, 0xe000ffffffffffff, 0x0000000000000008, 0x000007fffffffff8, 0xffffffffffe00000, 0xfffffffff00000ff, 0x0000080000000000, 0x000fff8000000000, 0x001fffffff800000, 0x0000000000001800, 0xffffffffffffe007, 0xfffffffc7fffffff, 0x0000000001ffc000, 0x01fc000001fc0000, 0x007fffffffffffff, 0xffff00007fffffff, 0xfffff03fffffffff, 0xfffffe007fffffff, 0x00000ffffffffffe, 0x01ff01ff01ff01ff, 0xffffc0007fffffff, 0xf80000ffffffffff, 0xffffffffffff803f, 0xffffff00001fffff, 0x003fffffffffffff, 0x000000001ffe0000, 0x0080000000000000, 0x00fe000000fe0000, 0xffffffffffffc01f, 0x07ffffffffffffff, 0x00fffffffff00000, 0xfffffc00001fffff, 0x0000ffffffff0000, 0xe00000000007ffff, 0xffff000fffff000f, 0xe03fe03fe03fe03f, 0xfc0000000000007f, 0x001ffffff8000000, 0xfffffe7fffffffff, 0xfffff00007ffffff, 0xfff80000000000ff, 0xffffffc0000000ff, 0x0001f8000001f800, 0xf9f9f9f9f9f9f9f9, 0xfffffffffffff7ff, 0xfe00000000000000, 0xfffe00000003ffff, 0xff003fffffffffff, 0xfffffe000000003f, 0xf00000001fffffff, 0xffffffff0000000f, 0x000001fffffff800, 0x0600000006000000, 0x001ffff000000000, 0xfffffffffff80003, 0x0380038003800380, 0x1fffff8000000000, 0x0f0f0f0f0f0f0f0f, 0xfc0000000003ffff, 0x0007000000070000, 0x7fffffffffffffe0, 0xfff01ffffff01fff, 0xffc000ffffffffff, 0xffffffffffffc000, 0x00000007c0000000, 0xffffffffffff000f, 0xffffffffffffffe1, 0xfffc0007fffc0007, 0xffe0ffffffe0ffff, 0x0007f00000000000, 0xff000007ff000007, 0xff807fffff807fff, 0xfffffffffff8007f, 0x00003ffe00003ffe, 0x0000000fffffff80, 0x80ffffff80ffffff, 0x003ffffffffffc00, 0xffffffc00007ffff, 0xfff800ffffffffff, 0xffffffc0000003ff, 0xfc000000ffffffff, 0x000fc000000fc000, 0xffffc1ffffffffff, 0x0000007e0000007e, 0xffffffff8000007f, 0x0000000003fffffc, 0x87ff87ff87ff87ff, 0xffffffffff03ffff, 0x8000ffffffffffff, 0xfbfffffffbffffff, 0xfffffff800000000, 0xfc00000000001fff, 0x7ffffc0000000000, 0xffffffffff00003f, 0x000003e0000003e0, 0x0000fff000000000, 0x0000003ffffffc00, 0xffffe3ffffffe3ff, 0x0000003fffe00000, 0xfffffffc000003ff, 0xf8000003f8000003, 0x0000000000018000, 0xffffffffffff003f, 0x1ffffffffffffc00, 0xffffc3ffffffffff, 0xfffff000000003ff, 0x00fffffc00000000, 0xfffe000000000001, 0xfffffff000000007, 0x0000000000000400, 0xffe00000ffffffff, 0xff000000000003ff, 0xffffc003ffffffff, 0xfffffffff003ffff, 0x80000000003fffff, 0x00003fffffffffff, 0xffff807fffffffff, 0xffffffffffff0001, 0x0000000ff0000000, 0x007f007f007f007f, 0xfff800000000000f, 0xfffffffffffff800, 0x000ffffc00000000, 0xf01fffffffffffff, 0xffffffff8000ffff, 0xc1ffc1ffc1ffc1ff, 0xfffffffff8000fff, 0xfffffe0003ffffff, 0xffffffffe1ffffff, 0x000000000fff0000, 0xffffff8000000fff, 0x0000030000000300, 0xfffffffff83fffff, 0x000000000000001f, 0x0000000000000060, 0xffffffffff00001f, 0xfc0ffffffc0fffff, 0x00000000000fffe0, 0xffffc007ffffc007, 0x0001fffffc000000, 0x8000000007ffffff, 0xfffffff800003fff, 0xffffe07fffffffff, 0xfe7ffffffe7fffff, 0x000000fffffc0000, 0x001e0000001e0000, 0xfe000000ffffffff, 0x7fffffff7fffffff, 0xfffe00000000ffff, 0xffffff0000000007, 0xf8000000000000ff, 0xfffff8fffffff8ff, 0x000000ffffffffc0, 0x007ffe00007ffe00, 0x01ffffc001ffffc0, 0xffe1ffe1ffe1ffe1, 0x1fffffffffff0000, 0xffc07fffffffffff, 0x0003f80000000000, 0x0003fffffc000000, 0x00000003fffffff0, 0xfffc00000000001f, 0xbfffbfffbfffbfff, 0xffffbfffffffffff, 0xff3fffffffffffff, 0xfffff00000000007, 0x00000001fe000000, 0x1fffff801fffff80, 0x007c0000007c0000, 0xfffffc07ffffffff, 0x7ffffe007ffffe00, 0x0000078000000000, 0x0003ffff00000000, 0x001f000000000000, 0xfffffffe001fffff, 0x001fffffc0000000, 0xf800000000007fff, 0x0000000000ffffff, 0xffffffff0000001f, 0x000000ffffffc000, 0x00000003ffffe000, 0xfffff87ffffff87f, 0xfff801ffffffffff, 0x80000000000000ff, 0xfffffffffffff80f, 0x0000000000010000, 0x3fffffffffe00000, 0x000007ffffc00000, 0x000000000003ff80, 0xffffff800000ffff, 0x0003fffe0003fffe, 0x0038000000000000, 0xfffffe00000003ff, 0xfffc000007ffffff, 0x80001fffffffffff, 0xfe3ffffffe3fffff, 0x0fffff8000000000, 0x000000000001fe00, 0xfffc1fffffffffff, 0xffffff001fffffff, 0xfc000000003fffff, 0xf80000000000001f, 0x87ffffff87ffffff, 0x000000001ffff800, 0xffffff0001ffffff, 0xc0001fffffffffff, 0xffff81ffffff81ff, 0x0008000000080000, 0x01fffe0001fffe00, 0x0000060000000000, 0x01fffffe00000000, 0xfffffffffffff81f, 0x0000780000007800, 0xfffffff001ffffff, 0x0000007fffffff80, 0x00fff00000fff000, 0xfc01ffffffffffff, 0x1fe000001fe00000, 0x7ff87ff87ff87ff8, 0x000001ffff800000, 0x0000fffffffe0000, 0xfffffff000000001, 0x00000000000001ff, 0x0000000000600000, 0x00000fff00000000, 0x00000fffc0000000, 0x000000000000001e, 0xfffffffff8ffffff, 0x1fffffffc0000000, 0xffff800000000fff, 0xfffff003fffff003, 0xffffffffc007ffff, 0xffffe000ffffe000, 0xffffffffdfffffff, 0x0000000000180000, 0xffffffff80003fff, 0x00c0000000000000, 0xfe00000001ffffff, 0x001fffffffff8000, 0xfffffffff0000007, 0xc0000000001fffff, 0x00000000007fff00, 0xfffffffffffffc7f, 0x000000001ffc0000, 0x0000000ffff00000, 0x03fffff003fffff0, 0x000000ffffe00000, 0xfe000000000fffff, 0x03ffff8000000000, 0xc1c1c1c1c1c1c1c1, 0xe0000007ffffffff, 0xffc01fffffffffff, 0xffff000fffffffff, 0xfc01fffffc01ffff, 0xfffff800000003ff, 0xfefffefffefffeff, 0x00007fc000000000, 0xff00ff00ff00ff00, 0x00f8000000000000, 0x007ffffffffffff8, 0xfffc0000007fffff, 0x0100010001000100, 0x7ffff8007ffff800, 0xff8000000000003f, 0xfffffffffc000001, 0x7fe0000000000000, 0x0ffffffffffffffc, 0x007fffffffff8000, 0xffc0000000000fff, 0xffffe7ffffffffff, 0x0000180000001800, 0xfffff8000000007f, 0xfffffffe000000ff, 0x00000e0000000000, 0xffc0001fffc0001f, 0x0000ff000000ff00, 0x0003fffe00000000, 0x0003fffffffe0000, 0xfffffffefffffffe, 0x00007ffffe000000, 0x0000000000070000, 0xfffffff803ffffff, 0xfffffc3ffffffc3f, 0x1ffffffffe000000, 0xf0000000f0000000, 0x001c0000001c0000, 0x0001fffffffff000, 0xfffffff00000000f, 0x3ffffffffffe0000, 0x003f8000003f8000, 0xffefffffffefffff, 0x8000000000000003, 0xfdfffdfffdfffdff, 0x0000007c0000007c, 0xf81ffffff81fffff, 0xffffffc1ffffffff, 0x00000001ffe00000, 0xff000000000001ff, 0x0001fc0000000000, 0xe0001fffffffffff, 0x3ffffc0000000000, 0xe0e0e0e0e0e0e0e0, 0x0007fffffe000000, 0xf0fffffff0ffffff, 0x000ffc0000000000, 0x00001fff00000000, 0x000000000007fffc, 0x00000000000007fc, 0xfffc000000ffffff, 0xfffffffff800003f, 0x000f0000000f0000, 0x00000001c0000000, 0x0000100000001000, 0xffffffffff007fff, 0x000001fffffffffc, 0xfffffffc0000ffff, 0xffffffffffffe07f, 0xfffff000000001ff, 0xffffffffffff0003, 0xffffffe0000000ff, 0x6060606060606060, 0x03fffff803fffff8, 0xfffffc000000003f, 0xfffffffffff3ffff, 0x8007ffff8007ffff, 0x0003ffe000000000, 0x003fffffffc00000, 0x1e0000001e000000, 0xfff0000fffffffff, 0xffffffffffe001ff, 0x000003fe00000000, 0xe0000000000007ff, 0xffffffff8001ffff, 0x0007f0000007f000, 0x0003ff0000000000, 0x81ffffff81ffffff, 0x0000ffffffffffc0, 0xffffffc000000fff, 0x3000300030003000, 0x03ff000003ff0000, 0xfffff7fffffff7ff, 0x00001fffffe00000, 0xffff8000000007ff, 0xffffc00007ffffff, 0x000000ffffffffe0, 0x1e1e1e1e1e1e1e1e, 0xc0000000003fffff, 0x0000000000fc0000, 0xff80000003ffffff, 0x00001fffffffffc0, 0xf000fffff000ffff, 0xff0000ffffffffff, 0xff8000ffff8000ff, 0x001ffffffffffffc, 0x000000001fffe000, 0x0000003fffffc000, 0x0003c0000003c000, 0xfffffffff8007fff, 0xffffffff000fffff, 0x000001fffffffffe, 0xffffffefffffffff, 0xfffff0ffffffffff, 0x0000000001ffffc0, 0x8f8f8f8f8f8f8f8f, 0x0078000000000000, 0xf00000000000007f, 0x00ffffe000ffffe0, 0x003fff0000000000, 0xffffffffffffc07f, 0xe0ffe0ffe0ffe0ff, 0x000003ffffff0000, 0xe000000fffffffff, 0x00003c0000003c00, 0x001c001c001c001c, 0xffffff00007fffff, 0xfffffffff000007f, 0xfffffffc1fffffff, 0xfff87fffffffffff, 0x00003ffff0000000, 0x7ffffffe00000000, 0x003c0000003c0000, 0x0400000004000000, 0x0ffffffc0ffffffc, 0x00000003ffffffff, 0x0000000007ffffc0, 0x1fffffffffffffff, 0xffffffff80000fff, 0xfff8000fffffffff, 0xffe0000000007fff, 0x0000000001ffffff, 0xfffffc000003ffff, 0xffffe001ffffffff, 0xffc0000000000007, 0xffffffe1ffffffff, 0xfffffffffffe003f, 0x00ffe00000ffe000, 0xfffc00001fffffff, 0x01e0000000000000, 0xfffffffffffe0001, 0xfc000000000001ff, 0x0000007ff0000000, 0x01f8000000000000, 0xffff0000ffffffff, 0xffffffffe3ffffff, 0x00ff800000000000, 0xffffffffff800000, 0xffffffffffe0000f, 0x00000007e0000000, 0xf0000000003fffff, 0xfe0000000fffffff, 0x3fe0000000000000, 0x0060000000000000, 0x7f80000000000000, 0x0018000000180000, 0x03ffe00003ffe000, 0x00000c0000000000, 0x00000000001fe000, 0xfffffffffffc3fff, 0x0004000000040000, 0x7ffffffffff00000, 0xfffffffffffc07ff, 0xfffff800000000ff, 0x8000001f8000001f, 0x000003ffffffe000, 0xfffffc0000ffffff, 0xffffc03fffffffff, 0xf800000000ffffff, 0xff803fffffffffff, 0x0000000000007ffe, 0xfffff800000001ff, 0xfffffffffff9ffff, 0x000003fe000003fe, 0x000000001ffffff0, 0xfffffe0000007fff, 0xfffffffffffc7fff, 0xffffe01fffffe01f, 0xfe00000000001fff, 0xffffffffffffffe0, 0x0100000001000000, 0x3e00000000000000, 0x00000fc000000000, 0xfffe001ffffe001f, 0x000007ffffffff00, 0x00003ff800000000, 0xffefffefffefffef, 0x0000040000000400, 0x0000007ffffffc00, 0x0ffffff80ffffff8, 0x3fffffffffffffff, 0xfffe3ffffffe3fff, 0xffc000001fffffff, 0xfffff801fffff801, 0xfffffe00000000ff, 0xfffffe000000ffff, 0xc000007fffffffff, 0xff0000000fffffff, 0x00000fffffffff00, 0xfff003fffff003ff, 0xfe1ffe1ffe1ffe1f, 0x0404040404040404, 0x0fff000000000000, 0xfffffc00003fffff, 0x0001f0000001f000, 0xffffff8fffffffff, 0x0000070000000000, 0xffe1ffffffe1ffff, 0x00fffffff0000000, 0x007ffff8007ffff8, 0x0007ffffe0000000, 0x00007fffffffc000, 0x07fffffffffffffe, 0xffffffff0fffffff, 0xfffffff3ffffffff, 0x3ffffff03ffffff0, 0xfc000007fc000007, 0xfffffe0000ffffff, 0x0000000001fe0000, 0x000fffffffffffe0, 0xfffc3fffffffffff, 0xf8000fffffffffff, 0xc07fc07fc07fc07f, 0xfffffffff8001fff, 0x00000000000000fc, 0xffffffff9fffffff, 0x000001f0000001f0, 0xc0000000007fffff, 0x00000007fffffe00, 0x07c0000007c00000, 0xfffe000000000000, 0x00000f0000000000, 0x3800380038003800, 0xc07fffffc07fffff, 0x00001f0000000000, 0xfffffffffff1ffff, 0xffffff8000003fff, 0xffffff000000000f, 0x000000000000000f, 0x0003ffff80000000, 0xe00000000000007f, 0x001c000000000000, 0xffffffffffff83ff, 0xe000003fe000003f, 0x00007ffffffffffe, 0x0000000006000000, 0xffe00000000001ff, 0xffffffffff000000, 0x000003ffffffffff, 0x007fffffffffff00, 0xffffff80000001ff, 0xfc03fc03fc03fc03, 0x00000ffff0000000, 0x00000000003fffff, 0x000001ffffffff80, 0x0000007fe0000000, 0xfffff800000007ff, 0x3ffffffc00000000, 0x0000018000000180, 0x83ffffffffffffff, 0xfffffffffffff03f, 0xfffc000000003fff, 0xfff0ffffffffffff, 0xfe1fffffffffffff, 0xffffffffffefffff, 0x803f803f803f803f, 0xe0000fffe0000fff, 0xffffc0001fffffff, 0xfffe1fffffffffff, 0xfe00ffffffffffff, 0xfffffffffff80000, 0xf9ffffffffffffff, 0xfffff8000000ffff, 0x0f000f000f000f00, 0x00000000001f0000, 0x00000000000fff80, 0x001fffffffc00000, 0xc000000000001fff, 0x00001fffe0000000, 0xfffffff0000003ff, 0xffe03fffffe03fff, 0xffcfffffffffffff, 0x000000ffe0000000, 0xffffe0ffffffffff, 0x000fffffe0000000, 0xffffffc000007fff, 0xff000000001fffff, 0x1ffffffe1ffffffe, 0x0000001fff000000, 0x3ffffffe3ffffffe, 0xf000000000000003, 0xfffffc00000fffff, 0x0000000000003ffc, 0xff8003ffffffffff, 0x0003000000030000, 0x0ffffffc00000000, 0x0000fffffffffff0, 0xffffc000000fffff, 0x00000000fffffe00, 0x01ffffe000000000, 0xefffffffefffffff, 0x3fffffffffffffc0, 0x00000ffffffe0000, 0xfffffffc007fffff, 0xffff0000000fffff, 0xfffffbffffffffff, 0xfe00000000000001, 0xff8000000007ffff, 0x07fff80000000000, 0x00000003ffffff80, 0x00007c0000007c00, 0xffc00000001fffff, 0xfe000007ffffffff, 0xfe00000000003fff, 0x003fffffffffffc0, 0xffffffffffffc7ff, 0x00000007fffff000, 0xe0000001ffffffff, 0x000ffffff0000000, 0x003ffc00003ffc00, 0xffffffffffc007ff, 0x01c0000001c00000, 0x0003ff000003ff00, 0xffffffffffe03fff, 0x3ff8000000000000, 0xfff000000000003f, 0xffffffffffe0ffff, 0xf8003fffffffffff, 0xffffffffe01fffff, 0xfffffc003fffffff, 0x003fffff80000000, 0x00000001fffff800, 0x0000000000003fff, 0x0000000fffffc000, 0xfffe1ffffffe1fff, 0xfffffffffe0fffff, 0x0000001000000000, 0x03ffffffffffe000, 0x0000060000000600, 0xe000003fffffffff, 0xf7fffffff7ffffff, 0x07fffffffffffff8, 0x0000000003e00000, 0xfff9fffffff9ffff, 0xfffff83ffffff83f, 0x000fffff00000000, 0x00000007ffffffc0, 0x00001ffffffffff0, 0xfff8000000000fff, 0xffc00000ffffffff, 0xfffff01ffffff01f, 0x00007fffffffffc0, 0x0000000000000078, 0xfe0000000001ffff, 0xffffff0000001fff, 0xffffffffe0000001, 0xfffffffffffc0001, 0xf8003ffff8003fff, 0x01fffff001fffff0, 0x03c0000003c00000, 0xff00007fffffffff, 0x0000000100000001, 0xffff00000001ffff, 0xfc00000000000007, 0xf0000001f0000001, 0x000001ffffffe000, 0x00000e0000000e00, 0xffffc0000fffffff, 0xfffffffffe07ffff, 0x000e000000000000, 0xff81ffffffffffff, 0xffffffffffffefff, 0x001fffffff000000, 0x0001e0000001e000, 0xfbffffffffffffff, 0xffffffff0000ffff, 0xfffff0000000000f, 0x0000000000001000, 0xf00000000000001f, 0xfffff800ffffffff, 0x0ffffffffffff000, 0xff3fffffff3fffff, 0x0000000000020000, 0xf3f3f3f3f3f3f3f3, 0xffffffff07ffffff, 0x0007fffe00000000, 0x0003000000000000, 0xfffffe003fffffff, 0x0000001ffc000000, 0xfffffffff800001f, 0x3ffffffffff00000, 0x000fffffffffffc0, 0x0003ffffc0000000, 0xfffe001fffffffff, 0x07fe000007fe0000, 0x007fffffffffff80, 0x7fff000000000000, 0xfff800000000003f, 0x0800080008000800, 0xffffffffffff807f, 0x00000003ffc00000, 0x0fffe00000000000, 0x0007ffffff800000, 0xffffffc00001ffff, 0x000ffffffffffffe, 0xffffffffff803fff, 0x00000000001ffc00, 0xff8000000000007f, 0x0000007f0000007f, 0x00000007fffff800, 0xfffff81fffffffff, 0xc003ffffffffffff, 0x0000800000000000, 0x03fe000000000000, 0xe7ffe7ffe7ffe7ff, 0xfffffe00000001ff, 0xffff8000ffff8000, 0xf00ff00ff00ff00f, 0x0000007ffffffff8, 0x0000000000000001, 0xffffc000003fffff, 0xfc003fffffffffff, 0x0000000000380000, 0xffff0000000000ff, 0xffcfffcfffcfffcf, 0x0000000300000003, 0xfffffffffffffff8, 0x000000000ffc0000, 0xf00000fff00000ff, 0x3fffffffc0000000, 0x00000000ffffffc0, 0xf7ffffffffffffff, 0x1ffffffff8000000, 0xfff000000001ffff, 0x000007fffffffffe, 0xffffc00000ffffff, 0x000ff000000ff000, 0x01ffffffffc00000, 0xfffff1ffffffffff, 0x1fc0000000000000, 0xffffffff800000ff, 0x1fffffffffe00000, 0x00fffffffffff000, 0x000000000ffffff0, 0xf81fffffffffffff, 0x0004000400040004, 0xffffffbfffffffbf, 0xf800000000000003, 0x000003fc000003fc, 0x000000000007ffff, 0xfffffffff801ffff, 0x00000000000fc000, 0xffdfffffffdfffff, 0x1fffffffffffffc0, 0xfffcfffffffcffff, 0xfc0003fffc0003ff, 0x0000003fffffff80, 0x3ffc00003ffc0000, 0xfffffffff007ffff, 0x000000000001fff8, 0x000003ff00000000, 0x8181818181818181, 0x00007f0000007f00, 0xfffff800003fffff, 0xffffffffefffffff, 0xfff00003fff00003, 0xffffffff80ffffff, 0xf800007ff800007f, 0x0003ffc00003ffc0, 0xffffc00000000fff, 0x0000007ffffffffe, 0x1c0000001c000000, 0x000001ffffe00000, 0x1fffffffffffff80, 0x00000000fffffff8, 0x03ff000000000000, 0xfffffe000001ffff, 0x000000000007fff8, 0x00000007fffffc00, 0x0003ffffffe00000, 0xffff80ffffffffff, 0xfc0fffffffffffff, 0x0000003c00000000, 0xfff7fffffff7ffff, 0xffffff0000000000, 0x9f9f9f9f9f9f9f9f, 0xe0001fffe0001fff, 0x000007fffe000000, 0xfffffc3fffffffff, 0xfc000ffffc000fff, 0x000ffffffffffff0, 0xffffc0000007ffff, 0xffc0ffffffffffff, 0x0000000400000004, 0xfffffffffff8001f, 0x007fffffffe00000, 0x0000fffffffff800, 0x0000000000001f00, 0xffff000003ffffff, 0x00000000ffff0000, 0x0fffffffffffc000, 0xfff80003fff80003, 0x1c00000000000000, 0x0000180000000000, 0x8000001fffffffff, 0xfffffffffffffbff, 0xfe0007ffffffffff, 0x0000038000000000, 0x01fffffffc000000, 0xfffff803ffffffff, 0xf000000fffffffff, 0xfffffffffff8003f, 0xfffffffff0003fff, 0x3fffffe03fffffe0, 0xfc0000fffc0000ff, 0xfffe0003fffe0003, 0xffffffe000003fff, 0x000000000fffc000, 0x0000000ff8000000, 0x0003ffe00003ffe0, 0x00fff80000000000, 0xffffffffffff1fff, 0x000001fffffff000, 0xfffffffffffc01ff, 0xfff1ffffffffffff, 0xfffc000ffffc000f, 0xffffffc7ffffffc7, 0xfffe0003ffffffff, 0xfc0000000000ffff, 0xffffffffffc000ff, 0x0000000000000006, 0x003fc00000000000, 0xe00001ffffffffff, 0x0000000007ffe000, 0x0000000ffffffff0, 0xe000000fe000000f, 0x00003fffffffffe0, 0xfff07fffffffffff, 0x800000000000ffff, 0xfffffffffffff9ff, 0x0000018000000000, 0xfff80001fff80001, 0x00000007ffc00000, 0x0001fe0000000000, 0x000ffffffffffc00, 0xffffffff1fffffff, 0x80000fff80000fff, 0x0000000003fff800, 0xffe00003ffffffff, 0x00001ff000000000, 0xf7f7f7f7f7f7f7f7, 0xfffffffe03ffffff, 0x0fffffffffffffc0, 0xffffff01ffffffff, 0x07fffffffffffffc, 0x00000000fffffc00, 0xffffffff8fffffff, 0x00000000000f0000, 0xfffffff000000fff, 0xfffff001fffff001, 0x3e0000003e000000, 0x07ffffffe0000000, 0x003ffffc003ffffc, 0x00003e0000000000, 0x00000000e0000000, 0xffff80000007ffff, 0x0007c00000000000, 0x0000001fffff8000, 0xfbfbfbfbfbfbfbfb, 0xfffe7fffffffffff, 0x01ff000001ff0000, 0xfffffe0007ffffff, 0x0001ffffffffffc0, 0xf003f003f003f003, 0xffff000000001fff, 0xfc00000007ffffff, 0xffffffff00000007, 0xfe00000fffffffff, 0x000000000ff80000, 0xfffffffe0000001f, 0x000000001ffffe00, 0xfffffe00003fffff, 0x01fff00001fff000, 0x007ffffe00000000, 0x1fffff0000000000, 0x00003fff00000000, 0xf0000ffff0000fff, 0xfe00001ffe00001f, 0x1800000018000000, 0x0000000fffffff00, 0xffffffffff000003, 0xfffffc01ffffffff, 0xffffffffe000007f, 0x0000000fff800000, 0x0003fffffffff800, 0x00003fe000000000, 0x0001ffe00001ffe0, 0xfffffffffe03ffff, 0xfffc000000000003, 0x0000c0000000c000, 0xfffe0fffffffffff, 0x00000007ffffff00, 0x007fffffffffc000, 0xfffffe0fffffffff, 0x3ffffffffff80000, 0xfffffff800000003, 0x3fffffffffff8000, 0xdfffdfffdfffdfff, 0x01ffe00001ffe000, 0x3ffffffffffffffe, 0x03fffffffffffffe, 0xe0000003e0000003, 0xf000007ff000007f, 0xffffc0000001ffff, 0xffe7ffffffffffff, 0xfffffffe007fffff, 0xfffff0003fffffff, 0xfffffffffff000ff, 0x0000007fffffffc0, 0x1fffffff00000000, 0x0003fe000003fe00, 0x0000000ffffe0000, 0x03ffffffc0000000, 0x03fc000003fc0000, 0x00000007fff00000, 0x000000fffffffffe, 0x0000600000000000, 0x0000000000000080, 0xc000000007ffffff, 0xfe00fffffe00ffff, 0xfffc000fffffffff, 0xffc00003ffc00003, 0x0000003ffffc0000, 0xfe000ffffe000fff, 0x00000000fffe0000, 0x0000000000000040, 0x00001f0000001f00, 0x0000030000000000, 0x00fffffffffffc00, 0x8001ffffffffffff, 0x0000000003f00000, 0x0000000000008000, 0xffe0ffe0ffe0ffe0, 0x0004000000000000, 0xfffffe1ffffffe1f, 0xe00000007fffffff, 0xffffff8000007fff, 0x00003f8000003f80, 0xf8000000000fffff, 0x000007ffffffe000, 0xffc00001ffffffff, 0xfe0000000000000f, 0x0000000007fc0000, 0xf800000000001fff, 0xfffffff000000000, 0xfffffffffffff00f, 0xc0000fffffffffff, 0xff00000000001fff, 0xffffc001ffffc001, 0xffe07fffffffffff, 0xfffffffe1fffffff, 0xfffffffc0007ffff, 0x000000000f000000, 0x1fffffffffffc000, 0xf000000000000007, 0x001ffffffffff000, 0x0000000002000000, 0x00000000ff800000, 0xfffff9fffffff9ff, 0xffffffc000000001, 0xfff00ffffff00fff, 0xfff0000007ffffff, 0x0000000000000300, 0x03fe03fe03fe03fe, 0x1ffe00001ffe0000, 0x00001ffff0000000, 0xfc0000000000003f, 0x0000000070000000, 0x0000fffffffffffc, 0xfff000001fffffff, 0x00000000001ff800, 0x00000000007f8000, 0x03ffffffffffff80, 0xffffffffffff8fff, 0x0800000000000000, 0x0000020000000200, 0xfffffffffff03fff, 0x0003ffffffff0000, 0x0007ffffffffe000, 0x000fffc0000fffc0, 0xf000000003ffffff, 0x01fff80001fff800, 0x00000000000001fe, 0x000000001fff0000, 0xfffffffffff8000f, 0xffff0007ffffffff, 0xff00003fff00003f, 0xe0007fffe0007fff, 0x00fffffff8000000, 0x0007ff0000000000, 0xffffffffffe003ff, 0x01fffffffff00000, 0xfffff3ffffffffff, 0x03ff03ff03ff03ff, 0xfffffffe7fffffff, 0x000fffe0000fffe0, 0xfffffffff807ffff, 0x000000001fffff00, 0x03ffffffffffffc0, 0x03fc03fc03fc03fc, 0x0001ffe000000000, 0x0000000000ffff80, 0x07ffe00000000000, 0x00fe00fe00fe00fe, 0xfffe0007fffe0007, 0xfffffffc000001ff, 0x0070000000000000, 0xfc0007fffc0007ff, 0xfffc00007fffffff, 0xfffffffff000ffff, 0x0000ffc00000ffc0, 0x3ffff80000000000, 0xe000000003ffffff, 0x7fffffe000000000, 0x1f1f1f1f1f1f1f1f, 0xffc000000001ffff, 0x0780078007800780, 0x3fc0000000000000, 0x0000fe000000fe00, 0xf800000000000000, 0x00000000000000fe, 0x0007fffff0000000, 0x0003fff80003fff8, 0xff0007ffffffffff, 0x00000000001ff000, 0x01f0000001f00000, 0xfffc0000000fffff, 0x000000007fffc000, 0xff800000001fffff, 0xfffe01ffffffffff, 0xe01fffffffffffff, 0xffffffffffffff01, 0xffffff0007ffffff, 0x003fffff00000000, 0x0010001000100010, 0xfffffc001fffffff, 0xffffff8003ffffff, 0x000000007fffe000, 0xffffffff800007ff, 0x000000ffffff8000, 0x000003ff000003ff, 0xe01fffffe01fffff, 0xf000003fffffffff, 0xc0007fffc0007fff, 0x0060006000600060, 0x0003fffff8000000, 0x0001ffffffc00000, 0xffffff8001ffffff, 0x3ffffffffffffc00, 0xfffffeffffffffff, 0xff80000000000fff, 0x00000fffe0000000, 0x03ffffff00000000, 0xffffffe00000000f, 0x0001fffffff00000, 0x0000007fffff8000, 0x007ffffffff00000, 0xf0000000000000ff, 0xffe0001fffffffff, 0x0000000000003f00, 0xfffffffff8003fff, 0xfffffc00000003ff, 0xfff0000000001fff, 0x0000000000003ff8, 0xffffffffffff0fff, 0x0000000000000fff, 0xfffff8000000000f, 0xffffc00fffffffff, 0xfe0000ffffffffff, 0x0000fffffffc0000, 0x0000000001fffff0, 0x000007ff80000000, 0xf87ff87ff87ff87f, 0x00000fffffff8000, 0xfffffff80000000f, 0xffffffffc000000f, 0x7fffff0000000000, 0xf80003fff80003ff, 0x00007fffffff8000, 0xffffff000fffffff, 0xffffffffff3fffff, 0xffff000000000fff, 0x00000000000003f0, 0xc0007fffffffffff, 0x00fffffffffffff8, 0xfe000001fe000001, 0x1ffffffffffffff0, 0x00000fffff800000, 0xfe00000000ffffff, 0x7800780078007800, 0x00001fffff000000, 0x000007fff0000000, 0xfffffffffffc003f, 0xf800fffff800ffff, 0x0000000038000000, 0xff0000ffff0000ff, 0x0000000008000000, 0x00ffffffffffc000, 0x00000000007ff000, 0xffffffffffc00fff, 0xffff00ffffff00ff, 0xfe0003fffe0003ff, 0xff00000000000007, 0xffffffff0007ffff, 0xfffff8001fffffff, 0xffff01ffffffffff, 0xff0000007fffffff, 0xff8000003fffffff, 0xfff80000007fffff, 0x003ffffffff00000, 0xfffffe0000000000, 0x0001fffffffffc00, 0xffffffffffffdfff, 0x0ffffffffffffe00, 0xffe0000000000fff, 0x00003ffffffffff0, 0xe00001ffe00001ff, 0xfffffff0000007ff, 0x3fffffffffc00000, 0x00000003fc000000, 0x6000000000000000, 0x03fff80000000000, 0xfffffffc0000000f, 0x0007fffffffffff8, 0x00000000fffffffe, 0xfffffffc00000001, 0x0000000fffc00000, 0xff000fffff000fff, 0xffffe0003fffffff, 0x007fe00000000000, 0xfc000000000003ff, 0xe000001fffffffff, 0x1ffffffc00000000, 0x07c0000000000000, 0x8000000000001fff, 0x03fffffffffffffc, 0xfffefffffffeffff, 0x00000001fffe0000, 0x00007ffc00007ffc, 0xe000000000000fff, 0x00000000000000c0, 0x007ffffffffc0000, 0xffffffc00000001f, 0x7000700070007000, 0x00000fffffffffff, 0xfffffffe01ffffff, 0x0007ffe000000000, 0x000000003ffff000, 0xfffffffff0007fff, 0x3f803f803f803f80, 0xf00000000000000f, 0x0000038000000380, 0xffffffffff801fff, 0xffffffff001fffff, 0x0000003ffffffe00, 0x0000000000003000, 0xff00000000003fff, 0x000000007ffffffe, 0xf00003fff00003ff, 0xfc000fffffffffff, 0x00003ffffffff800, 0xff7fffffff7fffff, 0x3ffe3ffe3ffe3ffe, 0x00000003ff000000, 0x000000ffc0000000, 0x001fff00001fff00, 0x0007ffff80000000, 0xfffffffc00ffffff, 0x003ffffff8000000, 0x0001fffffffc0000, 0x07f8000007f80000, 0xfffffffe00001fff, 0xffe00000007fffff, 0xfff80ffffff80fff, 0x000c0000000c0000, 0x000000ffffff0000, 0xf8007fffffffffff, 0xffffc00003ffffff, 0xfe007fffffffffff, 0xfffff000fffff000, 0xfffe0000000fffff, 0x3ffffff800000000, 0xff007fffff007fff, 0xffffe00003ffffff, 0xfffffc7ffffffc7f, 0xffff80007fffffff, 0x1ffffffffffff800, 0x8080808080808080, 0x0000000001f80000, 0xffff8000000fffff, 0x000000000000003c, 0xfffffffffc00ffff, 0x000000fe000000fe, 0xffff007fffffffff, 0x07fc000007fc0000, 0x007ffc00007ffc00, 0xfffffc0001ffffff, 0x00000000001ffffe, 0x000fffe000000000, 0xc00007ffffffffff, 0x0fffffff00000000, 0xff00000001ffffff, 0x800000000fffffff, 0x0000001fe0000000, 0x0000000000000800, 0xffff0000001fffff, 0xff0000000007ffff, 0x01ffff8001ffff80, 0x0000000001000000, 0x0000400000004000, 0xf3fffffff3ffffff, 0x000000000003fffe, 0x0000003f0000003f, 0xfffffffffe0001ff, 0x00000000fffc0000, 0xff800fffffffffff, 0xfffffc00000001ff, 0xffe00003ffe00003, 0x0000000000007f00, 0x007ffffff0000000, 0xfff803ffffffffff, 0xfffffffffff0003f, 0xfffffffffffffe07, 0x000003fffffe0000, 0xf1fffffff1ffffff, 0x00007fff00007fff, 0x0000007ffff00000, 0x0007fff000000000, 0x3838383838383838, 0x3ff03ff03ff03ff0, 0xe000000000001fff, 0xfffe000003ffffff, 0xffe00000000003ff, 0xfff000000000001f, 0x3fffffffe0000000, 0xff0fff0fff0fff0f, 0xffffff800000001f, 0xffffe000003fffff, 0xfffffffffff01fff, 0x0000001fffffff00, 0x0000fffffffff000, 0xffffefffffffefff, 0x80001fff80001fff, 0xfffffffc00001fff, 0xf1f1f1f1f1f1f1f1, 0xffffffffc00001ff, 0x01ffe00000000000, 0xfffdfffffffdffff, 0x7ffffffffffffe00, 0x7fffffffff000000, 0xffc00007ffc00007, 0x000007ffffff8000, 0x0003ffffff000000, 0x003ffffffffc0000, 0xfffffffffffffe01, 0xc00000000000001f, 0xfff80000000007ff, 0xfc000001fc000001, 0x80ffffffffffffff, 0xffffffffffffe03f, 0x000ffff0000ffff0, 0xffffffffffc7ffff, 0x8000000180000001, 0x000000e000000000, 0x003fffffffffff80, 0x000000000000007c, 0x0000001c0000001c, 0xff8000007fffffff, 0x80000000007fffff, 0x007ffe0000000000, 0x000000007c000000, 0x00000ffffffff800, 0x800001ffffffffff, 0x00000001fffffffc, 0x00003fffffe00000, 0xfffffffffffffffd, 0x00fff80000fff800, 0xfffffc000001ffff, 0xc03fc03fc03fc03f, 0xfff87ffffff87fff, 0xf80fffffffffffff, 0x00000000003fffe0, 0xe003ffffe003ffff, 0xfffc1ffffffc1fff, 0xfffdffffffffffff, 0xfff0007fffffffff, 0x00000000000001fc, 0x000e0000000e0000, 0x0c0000000c000000, 0x7f7f7f7f7f7f7f7f, 0x0ffffffffffc0000, 0xffffff80001fffff, 0xff00007fff00007f, 0xfc07fffffc07ffff, 0x1fffffffff800000, 0xfffff0007fffffff, 0xf0001fffffffffff, 0x0000000000000070, 0x7ff8000000000000, 0xc000000000003fff, 0x00000007fffe0000, 0xf8000001ffffffff, 0xe00003ffe00003ff, 0x00fc000000fc0000, 0x0000000000fff800, 0x000007fffff00000, 0xfffffff007ffffff, 0x00e000e000e000e0, 0x0fffffffffe00000, 0xfff800000000007f, 0xffff800000003fff, 0xffe0000000003fff, 0x00ffffffffffffff, 0x0000ffff00000000, 0x7fffc0007fffc000, 0xffc000000000007f, 0x01ffffffffffff80, 0x000003fffc000000, 0x0ffc00000ffc0000, 0x0000000003fffe00, 0xfff0001ffff0001f, 0x0001e00000000000, 0x1ffe000000000000, 0x00001ffc00001ffc, 0x00000000ff000000, 0xfc3fffffffffffff, 0xfff000003fffffff, 0x0ffffff000000000, 0x000000000ffffffc, 0xfffc03ffffffffff, 0xf800000ff800000f, 0xffffffdfffffffff, 0x0000000007e00000, 0x00000ff000000000, 0x00000000000003ff, 0x000000000007fff0, 0x00007ffffc000000, 0xfffffe00ffffffff, 0xfffffffffffffffc, 0x000001e0000001e0, 0x00000000003ff800, 0xff0000000003ffff, 0xffff800000000003, 0x0000000007fffe00, 0xfe00fe00fe00fe00, 0xffc00001ffc00001, 0x0fff80000fff8000, 0xffff83ffffffffff, 0x000000000000fc00, 0x0000ff0000000000, 0xff000000000007ff, 0x7f807f807f807f80, 0xfffffe00007fffff, 0x00007ff800007ff8, 0xfffffe0ffffffe0f, 0xf000000000001fff, 0x0001ffffc0000000, 0xe0000000003fffff, 0xfffffffff07fffff, 0x01fffffe01fffffe, 0x0fffff800fffff80, 0xfffffc000fffffff, 0x001fff80001fff80, 0x0000000003ffffe0, 0xfffffffffe001fff, 0x4000400040004000, 0x07fffffc07fffffc, 0xfffff80007ffffff, 0xffffffffffff0007, 0x0000001ffff00000, 0xc001c001c001c001, 0x003ffe0000000000, 0xffffffffff9fffff, 0xffffc0ffffffc0ff, 0xfff807fffff807ff, 0xfc00000000000003, 0x0000000007f80000, 0xffffff0000000003, 0xfff80000ffffffff, 0x0003ff800003ff80, 0x0003ffffff800000, 0xfffffe0000001fff, 0xfffffff80001ffff, 0xff00ffffffffffff, 0x000000000007ff80, 0x00000000c0000000, 0xffc00000007fffff, 0x000ffffffffffe00, 0xf8000000000003ff, 0xfffffffc03ffffff, 0xffffffe00000007f, 0xfffe00003fffffff, 0xfffe03ffffffffff, 0xffe3ffffffffffff, 0xfffffe00fffffe00, 0x0000fff00000fff0, 0x007fffffffffe000, 0xf80000003fffffff, 0xfff800000fffffff, 0x01ffff8000000000, 0x07fffffff8000000, 0x03f0000003f00000, 0xffffff81ffffff81, 0x00007ffffffff000, 0xfc003ffffc003fff, 0xfffffc00000000ff, 0xff0000000000003f, 0x00000000001c0000, 0xfff8001ffff8001f, 0xffffff00000003ff, 0xffff807fffff807f, 0x7f8000007f800000, 0xfffffffc0fffffff, 0x07ffffffffff0000, 0x1e00000000000000, 0xfff0003fffffffff, 0xffffffc03fffffff, 0x007ff000007ff000, 0xffffffff0003ffff, 0x07fffffffffffff0, 0xfe0000000000001f, 0xfffffffffff8ffff, 0xf0003ffff0003fff, 0xfffffff80000007f, 0x0000000100000000, 0xfffffe0000000003, 0xfffffcfffffffcff, 0x0000001fffff0000, 0x000000001fffffc0, 0x0000000000fffffe, 0x000000f0000000f0, 0x0003fffff0000000, 0xf87fffffffffffff, 0xfffe000000ffffff, 0x1fff800000000000, 0xf83fffffffffffff, 0x0002000200020002, 0xfffffe7ffffffe7f, 0xfffffbfffffffbff, 0xfffffffffffc0fff, 0xc00003ffc00003ff, 0x0003fffc0003fffc, 0xc0003fffffffffff, 0x000000003fffff00, 0x003ffffffffff800, 0x80000fffffffffff, 0x0000000ffffff800, 0x00007ffffff00000, 0x0038003800380038, 0x003f800000000000, 0xfdfffffffdffffff, 0xfffffff003ffffff, 0xc0000000000007ff, 0x800fffffffffffff, 0xffc1ffffffffffff, 0x000007ffffff0000, 0xfffffe0000003fff, 0x3ffffffffffc0000, 0x0003f0000003f000, 0xffffffe0000001ff, 0xfffffffffffffffb, 0x3fff000000000000, 0x00001fffffffff80, 0xffffffff003fffff, 0x0000008000000080, 0x0180018001800180, 0x00000000000e0000, 0x7fff7fff7fff7fff, 0x00000001fc000000, 0x0000000000ff0000, 0xfff0000001ffffff, 0xfffff8ffffffffff, 0xffffc07fffffffff, 0x0000000004000000, 0xffffffcfffffffff, 0x00001fe000001fe0, 0x0000007ffffffff0, 0xfff0007ffff0007f, 0x1ffffff800000000, 0x000000000ffffff8, 0xfffff00000ffffff, 0xffe1ffffffffffff, 0xfffff8000007ffff, 0xf80000000000ffff, 0x00fffffffffe0000, 0x6666666666666666, 0xffc0001fffffffff, 0xfff8000ffff8000f, 0x0000007fff800000, 0x7ffffffffffffff0, 0x7fffff807fffff80, 0xffe000ffffffffff, 0x00001ffffffffe00, 0xfffffffffffff87f, 0x0007fffc00000000, 0xffffffff80007fff, 0xffffffdfffffffdf, 0x07f0000000000000, 0xffff00000000000f, 0xffffff0fffffffff, 0xc007ffffc007ffff, 0x7fffffffffffc000, 0xffffffffc1ffffff, 0xffffe0000fffffff, 0xfc00000003ffffff, 0xff1fffffff1fffff, 0x0006000600060006, 0x00000000000003c0, 0xe07fffffe07fffff, 0xfe003ffffe003fff, 0xfffffffff0000003, 0xff80007fff80007f, 0x000fffffffffff80, 0x1ffffffffff00000, 0xffff80ffffff80ff, 0xfffffffff80000ff, 0xffffffffc0001fff, 0xfc000000007fffff, 0x007ffffc007ffffc, 0x1f80000000000000, 0xffffffe000000000, 0xf0ffffffffffffff, 0xff00000000000fff, 0xfffc00ffffffffff, 0x0000007ff8000000, 0x0ffffc000ffffc00, 0x000007ffffffffc0, 0xffffffffffff07ff, 0xfffc00000000003f, 0x0000000000f80000, 0x0ffffffff8000000, 0x07fffffffffff000, 0x000000001ffffffe, 0xfffc0000ffffffff, 0x000000000000ffe0, 0x4000000040000000, 0xfffff80003ffffff, 0xfff80001ffffffff, 0x00000000fffffffc, 0xfffffffc0001ffff, 0xc00000ffc00000ff, 0x8000000000000001, 0x00000fff00000fff, 0xfffc00000000007f, 0xffe0000007ffffff, 0xff00000fffffffff, 0xff0000000000001f, 0x00003ffe00000000, 0xffffe7ffffffe7ff, 0x00000003ff800000, 0xffffffffffffffc7, 0x01fffffffffffffe, 0xfffc03fffffc03ff, 0xffffffffe00fffff, 0x00007fff00000000, 0x0000001ffffffff0, 0xf801fffff801ffff, 0x3f00000000000000, 0x0000000000000018, 0x007ffffffff80000, 0x0000000000003fc0, 0xbbbbbbbbbbbbbbbb, 0x003ffffff0000000, 0xffff800000000001, 0x3fffffe000000000, 0xfc07ffffffffffff, 0xffff81ffffffffff, 0xffffffffffc1ffff, 0xfe00007fffffffff, 0x000000001f000000, 0xffffffffff8001ff, 0x0007fe000007fe00, 0x0007ffffffffc000, 0x003fffffffffffe0, 0xffffffffffffc001, 0x000000003fffffc0, 0x07ffffffffffff00, 0xfffe00000000001f, 0x800007ffffffffff, 0xffffffe0000007ff, 0x000000007ff80000, 0x000000000007f000, 0x000000003ffffc00, 0xfff800000003ffff, 0x7fffffffff800000, 0x00fffff800fffff8, 0xc3ffc3ffc3ffc3ff, 0xffc001ffffffffff, 0x1fe0000000000000, 0x00000fffffc00000, 0x0ffffffe00000000, 0xfff000fffff000ff, 0x0000007000000070, 0xe0000000007fffff, 0x0000007ffffffffc, 0x0000007fc0000000, 0x001f001f001f001f, 0xffc00000003fffff, 0xfffe03fffffe03ff, 0x00000001ffffffe0, 0x0007ffffc0000000, 0xcfffffffcfffffff, 0x0000007c00000000, 0x000000fffff00000, 0xffffcfffffffffff, 0x07ff07ff07ff07ff, 0x0000000003ffffff, 0xff8000000fffffff, 0x003fffff003fffff, 0xff8000000000ffff, 0xffff80000fffffff, 0x3ffffe003ffffe00, 0x8fffffffffffffff, 0x00000003fffe0000, 0x00fffffffffffffe, 0xfe01ffffffffffff, 0x003ffffffffffff0, 0x001fffffffffffe0, 0x800007ff800007ff, 0xf8000000000001ff, 0xfc000003fc000003, 0xf0000fffffffffff, 0x00001fffc0000000, 0xfffffff87fffffff, 0xffffffff01ffffff, 0x00ffffff00ffffff, 0xfffffffffe000003, 0x1fff1fff1fff1fff, 0x0700070007000700, 0xfffffff81fffffff, 0x000fffffffffe000, 0x0000001ffffff800, 0x000003fff8000000, 0x3fffff803fffff80, 0xffffffc0003fffff, 0x01fffffffffffffc, 0xfffffc00000007ff, 0xfffff00000000000, 0x000000e0000000e0, 0xffffe00000000fff, 0x7ffffffffff80000, 0xffc3ffc3ffc3ffc3, 0x3fffff003fffff00, 0xfc0000000000000f, 0x00003ffffffff000, 0xfffc00003fffffff, 0x03ffffffffff8000, 0xfffffffffbffffff, 0x0070000000700000, 0xfffffff801ffffff, 0xc00000001fffffff, 0x00000000001ffff8, 0xffffffffe00007ff, 0x07ffffc007ffffc0, 0xffffffffff800001, 0x1fffe0001fffe000, 0xff83ffffffffffff, 0x000ffffffffff800, 0xff0fffffff0fffff, 0xff8fffffffffffff, 0xffffffffff0001ff, 0xff0003ffffffffff, 0x001ffffffffffe00, 0x000000003fffffe0, 0xfffffffffffffc1f, 0x00000000001fff00, 0x01ffc00000000000, 0x001fff0000000000, 0x003ffffffe000000, 0xffffe00000000007, 0xffffffffff800007, 0xff81ffffff81ffff, 0xffff800007ffffff, 0x000007ffffffc000, 0xfff800000001ffff, 0xfffffffff80003ff, 0x00000001fffff000, 0x1800000000000000, 0x0001fffff0000000, 0xf03fffffffffffff, 0xfffff3fffffff3ff, 0xffffff7fffffffff, 0x001fffffffffffc0, 0x0007fff00007fff0, 0xfffffffff9ffffff, 0xffffffffffc03fff, 0x00000000000003fc, 0xffff8000001fffff, 0xfffffc0000000001, 0x0000001ffff80000, 0x0000001fffffffc0, 0xff80000fffffffff, 0x0fffffffff000000, 0xffffe0007fffffff, 0xfe03fffffe03ffff, 0x003f0000003f0000, 0xfff0000000000fff, 0x3e003e003e003e00, 0xfffffffffffff8ff, 0xffffffffe0000000, 0xfffffffffffff007, 0x1c001c001c001c00, 0xfffffe000000001f, 0xfff800003fffffff, 0xfffffff00003ffff, 0x00003e0000003e00, 0x00000001fffffe00, 0x3fffff0000000000, 0xfe0000000000007f, 0xffffffe03fffffff, 0xfc00003ffc00003f, 0x007e000000000000, 0xffe01fffffffffff, 0x001fc000001fc000, 0x000000001ffffffc, 0xfffc001fffffffff, 0xfe000000007fffff, 0x007fffffc0000000, 0xffffffe7ffffffff, 0x00000000fc000000, 0xfc00000000000001, 0x003fffffffff0000, 0x800000ffffffffff, 0x000003e000000000, 0xffffffffffc0001f, 0x0000002000000000, 0x00000000003fe000, 0x0e0e0e0e0e0e0e0e, 0x003ffff8003ffff8, 0xfffffc0000007fff, 0xfffff0001fffffff, 0x003fffffff000000, 0x07ffffc000000000, 0xffff80000001ffff, 0xff801fffffffffff, 0xfffc007ffffc007f, 0x000001ffffffc000, 0xfff83ffffff83fff, 0xfffc0001ffffffff, 0x8000000fffffffff, 0xfffc000000007fff, 0xff00000003ffffff, 0x000007ffffe00000, 0x0000001f0000001f, 0x0001ffffffff0000, 0x000003f8000003f8, 0x0000000000000f80, 0xe03fffffffffffff, 0xff8000000000001f, 0xe000000000000000, 0x00000000000001f8, 0xffe00007ffffffff, 0xfffffff0ffffffff, 0x001ffffffffffc00, 0xffffffffff00ffff, 0x0020000000200000, 0x03c0000000000000, 0xfffe0001ffffffff, 0xe001ffffe001ffff, 0x0030003000300030, 0xfff00000000001ff, 0xff8001ffffffffff, 0xfffff000000000ff, 0x001fffc000000000, 0xffffffffe0001fff, 0x00000000003ffff8, 0x001ff00000000000, 0x8000000000000007, 0xff0000001fffffff, 0xffffe03fffffffff, 0x01ffffc000000000, 0x0ffffffe0ffffffe, 0x0000001fffffc000, 0xffffffc07fffffff, 0xfff0001fffffffff, 0xfc00000000000000, 0xe7ffffffffffffff, 0xffffffffffffe00f, 0x0007fffffffffffe, 0xffffffc1ffffffc1, 0x0000000000007ffc, 0x07fffffff0000000, 0xffffffc00000003f, 0xfffff800000fffff, 0xffffe007ffffffff, 0xfffffe000fffffff, 0xfffffffffc0fffff, 0xc00000000001ffff, 0xc0000003c0000003, 0xff9fffffffffffff, 0x0ffe00000ffe0000, 0xffffffffff800fff, 0x01fe000000000000, 0x001ffffffffc0000, 0xffff80003fffffff, 0x7ffffffffe000000, 0x00000ffe00000000, 0xfff80000001fffff, 0xffffc0000000001f, 0x3ffffffffffff000, 0x0000007fffc00000, 0xfff81fffffffffff, 0xfffffe0000000001, 0xffffff00ffffffff, 0x00ffffffe0000000, 0x000003fffffc0000, 0x03ffffffffff0000, 0xfffffff8000003ff, 0x3ff83ff83ff83ff8, 0x0001fffffffffffc, 0x0000000000001ffc, 0xffffff9fffffff9f, 0x0300000003000000, 0x0000ffffffff8000, 0x0003ffffffff8000, 0xfffffffffe01ffff, 0xffffe1ffffffffff, 0xffffffffe007ffff, 0xff81ff81ff81ff81, 0xffffff0fffffff0f, 0x83ff83ff83ff83ff, 0x0fc000000fc00000, 0xfffe0000ffffffff, 0x000ffff800000000, 0xff80000000000000, 0x0000000003ff8000, 0x00007ffc00000000, 0x0fffffffff800000, 0x1fffffffffffff00, 0xfffffc1fffffffff, 0x00000007ffe00000, 0xffffc0003fffffff, 0x07fc07fc07fc07fc, 0xffffffffffe0003f, 0xffe000000007ffff, 0xfffe000ffffe000f, 0xc001ffffffffffff, 0x3fffffc000000000, 0x0000000007f00000, 0x0003e00000000000, 0x00000007ffffc000, 0x000000001fc00000, 0xffcfffffffcfffff, 0x0000008000000000, 0x0000001fff800000, 0x00003fffffc00000, 0x0000000000fffff0, 0xe000000007ffffff, 0xfe01fffffe01ffff, 0x00000001ffffff80, 0xf8001fffffffffff, 0x00001fff00001fff, 0x000000001fffffe0, 0x000000fff0000000, 0x00fffffffffffff0, 0x000fff00000fff00, 0xe0ffffffe0ffffff, 0xffe0000000000001, 0xfffffffff8000000, 0x0000003ffffff800, 0x003fffe000000000, 0x7e00000000000000, 0xffffffffe0000003, 0xfff8fff8fff8fff8, 0xe0000001e0000001, 0xf800000000000fff, 0xc007c007c007c007, 0xffffffe000000003, 0x0078007800780078, 0x0000000fffffe000, 0xfc00007ffc00007f, 0xff1fffffffffffff, 0x0000000001ff8000, 0xfffffffff001ffff, 0x801fffff801fffff, 0xffffffffffffe001, 0x000000003ffffffc, 0xfffff803fffff803, 0xff80ffffff80ffff, 0xc000003fffffffff, 0xefefefefefefefef, 0x8007ffffffffffff, 0xfe3ffe3ffe3ffe3f, 0x01ffffffffffc000, 0x0000000000000e00, 0x00007ffffffffffc, 0xc7ffffffc7ffffff, 0x7c7c7c7c7c7c7c7c, 0xaaaaaaaaaaaaaaaa, 0xff803fffff803fff, 0xffffffc3ffffffc3, 0x0000ffffffffc000, 0xfffffffe003fffff, 0x0380000000000000, 0x000000000000007e, 0x03fffffffffffff0, 0x000c000c000c000c, 0xffffffffc0000000, 0x00000000007fff80, 0xf80001fff80001ff, 0xfffe07fffffe07ff, 0x000ffff000000000, 0x8383838383838383, 0x000000000003fff8, 0x01ffffffffffffe0, 0x9fffffffffffffff, 0xfffe000000003fff, 0x0001fff00001fff0, 0x00003f8000000000, 0xfffffc000000ffff, 0xfffffe000007ffff, 0x00007fffffff0000, 0xfffffff8000001ff, 0x0fff0fff0fff0fff, 0x000000fc000000fc, 0xff807fffffffffff, 0x0007fffff8000000, 0x000003c0000003c0, 0xfffe00000000003f, 0xfffffffffc00007f, 0x01fffffffe000000, 0x0078000000780000, 0x000000000001ff00, 0xc0001fffc0001fff, 0xffc000ffffc000ff, 0xc000001fc000001f, 0xfffcffffffffffff, 0xffc00000000001ff, 0xffff9fffffff9fff, 0x001ffff800000000, 0xfffffc007fffffff, 0x0000200000002000, 0xf000ffffffffffff, 0x000000000001fff0, 0x000ffc00000ffc00, 0xffffff800003ffff, 0x0fffff0000000000, 0xfffffff00000007f, 0xffffffffc0ffffff, 0xfffffff8000000ff, 0x0000001e0000001e, 0x0000002000000020, 0x3fc03fc03fc03fc0, 0xfc03fffffc03ffff, 0x0000c00000000000, 0xfffffffffc07ffff, 0x7878787878787878, 0xffffffffffffffef, 0x00000003fffffffc, 0x00007fffffffffe0, 0x0000001ffffffffc, 0x000000000ffffc00, 0x00000ff000000ff0, 0x0007ffff00000000, 0xffffff87ffffff87, 0x0007000000000000, 0xffe000001fffffff, 0xffc0ffc0ffc0ffc0, 0x00fffffe00fffffe, 0xff00000fff00000f, 0x0000000f80000000, 0xfffffff01fffffff, 0x0000000000ffff00, 0xfffe00000000000f, 0xfffffff0000000ff, 0xffffffffffe7ffff, 0x0003fc000003fc00, 0xe7ffffffe7ffffff, 0xffffc00000000001, 0x01fffffffffffff8, 0xc0000000000000ff, 0xf000000000000000, 0x0000003fff800000, 0x0003ffffffffffff, 0x00000007fc000000, 0x0001ffc00001ffc0, 0xffffffc001ffffff, 0x0000000001fffc00, 0x000007fff8000000, 0x000000c0000000c0, 0x0000e0000000e000, 0xffff8000000001ff, 0xff9fffffff9fffff, 0x3ff000003ff00000, 0xf000000000000fff, 0xc0000001c0000001, 0xfff07ffffff07fff, 0x8000000380000003, 0xffffffffffff8000, 0xf80000000007ffff, 0x000000007f800000, 0x003fffffe0000000, 0xfffffffffc7fffff, 0x1fc000001fc00000, 0x1fe01fe01fe01fe0, 0xfff800000007ffff, 0xfe000000000007ff, 0xffffff0000007fff, 0x000000ffffffff00, 0x00000007ffffffe0, 0xffffc007ffffffff, 0x03fffffffffffff8, 0x0000fffe0000fffe, 0x0200020002000200, 0x8000000000000000, 0xffffffffffff01ff, 0x000003ffffffffc0, 0x3c003c003c003c00, 0x0000000078000000, 0x0808080808080808, 0xc000000000000003, 0xfffffffffe000fff, 0xc00fc00fc00fc00f, 0x7ff0000000000000, 0xffffffffff000001, 0x000fffffffffc000, 0xfffc0000fffc0000, 0xff800003ff800003, 0x0003fffffff00000, 0x00000fffff000000, 0x07fffff007fffff0, 0xfcfffffffcffffff, 0xfffffe000000000f, 0xffff800001ffffff, 0xff00003fffffffff, 0x0007ffc000000000, 0x000007f000000000, 0x00f0000000000000, 0x000ffffc000ffffc, 0x000000ffffffe000, 0xffffffffffffe000, 0xfe07fe07fe07fe07, 0x7e007e007e007e00, 0xffff80000000001f, 0x00ffffffffe00000, 0xffc000000000001f, 0xffffffffffffe3ff, 0xc0000fffc0000fff, 0x0000000000001c00, 0x0180000001800000, 0xfffffe03fffffe03, 0xffff803fffffffff, 0x00000003fffffffe, 0xfffff007ffffffff, 0xffffff00000fffff, 0xf00000ffffffffff, 0xfc01fc01fc01fc01, 0x3ffe000000000000, 0x000000000001ff80, 0x003fff80003fff80, 0x0e0000000e000000, 0xffffffffc00007ff, 0x000000000003e000, 0x80000000000007ff, 0x07fe000000000000, 0x000003fc00000000, 0xffc3ffffffffffff, 0x8000000000003fff, 0xf01ff01ff01ff01f, 0xf800000fffffffff, 0x1e001e001e001e00, 0x00001ffffff00000, 0xfffffffffc000007, 0xffe00fffffffffff, 0x3fffffffff000000, 0x000000000fff8000, 0x007fffffff000000, 0x000000007ffffff8, 0x0fffffffe0000000, 0xfffffffffe00001f, 0xffff83ffffff83ff, 0xffffffffcfffffff, 0x000000000000fff8, 0xff00000000000000, 0xffff003fffff003f, 0xffffff7fffffff7f, 0x7fffffffffc00000, 0x0000007fffe00000, 0xffe0000001ffffff, 0xfc000007ffffffff, 0xfe07ffffffffffff, 0x7fffff8000000000, 0xfffffffc0000003f, 0x3030303030303030, 0xffe000000003ffff, 0xfffffffffe003fff, 0x7fff80007fff8000, 0x81ffffffffffffff, 0x01fff00000000000, 0x0000000fc0000000, 0x07ffffffffffff80, 0xffffffffffffe0ff, 0xe00000000000001f, 0xffffffffffffffc0, 0x000001fffffffe00, 0x003fffffffffe000, 0x001fe00000000000, 0xfc000000001fffff, 0xfffffffffc0001ff, 0x000000fffffffff0, 0x1ffffffe00000000, 0xfffffff80000ffff, 0x000000007fffff00, 0xffffffffffff007f, 0xffffc000001fffff, 0xfff800fffff800ff, 0x0000ffffffffffe0, 0xffffffc0000fffff, 0x7ffffffff8000000, 0x03fff80003fff800, 0x007ffffffffe0000, 0x03f803f803f803f8, 0x00000003ffffffe0, 0xfffffffffff80001, 0x0000000007ff8000, 0x000003f800000000, 0x801f801f801f801f, 0x03fff00003fff000, 0xfffff00000003fff, 0xffffffff800001ff, 0x000003fffffffff8, 0xffff000000ffffff, 0xffffffff80000003, 0x00000001fff80000, 0xc000000003ffffff, 0x0008000800080008, 0x0007800000078000, 0xfffffffe00003fff, 0xc00000000007ffff, 0x001fffffffffe000, 0x000007fe00000000, 0x000001fffffffc00, 0xfe000003ffffffff, 0x3ffc3ffc3ffc3ffc, 0x000000007e000000, 0xfffffffffc0003ff, 0x8000000000000fff, 0xc0ffffffc0ffffff, 0x00fff00000000000, 0xfffffffe0003ffff, 0xffffffffc00fffff, 0x000fff80000fff80, 0xe00000000fffffff, 0xc000000fffffffff, 0x0000000003fffff0, 0xfffffc1ffffffc1f, 0xfffffff800007fff, 0xfff00000007fffff, 0xffff80000000007f, 0x003e000000000000, 0xc000000000007fff, 0x003ffff0003ffff0, 0xfffffffffffffc00, 0x00000001e0000000, 0x1ffffe0000000000, 0xff80ffffffffffff, 0x00f000f000f000f0, 0xfffe0000000000ff, 0x1ffffffc1ffffffc, 0x000ffffffffffff8, 0x0ffff80000000000, 0x0000000001ffe000, 0xfffff0000000003f, 0xfc7ffffffc7fffff, 0x000000000007e000, 0x7800000078000000, 0x0000000001fffff8, 0xfffc0ffffffc0fff, 0x2222222222222222, 0x000000001ffffc00, 0xffffffe0003fffff, 0x0000000000700000, 0xfffffffe0000ffff, 0xff0000000000ffff, 0x0000ffffffffe000, 0x00001ffe00001ffe, 0xfe01fe01fe01fe01, 0x000000003f800000, 0xffff0000007fffff, 0x0000000000000ffc, 0x01fffff801fffff8, 0x0000001000000010, 0xffffffe00000001f, 0xffffffffffffff83, 0xfffffffffffe0fff, 0x000f000f000f000f, 0xffffff8000000000, 0x0007fffffffc0000, 0x003ffe00003ffe00, 0xffffe03fffffe03f, 0x0007ff8000000000, 0x00000001fffc0000, 0xfff80007fff80007, 0x000000000000f800, 0xfffffff80000003f, 0xfffc0007ffffffff, 0x00ffffff00000000, 0x0000001fffffffff, 0x3ffffffffffff800, 0x0000000000000030, 0xfffffffffc00000f, 0xe00000001fffffff, 0x000007fc000007fc, 0x000000007fffff80, 0xffc1ffffffc1ffff, 0x07ff800007ff8000, 0x07ff000000000000, 0x00000003ffe00000, 0xff800003ffffffff, 0x000000fffffe0000, 0xffffffffffffc007, 0x00000000001fffe0, 0x01ffffffe0000000, 0x00003fffffff0000, 0x000000007ff00000, 0xfc007fffffffffff, 0x3f3f3f3f3f3f3f3f, 0x0000003ffffffff0, 0xffffffff00001fff, 0x0ffff8000ffff800, 0x001f800000000000, 0x0000000001fff800, 0xfffffe000003ffff, 0x0001fff80001fff8, 0xffffffc0000001ff, 0x00ffff8000ffff80, 0xfff8000000003fff, 0xfe007ffffe007fff, 0xffffffe00003ffff, 0x01ffffffffffe000, 0x0fff00000fff0000, 0x0000000000006000, 0x00001fffffc00000, 0x0003ffffffc00000, 0xfffffffff00003ff, 0x00003ffffffffffc, 0x0fffffe00fffffe0, 0x0000000000100000, 0x3ffffffffffffff0, 0xfffffffffffff01f, 0xffffff80ffffffff, 0x000001fffe000000, 0x00003ffffffc0000, 0x03fffffffffff000, 0xffffffffffff801f, 0xffff800000007fff, 0x7ffffffffffffff8, 0xffffffffffff8001, 0x00007fe000007fe0, 0xff87ffffff87ffff, 0x001ffff0001ffff0, 0x0180000000000000, 0xffffffffffff87ff, 0xffffe000000001ff, 0xfe0000000003ffff, 0xffff00000000001f, 0x000000000fffffff, 0x0040004000400040, 0xfc0000007fffffff, 0x07fffff000000000, 0x00000000000ff000, 0x07f807f807f807f8, 0x3ffffffffffffffc, 0xf00007fff00007ff, 0xffffffe0001fffff, 0x000000000000003f, 0x07fffffc00000000, 0xfffffffeffffffff, 0x0001fffe00000000, 0x1fffffe01fffffe0, 0xffffc00001ffffff, 0x0007fffffffffc00, 0xffffffe0ffffffff, 0x1ffff00000000000, 0xffff0000ffff0000, 0x0000001fffffe000, 0x01ff800000000000, 0xffff0fffffff0fff, 0x000000ffffc00000, 0xfffffc7fffffffff, 0xffffffffe0000fff, 0xffffff800fffffff, 0x00ffffffffffffe0, 0xfe000fffffffffff, 0x0000000003fc0000, 0x03fffff800000000, 0xf80ffffff80fffff, 0x00000000003ffffc, 0x0000001ffffffc00, 0xfffffffffc000003, 0xe000000000007fff, 0xff8000000000000f, 0x0ffff0000ffff000, 0xfffff80ffffff80f, 0xfffff000001fffff, 0x7fffff007fffff00, 0xfffe0000000001ff, 0x0000001fffc00000, 0x1fffffe000000000, 0x01ffffffffff0000, 0xffbfffbfffbfffbf, 0xfffffffffe000007, 0xffffff0000ffffff, 0xfffffff7ffffffff, 0xf8f8f8f8f8f8f8f8, 0xff0000000000000f, 0x03fffffffff00000, 0xf80001ffffffffff, 0xfff8007fffffffff, 0xfff00000001fffff, 0x1fff000000000000, 0x07ffffffffe00000, 0x00003fff00003fff, 0xffffe00000003fff, 0xff0000000000007f, 0xff8fffffff8fffff, 0xc0003fffc0003fff, 0x1ff800001ff80000, 0xffffffffffe1ffff, 0x1010101010101010, 0xffffffc00000000f, 0x007ffffffc000000, 0xffc00fffffffffff, 0xe000000000003fff, 0x000001ffffffffff, 0xffffffbfffffffff, 0xfffffffffe00ffff, 0xf00003ffffffffff, 0x0000000007fff000, 0xffc00000ffc00000, 0x00000000007ffff8, 0x3ffff00000000000, 0xffff00000000003f, 0xf800001ff800001f, 0x0000ffe00000ffe0, 0x0ffffffffffe0000, 0x7ff000007ff00000, 0x0fff800000000000, 0xffffffffc0003fff, 0xfffff0fffffff0ff, 0xf07ff07ff07ff07f, 0x00001f8000001f80, 0x0007f8000007f800, 0x03f8000000000000, 0x00fc000000000000, 0x001fffffffffffff, 0x000000000000f000, 0x00000003f0000000, 0xffffffc00fffffff, 0xfffe00001fffffff, 0xfffbfffffffbffff, 0x8001800180018001, 0xfc000003ffffffff, 0xf1fff1fff1fff1ff, 0xfffffffffffe0007, 0xfff8fffffff8ffff, 0x03fffc0003fffc00, 0x00000001fffffc00, 0x0000000000000f00, 0x000003ffffffff80, 0x6000600060006000, 0x00000ffe00000ffe, 0x000007ffffffffff, 0x03ffff0003ffff00, 0xfffff0000001ffff, 0xffffff800000007f, 0x1fffffffffc00000, 0xffffe000000000ff, 0x0000000000fffc00, 0xffff000007ffffff, 0x0000000000080000, 0x0000000f0000000f, 0x0000000000e00000, 0x00000ffffffffffc, 0x00000000ffffff80, 0xffff8007ffffffff, 0xfffffff0fffffff0, 0xffff00000000007f, 0x003ffffffffffe00, 0xfffffffffffe001f, 0xffe00000001fffff, 0x003ff80000000000, 0x007fff00007fff00, 0x0000000180000000, 0xfffffff8007fffff, 0xf801ffffffffffff, 0xffffffffe0ffffff, 0xffffffefffffffef, 0x01fffffffffffe00, 0x1ffffffffffffffc, 0xfffff83fffffffff, 0xfffff8007fffffff, 0x3ff800003ff80000, 0x001fffff00000000, 0x007ffc0000000000, 0x00000000003ffc00, 0x00000003fffff000, 0x00f8000000f80000, 0x000001fe000001fe, 0x003ffffffff80000, 0xffffc000000001ff, 0x000000000000ff00, 0xff800001ffffffff, 0xf007ffffffffffff, 0xfffffffff7ffffff, 0xffe0000fffe0000f, 0xffff800000ffffff, 0x00000000001fffff, 0x01fffffffffc0000, 0x000000001fffc000, 0xf8000003ffffffff, 0x00e0000000000000, 0xfffffffffffc0007, 0xffffff800000000f, 0x1ffe1ffe1ffe1ffe, 0x00001ff800000000, 0xf80000000001ffff, 0x7ffffff000000000, 0x00000003fffffe00, 0x0000000001ffffe0, 0xfffffffffffffff7, 0xffffffffffe00001, 0xfffffffffe0000ff, 0xffffff80007fffff, 0x3f80000000000000, 0x00000000ffffffff, 0xc00001ffc00001ff, 0xfffe0000fffe0000, 0x007fffc0007fffc0, 0x001ffffffffffffe, 0xffffff1fffffffff, 0x0000003000000030, 0x0000000003000000, 0x1ffffff81ffffff8, 0xc000000000000fff, 0xfffe0000003fffff, 0x0000001ffe000000, 0x03e0000003e00000, 0xfc7fffffffffffff, 0x00001f8000000000, 0x00000000003ff000, 0xbfffffffbfffffff, 0xfc00000ffc00000f, 0xfffe007ffffe007f, 0xffffe0000000ffff, 0xfff8000000000003, 0x001fffff001fffff, 0xfff0000000000007, 0x0000000ffffc0000, 0xffffe0001fffffff, 0x007ffffffffffffe, 0xefffefffefffefff, 0xfffe000007ffffff, 0xffc0000000001fff, 0xffffe000000fffff, 0x001ffff8001ffff8, 0x00fffc0000000000, 0x0000000000200000, 0xffff8000000000ff, 0x0000000000078000, 0x00ffff0000ffff00, 0xffff800000000000, 0x0000f0000000f000, 0xff00ffffff00ffff, 0x0000003ffe000000, 0xc0000000ffffffff, 0xfff80fffffffffff, 0x000000ff000000ff, 0xfffffc000000000f, 0x003fffffc0000000, 0x0000007fffff0000, 0xfffffc00fffffc00, 0x000001fff8000000, 0xf81ff81ff81ff81f, 0x0000000000000200, 0x000000000003ffe0, 0x3fffffffffffff00, 0xffffffe001ffffff, 0x0000000007fffc00, 0xffffffffffff9fff, 0x00000000007ff800, 0xffff8000000003ff, 0xfffffffe00000000, 0xf803f803f803f803, 0xfff80000fff80000, 0xffc0007fffc0007f, 0x000001ffffffffc0, 0xffe00000003fffff, 0x000000003ffe0000, 0x01fffffffffff800, 0x007fe000007fe000, 0x0003fff800000000, 0xfffffffff0001fff, 0x000003fffffff000, 0xfffffff800000fff, 0x000007fe000007fe, 0x0000000000000004, 0xffffe0000000001f, 0x000000003ffffffe, 0x00007ffffffffff0, 0x0000000000ffffe0, 0x7fffffffffffff80, 0xff000000ff000000, 0x0fe00fe00fe00fe0, 0x00000001ffff8000, 0x000003fffffffff0, 0xffffffff00003fff, 0x0007ffffffffff80, 0x00000000000c0000, 0xfffffffffff007ff, 0x00ffffc000ffffc0, 0xffffc7ffffffffff, 0xfffffff0001fffff, 0x000003f0000003f0, 0x0000ffffff000000, 0xffffffff00007fff, 0x00fffffffffffe00, 0x000fffffffffffff, 0x001ff800001ff800, 0x00000000001f8000, 0x003fffc0003fffc0, 0x000000007ffffc00, 0x3fff800000000000, 0xe00000ffffffffff, 0xfffff0000000007f, 0xffffc000ffffffff, 0x000007fffffc0000, 0x3ffffffc3ffffffc, 0xe000000000ffffff, 0xfffffffff03fffff, 0x03ff800003ff8000, 0x00003fffff000000, 0xffffffe000000fff, 0xfff00000000fffff, 0x7fffffffffe00000, 0x00007e0000007e00, 0xfc00000001ffffff, 0x0001000000000000, 0x00000fffffffc000, 0x007ffffffffffffc, 0x0007fffffffe0000, 0xffff0000000003ff, 0x0000000003fffffe, 0x00ffff0000000000, 0x00007c0000000000, 0xfff8003ffff8003f, 0x3fffffc03fffffc0, 0x0000000c0000000c, 0x8000800080008000, 0x0000f00000000000, 0x000000000001f000, 0xfff0000000000003, 0x00000003fffc0000, 0xffffe003ffffe003, 0x0000004000000000, 0x000003ffffffc000, 0xfffffffc00007fff, 0xfffffffffffffc07, 0xfc07fc07fc07fc07, 0x00001ffffffc0000, 0xffffffffff1fffff, 0xc03fffffffffffff, 0x0000ffc000000000, 0x000000001fffffff, 0xfc00000000003fff, 0xf00000007fffffff, 0x01fffffffffff000, 0xe000ffffe000ffff, 0x00000003ffffc000, 0xc00001ffffffffff, 0xffffffe1ffffffe1, 0x0000003800000000, 0xfffffffffffffc03, 0x7fffffffffffffc0, 0x7fffe0007fffe000, 0x00000fffffffffc0, 0xfffffff000003fff, 0xfffffffffff07fff, 0x00007fff80000000, 0xfff800007fffffff, 0x0001ff800001ff80, 0x0000001fc0000000, 0xc00000000000007f, 0x0000000000007fc0, 0x0ffc000000000000, 0x0000003ffff00000, 0x000007ffff800000, 0x0000000018000000, 0x0c00000000000000, 0x00000000000fffc0, 0x000001fffff80000, 0x2020202020202020, 0x01fffff000000000, 0xfffff80001ffffff, 0x0003ffffe0000000, 0xff8007ffff8007ff, 0x003ffffffffff000, 0xffe0007fffffffff, 0xf0000000001fffff, 0x0000000000003800, 0x000000007fffffc0, 0x7ffffffff0000000, 0x000001ffffc00000, 0x0000003f00000000, 0x0000000000007c00, 0x0001ffffffffffff, 0xffe000000000001f, 0x000001ffffffffe0, 0xf007fffff007ffff, 0xff800fffff800fff, 0xf0003fffffffffff, 0xffffff3fffffffff, 0xfffffffffffffc3f, 0xffffffe07fffffff, 0x8fff8fff8fff8fff, 0xfffffffff000001f, 0xc0ffffffffffffff, 0xffffffffffffe7ff, 0xffffff80ffffff80, 0xff00000000ffffff, 0xefffffffffffffff, 0x07fc000000000000, 0x00003fffff800000, 0xffffc01fffffc01f, 0x0000ffe000000000, 0x1ffff80000000000, 0xf807f807f807f807, 0x000000000003ffc0, 0x1fffffff1fffffff, 0x00000000000007e0, 0xe0000000000fffff, 0x000fffffffff8000, 0x003fe000003fe000, 0xf003ffffffffffff, 0xffffffff800fffff, 0xff8007ffffffffff, 0x07e0000000000000, 0xffffffff807fffff, 0x000000003ffff800, 0x80000000ffffffff, 0x7777777777777777, 0x807fffffffffffff, 0x000000000000ffc0, 0xffffffffffffff9f, 0xfffff80000000007, 0xe0000000000000ff, 0x000007fffffff000, 0xfff8000000000000, 0x000000fffff80000, 0x1ffffffffffffffe, 0xfffe3fffffffffff, 0xfffffc0000000003, 0xc07fffffffffffff, 0x03fffffffff80000, 0xfe000007fe000007, 0xfffff9ffffffffff, 0x0000003ffff80000, 0x00000000ffc00000, 0x07ffc00007ffc000, 0x3fffffffffffff80, 0xf00000003fffffff, 0xffffffff00ffffff, 0x001fffe000000000, 0x007ffffffffffc00, 0xffc000000fffffff, 0x00000000ffff8000, 0xfc00007fffffffff, 0x0000000003fffc00, 0xfc0000000001ffff, 0x000003fffffffe00, 0xffffffff801fffff, 0xfc00001ffc00001f, 0x000003c000000000, 0xfff8000000000007, 0xfffff000000007ff, 0x1ffffc001ffffc00, 0xffffffffc003ffff, 0xffffffffffffff0f, 0x000000000fffffc0, 0xffffff0000003fff, 0xfff0000003ffffff, 0xffffffffffffffe3, 0xfffffffffc03ffff, 0x000000001f800000, 0x7fffffff80000000, 0xfc0000ffffffffff, 0xffffffff8000001f, 0x800000000000007f, 0xfffffff8003fffff, 0xffc001ffffc001ff, 0xfffffc00ffffffff, 0xffffffffff000007, 0xff800001ff800001, 0x1fffe00000000000, 0xffffffffffc00000, 0x00003fffffffff00, 0xfffffffffffe007f, 0x801fffffffffffff, 0xf00000000007ffff, 0x3ffffffff8000000, 0xfffff80fffffffff, 0x001ffe0000000000, 0x00003fe000003fe0, 0x003f003f003f003f, 0x07fff00000000000, 0xfffffffc000fffff, 0xfffffe1fffffffff, 0x3f003f003f003f00, 0xffffff00ffffff00, 0x7ffff0007ffff000, 0xfffc000001ffffff, 0x0000ffffffffff80, 0xfffffffffffffe03, 0x7e0000007e000000, 0x000001f8000001f8, 0x000000000fffe000, 0x001fffffffffff80, 0xc00003ffffffffff, 0x0000000001ffff00, 0x8003800380038003, 0x0000007fffffffff, 0xff800000007fffff, 0xffffffffff0fffff, 0xf0000000ffffffff, 0xffe000000000ffff, 0xffffffc00000007f, 0x7fffffc07fffffc0, 0xffff0003ffffffff, 0xffffffe00fffffff, 0xffff87ffffffffff, 0x0000ff8000000000, 0xff80000000001fff, 0x000000003fffffff, 0x03ffffffffffffff, 0xfffffffe00ffffff, 0x3fffff8000000000, 0x00000003fffffc00, 0x7fc0000000000000, 0xfffffffffff80fff, 0x0000020000000000, 0x0000000007ffffe0, 0x007fffff00000000, 0x00000007fffffffc, 0xfffffffc01ffffff, 0xfffffc00007fffff, 0xffc03fffffc03fff, 0x0700000007000000, 0xffff0000000007ff, 0xffffffffff07ffff, 0xffffffff0001ffff, 0xf80000000000007f, 0xfffffff07fffffff, 0xfe000000000001ff, 0x00001ffffffffc00, 0xfffffc01fffffc01, 0xffc03fffffffffff, 0x7fe000007fe00000, 0xffff0003ffff0003, 0xffc0000000000003, 0xf07fffffffffffff, 0xff00001fff00001f, 0x0001ffff80000000, 0xffffffffc001ffff, 0x00000000000001e0, 0x000000001ff80000, 0x000001ff80000000, 0xfffffffff3ffffff, 0x01fffffff8000000, 0xe00000000000ffff, 0xffffff0000000fff, 0x7ffffffffc000000, 0x0ff0000000000000, 0xf8000ffff8000fff, 0x001ff80000000000, 0xfffc00000000000f, 0x0000070000000700, 0xffffffffc07fffff, 0xffffff000000001f, 0xfffffffe00000001, 0x00000003fff00000, 0xfffe0000007fffff, 0xfe00001fffffffff, 0x000000000000ffff, 0x0001fffe0001fffe, 0x800fffff800fffff, 0xffffff8000001fff, 0x000ffffff8000000, 0x0000000ffffffe00, 0xffffffe000ffffff, 0xffffffe007ffffff, 0xffffffc0000007ff, 0x0000ffffe0000000, 0xff8003ffff8003ff, 0xf000000ff000000f, 0x0080008000800080, 0xf3ffffffffffffff, 0xffffff8000ffffff, 0x0003ffc000000000, 0x00f800f800f800f8, 0xfffffffffffffff1, 0x7fffffffffffffff, 0x0fc0000000000000, 0xffffffffc000001f, 0xfffffcffffffffff, 0x07ff000007ff0000, 0xfffffffffeffffff, 0x0000000000003ffe, 0x0ffffffffe000000, 0x0000fff80000fff8, 0xffffffffc01fffff, 0x0000000000000180, 0xfffffffff81fffff, 0xc3ffffffffffffff, 0xfff03ffffff03fff, 0xff01ff01ff01ff01, 0xc00000007fffffff, 0x00000000007f0000, 0x7e7e7e7e7e7e7e7e, 0xffffffffff80007f, 0x007ffff800000000, 0x8000007f8000007f, 0x3c3c3c3c3c3c3c3c, 0x3fffe0003fffe000, 0x3fffffff00000000, 0xfffffffffff00007, 0xfffffffffe0007ff, 0x000000000003f000, 0xe000000000000001, 0x000000000000fffe, 0x0000400000000000, 0x1fff80001fff8000, 0xffffffffffe01fff, 0xfffffffffe0003ff, 0x00000000001ffe00, 0xffe0000000000007, 0x0003ffff0003ffff, 0xffffffffc03fffff, 0xfffffffffc000000, 0xffff01ffffff01ff, 0x0007ffffffff8000, 0x7ffffffc00000000, 0x7ffffe0000000000, 0x000000007fffffff, 0x87ffffffffffffff, 0x0000000600000006, 0xfffffffffc000fff, 0xffffffffffff7fff, 0x00000ffc00000ffc, 0xffffffc3ffffffff, 0x7ffffffffffffc00, 0x00ffffffff800000, 0xffff8000007fffff, 0xfc00000fffffffff, 0xfff8000003ffffff, 0x00ffffffffff0000, 0xffffffffffffffdf, 0x3ffffffffffffe00, 0x00000ffffff80000, 0xe07fffffffffffff, 0xf000003ff000003f, 0xffff800fffffffff, 0xffe000003fffffff, 0x0000000003ff0000, 0xffffff807fffffff, 0x000007fffc000000, 0x01fff80000000000, 0xffc0000000000001, 0x800000000001ffff, 0x000007f800000000, 0x000003fffffff800, 0x00001fe000000000, 0x0000000800000000, 0x03fffffffffffe00, 0x07ffffffc0000000, 0xfffc000003ffffff, 0xffffe00000ffffff, 0xfffffffdffffffff, 0xff80003fff80003f, 0xf0f0f0f0f0f0f0f0, 0xfffffffffffffffe, 0xffffffc0007fffff, 0xfffffff8000fffff, 0x00fffffffffc0000, 0xffffffc0ffffffc0, 0xffffe0000000000f, 0xf000001fffffffff, 0x001f0000001f0000, 0xfffffffffffffe1f, 0xe003e003e003e003, 0x1000000000000000, 0xffffffffe000003f, 0xff000007ffffffff, 0x0003fffffff80000, 0xffffffffffc0003f, 0x00000000000fffff, 0x7000000070000000, 0x001ffffff0000000, 0xfff80007ffffffff, 0xfffffdffffffffff, 0x3e3e3e3e3e3e3e3e, 0x0003fff000000000, 0xff000000003fffff, 0xf803fffff803ffff, 0xff01ffffff01ffff, 0x0001fff000000000, 0x0000fffff8000000, 0x00003fc000003fc0, 0x0000fffe00000000, 0xffffe0000000007f, 0x0001800000000000, 0x0001ff000001ff00, 0xfffffffffff00003, 0xfffffffcfffffffc, 0xffffffffffe07fff, 0x000ffff8000ffff8, 0xffffffffffff03ff, 0x00000003f8000000, 0xfe00000000007fff, 0x000003fffffffffe, 0x0000007e00000000, 0x07fffffffffe0000, 0xff000000000fffff, 0x00000ffffffffff0, 0x0007ff800007ff80, 0xffffffffffdfffff, 0xf01ffffff01fffff, 0x000007ffc0000000, 0xfc0003ffffffffff, 0xf0000003ffffffff, 0x000000003fffff80, 0x0000000001c00000, 0xfffffff8ffffffff, 0x3fe03fe03fe03fe0, 0x0007fffffffffe00, 0x00000000f0000000, 0xe000001fe000001f, 0xfffffff80000001f, 0xfffffc000000001f, 0x00ffc00000ffc000, 0xffffffffe0007fff, 0x0000080000000800, 0x03f0000000000000, 0x0000000000fffff8, 0x00001ffffffffffe, 0x0010000000100000, 0x000000007ffffff0, 0xfffffffffffffe7f, 0x0800000008000000, 0x0003fffffffffc00, 0x000e000e000e000e, 0xffc0000001ffffff, 0x00001c0000001c00, 0xfc0000000000001f, 0x01ffffff00000000, 0xfffff01fffffffff, 0x0000fc000000fc00, 0xfffffff00000001f, 0x000001fff0000000, 0x0000078000000780, 0x03ffffffff800000, 0x0000fe0000000000, 0x3000000000000000, 0xfffff8000fffffff, 0x0fffff000fffff00, 0x03ffffff03ffffff, 0xfe1ffffffe1fffff, 0xfe000000001fffff, 0xffffffffffffff00, 0xfff9fff9fff9fff9, 0x0000000007ffc000, 0x01fffffffffffc00, 0x07ffff0000000000, 0x000007fffffff800, 0xfffc0003ffffffff, 0x000007fffffffe00, 0xff80000000007fff, 0xffffc000ffffc000, 0x0000000007fffffc, 0x0e00000000000000, 0x00000ffff8000000, 0x0000010000000000, 0x0000003ffffe0000, 0x0000007fffffc000, 0x0000000380000000, 0xffffffc000000007, 0x000003ffffc00000, 0xf000000000003fff, 0xfffffffff0000001, 0x7fff800000000000, 0x00000007f0000000, 0xffe000007fffffff, 0x00000000ffffe000, 0x00000000000ffff8, 0xfffc0000000007ff, 0x1ffffffffffc0000, 0x00003ff800003ff8, 0x7ffe7ffe7ffe7ffe, 0x7ffe00007ffe0000, 0x00007ffffffffe00, 0xff80000000000007, 0x0000001ffffffffe, 0xffffff007fffffff, 0xfffffffffffff3ff, 0xf8000000003fffff, 0x0000000001fffffe, 0xffffff00000007ff, 0x00000fffffffffe0, 0xfffffffffffff001, 0xc000ffffc000ffff, 0xffffe07fffffe07f, 0x0700000000000000, 0xffffffffc0000fff, 0x1fffffffffffffe0, 0x0000000000ff8000, 0xfff000000fffffff, 0x007f8000007f8000, 0xffffcfffffffcfff, 0x07fffc0007fffc00, 0x7800000000000000, 0xfff00000000003ff, 0x0001fffffffe0000, 0xfffffffff800ffff, 0x00003fffc0000000, 0xfffffffffff00001, 0xffffff8007ffffff, 0xff03ffffff03ffff, 0xfffeffffffffffff, 0xfff00000fff00000, 0x0000000000001fe0, 0xfffffffffc0000ff, 0xfff0000ffff0000f, 0xffc07fffffc07fff, 0xffe003ffffe003ff, 0x0000000003f80000, 0x00000001ffff0000, 0xffffc7ffffffc7ff, 0xfffc000000000000, 0x0007fc000007fc00, 0xff8fff8fff8fff8f, 0x000000000007fe00, 0xffc00003ffffffff, 0x0ff000000ff00000, 0x001e001e001e001e, 0xffffffffff0007ff, 0xfffffc0000000fff, 0xffff80000003ffff, 0xf003fffff003ffff, 0xffff000000007fff, 0xfffff0000003ffff, 0xfffffffffffdffff, 0xff8000001fffffff, 0xfffffffff00001ff, 0x0000000000004000, 0x00000000000007f8, 0x0000000007000000, 0xf80003ffffffffff, 0x0020002000200020, 0xfffffffffc00003f, 0xfffffe07ffffffff, 0x000ff80000000000, 0x7fffffffc0000000, 0xe3ffffffffffffff, 0x007ffffffffffff0, 0xffe000000000007f, 0xffff03ffffffffff, 0x00000000000ffffe, 0xffffffc007ffffff, 0x0000000020000000, 0x00000000000000e0, 0x7fffc00000000000, 0xfffffff0000fffff, 0xfffff8003fffffff, 0xfffffffffffffdff, 0x0000000780000000, 0x07ffffffffffe000, 0xfffffff800000001, 0x0000000003ffff00, 0xffbfffffffbfffff, 0xfffff1fffffff1ff, 0xffe0001fffe0001f, 0x7c0000007c000000, 0xfffff07fffffffff, 0x00000000007fffff, 0xffe00001ffe00001, 0x0000ffff0000ffff, 0x0000001e00000000, 0x00007ff000000000, 0x07fffffffffffe00, 0x000000000ffe0000, 0xffff803fffff803f, 0xfff8003fffffffff, 0xe3ffffffe3ffffff, 0x00ffffffc0000000, 0x000000001ff00000, 0xfc0000003fffffff, 0x800000000000001f, 0xffffff8000000007, 0x00000c0000000c00, 0x0000000ffffffff8, 0x0000000000007fff, 0x00003f0000000000, 0x01ffffffffffffc0, 0xffffc000000007ff, 0x000ffe00000ffe00, 0xff87ff87ff87ff87, 0xffffffffffffff1f, 0xf8000000ffffffff, 0x007ffffe007ffffe, 0x00001ffffff80000, 0xf00fffffffffffff, 0x003e0000003e0000, 0x8000000000007fff, 0xffffffffff83ffff, 0xfffff80000000001, 0xffffffffff7fffff, 0xffff0001ffff0001, 0x7ffffff07ffffff0, 0xfc1fffffffffffff, 0x0fffffffffff8000, 0x0007e00000000000, 0xfffffff80003ffff, 0x0001ffffffffff00, 0xfe000003fe000003, 0x0003fffffffffffe, 0xff8000000003ffff, 0x000000000c000000, 0xf83ff83ff83ff83f, 0xfffffe001fffffff, 0x0000000ffffff000, 0xfc00ffffffffffff, 0x800003ff800003ff, 0x00000001f8000000, 0xe0000003ffffffff, 0x03ffffe000000000, 0xffffffffc00003ff, 0xffe07fffffe07fff, 0xf0007ffff0007fff, 0x000001f800000000, 0x000000fe00000000, 0xfff83fffffffffff, 0xf7fff7fff7fff7ff, 0xffffff000000003f, 0xfcfffcfffcfffcff, 0x0010000000000000, 0xfffff80000ffffff, 0xffffff803fffffff, 0xffffc07fffffc07f, 0xfffc01fffffc01ff, 0x000fffffff000000, 0x0000007ffffffe00, 0x000000007fe00000, 0x0fffffffffff0000, 0x00000003c0000000, 0xffff07ffffffffff, 0x001fffffffe00000, 0xf800000000000007, 0x0001ffffffffff80, 0xc00000000000003f, 0x1ff000001ff00000, 0x003e003e003e003e, 0xffffffffffe00003, 0x00fffff000000000, 0xffffe00007ffffff, 0x001fffe0001fffe0, 0xfffff807ffffffff, 0x000000ffff800000, 0xfc0000000fffffff, 0x00007ffffffc0000, 0x0000006000000060, 0x0001fffc00000000, 0x00000007fffffff8, 0x0ffffffffffff800, 0x0001ffffffff8000, 0xffff87ffffff87ff, 0xffc000000000003f, 0xfffffffffffc000f, 0x0001fffffffffffe, 0x00000007ff800000, 0x07ffffffff000000, 0xfff000000000000f, 0x0000800000008000, 0xff801fffff801fff, 0xfffff0000000001f, 0x0003c00000000000, 0xffffc0000000007f, 0x0000000000001fff, 0xc000ffffffffffff, 0x0000000000c00000, 0xffe00007ffe00007, 0x007c000000000000, 0x00c0000000c00000, 0x00007f8000007f80, 0xe0007fffffffffff, 0xffffffffc000003f, 0xffffffff7fffffff, 0x000fffffffff0000, 0x00ff00ff00ff00ff, 0x0fffffe000000000, 0xffffffff0000007f, 0xc00000000fffffff, 0x0000000000000c00, 0xffc0000000003fff, 0x0000003fffff8000, 0x00000000000ff800, 0xffc00000000003ff, 0x00fffffffff80000, 0xfffffc03ffffffff, 0x00000000ffffffe0, 0x03fffffffe000000, 0xffffe3ffffffffff, 0xffffffffe0000007, 0xfe00000ffe00000f, 0x0000003f80000000, 0x000001ffe0000000, 0x0002000000000000, 0xfffffe0000000fff, 0xe1ffe1ffe1ffe1ff, 0xfffffffffffffeff, 0xf000000000ffffff, 0x00001ffff8000000, 0x5555555555555555, 0x07fff00007fff000, 0x00003fffffffffc0, 0x0001ffffff800000, 0xffffff0000000001, 0xffe0000003ffffff, 0xffc0000fffffffff, 0x001ffffffffffff8, 0xff000003ffffffff, 0xffffffffff807fff, 0xffffc00000000007, 0xfe000000fe000000, 0x00fffffffc000000, 0x80000003ffffffff, 0xfffffffc07ffffff, 0xe0000000ffffffff, 0x00003ffffffffe00, 0x000fe00000000000, 0xfefffffffeffffff, 0x007f800000000000, 0xfffc001ffffc001f, 0x00000fffffe00000, 0x0003e0000003e000, 0x007fffe000000000, 0x00000000001ffff0, 0xfffffff0000001ff, 0x0000000000000fc0, 0x0000000700000000, 0x0ffffff00ffffff0, 0xfffff0000000ffff, 0x3ffffffffe000000, 0xffc00000000000ff, 0x00000000001fc000, 0xf8fffffff8ffffff, 0xc000000fc000000f, 0x7ffffff87ffffff8, 0x0001ffffffe00000, 0xffffff1fffffff1f, 0x000000ffffffff80, 0xffffffff80000007, 0xfffff80000000fff, 0x000000000fffffe0, 0x80003fff80003fff, 0x00001ffffffe0000, 0xffffffffffffe1ff, 0x3fffffff80000000, 0x001ffe00001ffe00, 0x00003ffffffffc00, 0xffffe01fffffffff, 0xfffffffffff83fff, 0x7f007f007f007f00, 0xfff00000000000ff, 0xffe0000fffffffff, 0x00fffff000fffff0, 0x00000f8000000000, 0xffe000000000003f, 0xfffe003ffffe003f, 0x1fffffff80000000, 0x003fff8000000000, 0xfffc01ffffffffff, 0xffffc000000003ff, 0x0000000030000000, 0x7c007c007c007c00, 0xffffff000007ffff, 0xff80000000000001, 0xffffffff80000000, 0x0000000003ffffc0, 0xf9fffffff9ffffff, 0xffffffffe001ffff, 0x0003fffffffffe00, 0xf800ffffffffffff, 0xffffc0000003ffff, 0x000003ffffffff00, 0xffffc00fffffc00f, 0xfe0001ffffffffff, 0x0001fffc0001fffc, 0x000007f0000007f0, 0xfffe07ffffffffff, 0xfffffffff0000000, 0xfff8000001ffffff, 0xfffe0007ffffffff, 0xfffff00fffffffff, 0x0000000fffffffff, 0xfffe00ffffffffff, 0x3fffffffffffc000, 0xfffc00000fffffff, 0x000000001ffffff8, 0xfc000000000000ff, 0xfffe00000001ffff, 0xffffffffe000001f, 0xc001ffffc001ffff, 0xffc01fffffc01fff, 0xe00007ffffffffff, 0xffffffe000000007, 0x0ffffffffff00000, 0xffff1fffffffffff, 0x00007fffc0000000, 0x1fffff001fffff00, 0xff0001ffffffffff, 0xffffffff000003ff, 0xffe3ffe3ffe3ffe3, 0xffff000000000003, 0xfffffff1fffffff1, 0xff000fffffffffff, 0x01fffffc01fffffc, 0x00f0000000f00000, 0xfffffffe000003ff, 0x0006000000000000, 0xfffffffffcffffff, 0x0000010000000100, 0xf8000000000007ff, 0x00001fffffffffff, 0x0000000060000000, 0x00001fffffffffe0, 0x000000007ffffe00, 0xc00fffffc00fffff, 0x80000007ffffffff, 0x0000fffffff80000, 0x00007ffffffff800, 0xf8fff8fff8fff8ff, 0x0ffffffffffffffe, 0x800000000000000f, 0x000000c000000000, 0xfffffffe0000003f, 0x0000000003800000, 0x0000fffffffffffe, 0xffff007fffff007f, 0x0008000000000000, 0x03ffc00003ffc000, 0xffffffffffe00fff, 0x0101010101010101, 0xc00000ffffffffff, 0x0000004000000040, 0x03fffffe03fffffe, 0x0000000000038000, 0xf07ffffff07fffff, 0x00000000007e0000, 0x00003ffc00003ffc, 0xffffff801fffffff, 0xffffffffff8fffff, 0xff80000001ffffff, 0xfffffffffffff07f, 0x0f8000000f800000, 0x803fffff803fffff, 0xe00007ffe00007ff, 0x07fffffe07fffffe, 0xfff3fffffff3ffff, 0x000000000003ffff, 0x0000000700000007, 0x0000007800000078, 0xffff7fffffffffff, 0xffffffe7ffffffe7, 0x00007fffffffff00, 0x800001ff800001ff, 0x00007ffffffffc00, 0x0303030303030303, 0x000000000000c000, 0xfffffffffffc1fff, 0x00001ffffffffffc, 0xfffffff0007fffff, 0xffffffffe0003fff, 0xffff9fffffffffff, 0xff800000003fffff, 0xffffbfffffffbfff, 0xfc1ffc1ffc1ffc1f, 0x01fe01fe01fe01fe, 0x000000000000e000, 0xfff00001ffffffff, 0x0fffc0000fffc000, 0x1ffffff000000000, 0x00007ffffff80000, 0x0000600000006000, 0x00000000000f8000, 0x0c0c0c0c0c0c0c0c, 0xf8ffffffffffffff, 0x007fffffffff0000, 0xffffffffc7ffffff, 0xe00fe00fe00fe00f, 0x0000000fffffffc0, 0x8000000003ffffff, 0x007fffff007fffff, 0x00000000007ffc00, 0xffffffe00007ffff, 0xffff00ffffffffff, 0xff87ffffffffffff, 0xffdfffffffffffff, 0x03ffff0000000000, 0x0000380000003800, 0xffff001fffffffff, 0x03ffffffe0000000, 0x00003fffffffff80, 0x000000003ff80000, 0x800000000000003f, 0x00007f8000000000, 0x0f800f800f800f80, 0xe00000003fffffff, 0x80000001ffffffff, 0x1fc01fc01fc01fc0, 0xfffff80000000003, 0xfff0fff0fff0fff0, 0xffffffff3fffffff, 0xfc00000000007fff, 0x000007fffffffffc, 0xfc000000000fffff, 0xffffffff8000000f, 0x00001fffffffe000, 0x0001f00000000000, 0x00000001fffffff8, 0xffffffc00003ffff, 0xff7fff7fff7fff7f, 0xffffffffff00000f, 0x007ffff000000000, 0x007ffffff8000000, 0x0000000000001ffe, 0x000000fffffffff8, 0xfffffffffe7fffff, 0x0000000f00000000, 0x03e0000000000000, 0xfffffffffff803ff, 0x00001fc000000000, 0xe000007fe000007f, 0xffffffffffffc03f, 0x0000fffffff00000, 0x00000000000000f0, 0x00000000ffe00000, 0xfff8000000001fff, 0x8000000780000007, 0x0003fc0000000000, 0xffffff03ffffffff, 0x00000000007ffffc, 0xfffffffffff801ff, 0xff800000ff800000, 0xfffe00000007ffff, 0x0fffffffffffffe0, 0xc01fffffc01fffff, 0xfc007ffffc007fff, 0x0040000000000000, 0x0000000001fc0000, 0x00000000fffff000, 0xffffffffffc3ffff, 0xffffffe000001fff, 0xc1ffffffffffffff, 0x0000000fffff8000, 0x003fffe0003fffe0, 0x0001c0000001c000, 0x000000003e000000, 0x00ffffffffc00000, 0xfffff81ffffff81f, 0x80000000000003ff, 0xfffc00fffffc00ff, 0x0000007f00000000, 0x00ffc00000000000, 0xffc0000007ffffff, 0xfff01fffffffffff, 0x07ffffffffc00000, 0x007fffffffffffc0, 0x1ffffffffffff000, 0x0007fffffffff800, 0xfe00000003ffffff, 0x0ff8000000000000, 0x0000fffc0000fffc, 0xffe000ffffe000ff, 0x00ffffff80000000, 0xfffffffffff0000f, 0x0000000001e00000, 0xffffc03fffffc03f, 0xfe0000000000ffff, 0x0ffffff800000000, 0xfffff800007fffff, 0x0007fe0000000000, 0x000ff800000ff800, 0x0001000000010000, 0xfffffffcffffffff, 0x00003ffffff80000, 0x000007ff00000000, 0xffffffffffffe003, 0xffffc000000000ff, 0x0001fffffff80000, 0x0007c0000007c000, 0x000003fffff80000, 0x000000000000000c, 0x0000000e0000000e, 0x00007fffffe00000, 0xffffffffffff800f, 0xffffffc000ffffff, 0xfff000000003ffff, 0xfdfdfdfdfdfdfdfd, 0xfbfffbfffbfffbff, 0xf001fffff001ffff, 0x1f801f801f801f80, 0xfffffffe00000fff, 0x7f00000000000000, 0x0000fffffffffe00, 0x000000000003fffc, 0x00000003fff80000, 0x00000ffffe000000, 0x000fffffffffff00, 0x0001fffffffffe00, 0x3c00000000000000, 0xffffffffffffff80, 0xffff00000007ffff, 0xffff0007ffff0007, 0x07ffffffffffffc0, 0xfffffffc0003ffff, 0x0ff00ff00ff00ff0, 0x03ffc00000000000, 0xfffff801ffffffff, 0xfff00007ffffffff, 0xff800007ffffffff, 0x001ffffc00000000, 0xfc00fffffc00ffff, 0x1ffffffffff80000, 0xfffffffc000000ff, 0x0007ffffffffffff, 0xf00001fff00001ff, 0x0000fc0000000000, 0xffc0003fffc0003f, 0xfffe01fffffe01ff, 0x0000300000003000, 0x00007ffffffe0000, 0x0000000000007fe0, 0xffffffc000000000, 0x0000007ffff80000, 0xfffffc0fffffffff, 0xfc0000001fffffff, 0x03ff800000000000, 0xffff800000001fff, 0x00000000001e0000, 0xfe0fffffffffffff, 0xffffffffffffc1ff, 0x003ff800003ff800, 0xffffffcfffffffcf, 0x0fffffffffc00000, 0x00007fffff800000, 0x00003fffffff8000, 0xffffffffffcfffff, 0xffffffffffffff8f, 0x000003ffff000000, 0xfff0000000ffffff, 0xfffffc0000003fff, 0xffffff8000000003, 0xfffc7ffffffc7fff, 0xff0003ffff0003ff, 0xfffffffdfffffffd, 0xffffc00000000003, 0xff0007ffff0007ff, 0x00001fffffffff00, 0xffc00000000007ff, 0xff800007ff800007, 0xfffe003fffffffff, 0xfffffffffffeffff, 0xfffffffc00000003, 0xfffffffffffc0000, 0x00ffffe000000000, 0xff00000007ffffff, 0xfffc7fffffffffff, 0xffffffff80000001, 0x01ffffffc0000000, 0x1ffc1ffc1ffc1ffc, 0xfffffffff8000007, 0xe0000000000003ff, 0xf800000003ffffff, 0x1fff00001fff0000, 0x0000000003ffc000, 0xffe00fffffe00fff, 0xfffffffffffe3fff, 0x1ffffffffffffff8, 0xffe003ffffffffff, 0x00000ff800000000, 0xfe0000000000003f, 0xffffffffffff3fff, 0x00000000001fffc0, 0xfc00000000000fff, 0xffe00000000fffff, 0x7ffffffffffc0000, 0xfffffff0003fffff, 0x000001fe00000000, 0xfffc0000000000ff, 0xffffff07ffffff07, 0x3fffffffffffe000, 0xffffff87ffffffff, 0x01ff800001ff8000, 0xfffffffffffff807, 0xff800000000007ff, 0x00000000fffff800, 0x07fffffffc000000, 0x07fe07fe07fe07fe, 0xfc0001ffffffffff, 0x0fffffffffffe000, 0xffc000000003ffff, 0xfefefefefefefefe, 0x000001c000000000, 0x0000001ffffff000, 0x0ffffffffffffff0, 0xfffc07fffffc07ff, 0x007ffffffffff000, 0x000007c000000000, 0xfffe000000000003, 0x0000000000ffffc0, 0x7ffff00000000000, 0x007fff80007fff80, 0xffffffffffe000ff, 0x000000000000ff80, 0x007fffffe0000000, 0x0000000000007ff8, 0x00000000000007f0, 0xfffc0000003fffff, 0x0007800000000000, 0x0000003fffffffff, 0xffff0001ffffffff, 0x00000007ffffff80, 0x0000003ff8000000, 0xffff7fffffff7fff, 0xfffc000000001fff, 0x83ffffff83ffffff, 0xe007ffffffffffff, 0x0000001ff8000000, 0xfffffffffff0007f, 0xc7ffc7ffc7ffc7ff, 0x0000001ffffffe00, 0x00fffe0000fffe00, 0x7ffffffffffffffc, 0xffe0000000000003, 0x000000000007f800, 0x001e000000000000, 0x7fffffc000000000, 0xfffffe00000007ff, 0x0000001ffffffff8, 0x000000003ffffe00, 0xc0000000c0000000, 0x001fffff80000000, 0xf00000000fffffff, 0x0707070707070707, 0xfe03fe03fe03fe03, 0xff80ff80ff80ff80, 0x7ffff80000000000, 0x0ff800000ff80000, 0x00000ffffffffc00, 0x3800000000000000, 0x00000007fff80000, 0xc0000000000003ff, 0xfffffe00000fffff, 0x3ff0000000000000, 0x0000000001f00000, 0x000000000ffff800, 0x01ffffffffffffff, 0xfffffffe00007fff, 0xc7c7c7c7c7c7c7c7, 0xffffffffc0000007, 0xffffffff87ffffff, 0x1fffffffffff8000, 0x0ffff00000000000, 0x0000000080000000, 0x0000007ffffc0000, 0x0000e00000000000, 0xffe3ffffffe3ffff, 0x00000003ffffff00, 0xfffff87fffffffff, 0x00000000007fffe0, 0xffff8000003fffff, 0xfffefffefffefffe, 0xffe01fffffe01fff, 0x7ffffffffffffffe, 0x00000000000001f0, 0x007ffffffffff800, 0x0000000001fffffc, 0x00000003ffff0000, 0x00000ffffffff000, 0x0000000000800000, 0xffc000003fffffff, 0x0000000800000008, 0xf0007fffffffffff, 0x0000007fff000000, 0x0000000003fffff8, 0x00000000000007fe, 0x0000000007fff800, 0x00000000007ffffe, 0xfffe00000000007f, 0x07ffff8007ffff80, 0x3ffe00003ffe0000, 0xffff0fffffffffff, 0xffc003ffffc003ff, 0x00000ffc00000000, 0xffffffffffc07fff, 0x00000000000fff00, 0x0000000000000007, 0xfcffffffffffffff, 0x07fffffe00000000, 0xfffffffc00000007, 0x07fffffffffff800, 0xffffff003fffffff, 0xffffff800001ffff, 0xfffffffe000001ff, 0x0001fe000001fe00, 0x1f00000000000000, 0xff800000000001ff, 0xfffffffbffffffff, 0xfffffff00000ffff, 0xf8000001f8000001, 0xffffffc0001fffff, 0xfffffc0ffffffc0f, 0x007e007e007e007e, 0x001ffffffff80000, 0x807f807f807f807f, 0x01fc000000000000, 0xffe0ffffffffffff, 0xfeffffffffffffff, 0xffe00000000007ff, 0x0060000000600000, 0x0000000000001ff8, 0x00003ffffffe0000, 0x000000000ffffffe, 0xffff00000fffffff, 0x1fffc00000000000, 0x000ffffffffc0000, 0x0007ffffffffffe0, 0x00000000000007c0, 0x000000003fffe000, 0x0000000000000ff0, 0x07ffffffff800000, 0xfffffffe0007ffff, 0xfffffffffe00000f, 0xffffc00000000000, 0x000000000003fe00, 0xffffffffe00001ff, 0xf0000000000003ff, 0x000000000000007f, 0x0001fffff8000000, 0x00000001ffffff00, 0xffffffe00000003f, 0x0fffffffc0000000, 0x00000001ff000000, 0x7ffffffc7ffffffc, 0xe0003fffe0003fff, 0xfe00000000000003, 0x003ffffe00000000, 0xfffc3ffffffc3fff, 0x0007000700070007, 0x03fffffffffff800};
    // 将数组元素存入哈希集合（自动去重，查找时间 O(1)）
    static std::unordered_set<uint64_t> target_set(
        target_nums, 
        target_nums + sizeof(target_nums) / sizeof(target_nums[0])  // 计算数组长度
    );
    // 检查 imm 是否在集合中
    return target_set.find(imm) != target_set.end();
}
