#include <string>
#include <iostream>
#include <bitset>
#include <cstring>

#include "../../include/ir/koopa.h"
#include "../../include/back/koopa_print.hpp"

//该文件用来输出koopa中各种数据结构
extern std::stack<std::string> reg_stack;
extern std::stack<std::string> sreg_stack;
extern std::string sp_reg;
extern const std::string zero_reg;
extern uint16_t fimm_low,fimm_high;
extern uint32_t fimm_32;
extern bool binary_for_br;
extern int cout_len,total_ldr_times,total_str_times,total_mov_times,total_add_times,size_of_stack_frame,FRAME_SIZE_OF_R_REGS,FRAME_SIZE_OF_S_REGS;
//打印指令的tag类型
std::string TagToString(koopa_raw_value_tag_t tag) {
    switch (tag) {
        case KOOPA_RVT_INTEGER: return "KOOPA_RVT_INTEGER";
        case KOOPA_RVT_FLOATNUM: return "KOOPA_RVT_FLOATNUM";
        case KOOPA_RVT_ZERO_INIT: return "KOOPA_RVT_ZERO_INIT";
        case KOOPA_RVT_UNDEF: return "KOOPA_RVT_UNDEF";
        case KOOPA_RVT_AGGREGATE: return "KOOPA_RVT_GGREGATE";
        case KOOPA_RVT_FUNC_ARG_REF: return "KOOPA_RVT_FUNC_ARG_REF";
        case KOOPA_RVT_BLOCK_ARG_REF: return "KOOPA_RVT_BLOCK_ARG_REF";
        case KOOPA_RVT_ALLOC: return "KOOPA_RVT_ALLOC";
        case KOOPA_RVT_GLOBAL_ALLOC: return "KOOPA_RVT_GLOBAL_ALLOC";
        case KOOPA_RVT_LOAD: return "KOOPA_RVT_LOAD";
        case KOOPA_RVT_STORE: return "KOOPA_RVT_STORE";
        case KOOPA_RVT_GET_PTR: return "KOOPA_RVT_GET_PTR";
        case KOOPA_RVT_GET_ELEM_PTR: return "KOOPA_RVT_GET_ELEM_PTR";
        case KOOPA_RVT_BINARY: return "KOOPA_RVT_BINARY";
        case KOOPA_RVT_BRANCH: return "KOOPA_RVT_BRANCH";
        case KOOPA_RVT_JUMP: return "KOOPA_RVT_JUMP";
        case KOOPA_RVT_CALL: return "KOOPA_RVT_CALL";
        case KOOPA_RVT_RETURN: return "KOOPA_RVT_RETURN";
        default: return "UNKNOWN";
    }
}

std::string type_TagToString(koopa_raw_type_tag_t tag) {
    switch (tag) {
        case KOOPA_RTT_INT32: return "KOOPA_INT32";
        case KOOPA_RTT_UNIT: return "KOOPA_UNIT";
        case KOOPA_RTT_ARRAY: return "KOOPA_ARRAY";
        case KOOPA_RTT_POINTER: return "KOOP_POINTER";
        case KOOPA_RTT_FUNCTION: return "KOOPA_FUNCTION";
        default: return "UNKNOWN";
    }

}
//std::string precise_float(float number);
//浮点数在koopa ir中以2进制形式输出
void float2uint(float num) {
    uint32_t asInt;
    memcpy(&asInt, &num, sizeof(num)); // 将 float 转换为 uint32_t
    std::bitset<32> binary(asInt); // 获取 32 位二进制形式
    fimm_low = asInt & 0xFFFF;       // 提取低 16 位
    fimm_high = (asInt >> 16) & 0xFFFF; // 提取高 16 位
}
void float2uint32(float num) {
    uint32_t asInt;
    memcpy(&asInt, &num, sizeof(num)); // 将 float 转换为 uint32_t
    std::bitset<32> binary(asInt); // 获取 32 位二进制形式
    fimm_32 =asInt ; // 提取32 位
}
void printStack(std::stack<std::string> reg_stack) {
    // 创建一个临时栈来存储弹出的元素
    std::stack<std::string> temp_stack;
    //std::cout << "@reg_stack:";
    // 弹出所有元素并存储在临时栈中，同时打印它们
    while (!reg_stack.empty()) {
        std::string top = reg_stack.top();
        std::cout << top <<",";
        temp_stack.push(top);
        reg_stack.pop();
    }
    std::cout <<std::endl;
    // 恢复原来的栈
    while (!temp_stack.empty()) {
        reg_stack.push(temp_stack.top());
        temp_stack.pop();
    }
}

void print_stack_map(const std::map<koopa_raw_value_t, int>& stack_map) {
    std::cout << "@Stack Map (value -> offset):" << std::endl;
    for (const auto& entry : stack_map) {
        if(entry.first->name)
        std::cout <<"@" <<TagToString(entry.first->kind.tag) <<"->"<<entry.first->name<<" -> " << entry.second << std::endl;
    }
}

void mov_print(std::string dstreg, int64_t imm, std::string srcreg, bool imm2reg, std::string cond)
{
    // 跳过相同寄存器的移动（仅对寄存器到寄存器有效）
    if (!imm2reg && dstreg == srcreg) return;
    
    bool is64bit = (dstreg[0] == 'x' || dstreg[0] == 'X');
    total_mov_times += 1;

    if(imm2reg)
    {
        const std::string tmp_reg = reg_stack.top(); // 临时寄存器
        if (tmp_reg!=dstreg){
            std::cout << std::setw(cout_len) << "mov " << "x19" << ", " << tmp_reg << std::endl;
        }
        reg_stack.pop();
        uint64_t val = static_cast<uint64_t>(imm); // 转换为无符号，处理补码
        
        // 提取4个16位段（分别对应0、16、32、48位偏移）
        uint16_t parts[4] = {
            static_cast<uint16_t>(val & 0xFFFF),
            static_cast<uint16_t>((val >> 16) & 0xFFFF),
            static_cast<uint16_t>((val >> 32) & 0xFFFF),
            static_cast<uint16_t>((val >> 48) & 0xFFFF)
        };

        // 用MOVZ加载最低16位（如果非零）
        if (parts[0] != 0) {
            std::cout << std::setw(cout_len) << "movz " << tmp_reg << ", #" << parts[0] << std::endl;
        } else {
            // 如果最低16位为0，先用movz清零（避免寄存器原有值干扰）
            std::cout << std::setw(cout_len) << "movz " << tmp_reg << ", #0" << std::endl;
        }

        // 用MOVK加载其余16位段（如果非零）
        if (parts[1] != 0) {
            std::cout << std::setw(cout_len) << "movk " << tmp_reg << ", #" << parts[1] << ", lsl #16" << std::endl;
        }
        if (parts[2] != 0) {
            std::cout << std::setw(cout_len) << "movk " << tmp_reg << ", #" << parts[2] << ", lsl #32" << std::endl;
        }
        if (parts[3] != 0) {
            std::cout << std::setw(cout_len) << "movk " << tmp_reg << ", #" << parts[3] << ", lsl #48" << std::endl;
        }

        // 条件移动到目标寄存器
        if(cond.empty()){
            std::cout << std::setw(cout_len) << "mov " << dstreg << ", " << tmp_reg << std::endl;
        } else {
            std::cout << std::setw(cout_len) << "csel " << dstreg << ", " << tmp_reg << ", " << dstreg << ", " << cond << std::endl;
        }
        if (tmp_reg!=dstreg){
            std::cout << std::setw(cout_len) << "mov " << tmp_reg << ", " << "x19" << std::endl;
            std::cout << std::setw(cout_len) << "mov " << "x19" << ", #0" << std::endl;
        }
        reg_stack.push(tmp_reg);

    }
    else 
    {
        // 寄存器到寄存器的移动（保持原逻辑）
        if(cond.empty()){
            std::cout << std::setw(cout_len) << "mov " << dstreg << ", " << srcreg << std::endl;
        } else {
            std::cout << std::setw(cout_len) << "csel " << dstreg << ", " << srcreg << ", " << dstreg << ", " << cond << std::endl;
        }
    }
}



//打印add语句，arg1是目的寄存器r0，arg2是源寄存器r1，arg3是立即数，arg4是源寄存器r2，没有改参数则置""
//可以处理正/负立即数，但绝对值必须小于0xff，否则扩充语句
// 判断一个立即数是否在64位ADD/SUB指令的有效范围内

void add_print(std::string r0, std::string r1, int64_t imm, std::string r2, bool addimm, std::string cond) 
{
    total_add_times += 1;
    
    // 判断寄存器大小
    bool is64bit = (r0[0] == 'x' || r0[0] == 'X');
    const std::string tmp_reg = reg_stack.top();
    reg_stack.pop();
    if (addimm){
         // 临时寄存器（假设未被占用）
        // 检查是否在有效立即数范围内
        if (isopimm(imm, is64bit)) {
            if (imm >= 0) {
                if (imm<=0xFFF){
                // 直接使用ADD指令
                std::cout << std::setw(cout_len) << "add" << " " << tmp_reg << ", " << r1 << ", #" << imm << std::endl;
                }
                else{
                    std::cout << std::setw(cout_len) << "add"<< " " << tmp_reg << ", " << r1 << ", #" << (imm>>12)<< ", lsl #12" << std::endl;
                }
            } else {
                uint64_t abs_imm = imm < 0 ? static_cast<uint64_t>(-imm) : static_cast<uint64_t>(imm);
                if (abs_imm<=0xFFF){
                // 使用SUB指令处理负值
                std::cout << std::setw(cout_len) << "sub" << " " << tmp_reg << ", " << r1 << ", #" << abs_imm << std::endl;
                }
                else{
                    std::cout << std::setw(cout_len) << "sub" << " " << tmp_reg << ", " << r1 << ", #" << (abs_imm>>12)<< ", lsl #12" << std::endl;
                }
            }
        }
        else {
            // 处理大立即数
            if(imm>0){
            if (r0 == r1) {
                std::string temp_reg = reg_stack.top();
                reg_stack.pop();
                // 将立即数加载到临时寄存器
                mov_print(temp_reg, imm, "", true, cond);
                // 执行寄存器加法
                if(r1=="sp"){
                    r1=reg_stack.top();
                    reg_stack.pop();
                    std::cout << std::setw(cout_len) << "mov " << r1 << ", " << "sp" << std::endl;
                    reg_stack.push(r1);
                }
                std::cout << std::setw(cout_len) << "add" << " " << tmp_reg << ", " << r1 << ", " << temp_reg << std::endl;
                
                reg_stack.push(temp_reg);
            } 
            else{
                // 将立即数加载到目标寄存器
                mov_print(tmp_reg, imm, "", true, cond);
                // 执行加法
                if(r1=="sp"){
                    r1=reg_stack.top();
                    reg_stack.pop();
                    std::cout << std::setw(cout_len) << "mov " << r1 << ", " << "sp" << std::endl;
                    reg_stack.push(r1);
                }
                std::cout << std::setw(cout_len) << "add" << " " << tmp_reg << ", " << tmp_reg << ", " << r1 << std::endl;
            }}
            else{
                if (r0 == r1) {
                std::string temp_reg = reg_stack.top();
                reg_stack.pop();
                // 将立即数加载到临时寄存器
                mov_print(temp_reg, -imm, "", true, cond);
                // 执行寄存器加法
                if(r1=="sp"){
                    r1=reg_stack.top();
                    reg_stack.pop();
                    std::cout << std::setw(cout_len) << "mov " << r1 << ", " << "sp" << std::endl;
                    reg_stack.push(r1);
                }
                std::cout << std::setw(cout_len) << "sub" << " " << tmp_reg << ", " << r1 << ", " << temp_reg << std::endl;
                
                reg_stack.push(temp_reg);
            } 
            else{
                // 将立即数加载到目标寄存器
                mov_print(tmp_reg, -imm, "", true, cond);
                // 执行加法
                if(r1=="sp"){
                    r1=reg_stack.top();
                    reg_stack.pop();
                    std::cout << std::setw(cout_len) << "mov " << r1 << ", " << "sp" << std::endl;
                    reg_stack.push(r1);
                }
                std::cout << std::setw(cout_len) << "sub" << " " << tmp_reg << ", " << r1 << ", " << tmp_reg << std::endl;
            }
        }}
       if(cond==""){
            std::cout << std::setw(cout_len) << "mov " << r0 << ", " << tmp_reg << std::endl;
        }
        else{
        std::cout << std::setw(cout_len) << "csel " << r0 << ", " << tmp_reg << ", " << r0 << ", " << cond << std::endl;
        }
        reg_stack.push(tmp_reg);}
    else {
        // 寄存器到寄存器加法
        std::cout << std::setw(cout_len) << "add" << " " << tmp_reg << ", " << r1 << ", " << r2 << std::endl;
        if(cond==""){
            std::cout << std::setw(cout_len) << "mov " << r0 << ", " << tmp_reg << std::endl;
        }
        else{
        std::cout << std::setw(cout_len) << "csel " << r0 << ", " << tmp_reg << ", " << r0 << ", " << cond << std::endl;
        }
        reg_stack.push(tmp_reg);
    }
}

//打印and/orr/eor语句
//可以处理正/负立即数，但绝对值必须小于0xff，否则扩充语句
void bit_cal_print(std::string r0, std::string r1, uint64_t imm, std::string r2, bool useimm, const koopa_raw_binary_op_t op) {
    std::string opcode;
    switch (op) {
        case KOOPA_RBO_AND:
            opcode = "and";
            break;
        case KOOPA_RBO_OR:
            opcode = "orr";
            break;
        case KOOPA_RBO_XOR:
            opcode = "eor";
            break;
        default:
            return; // 不支持的操作码直接返回
    }

    if (useimm) {
        // 检查立即数是否可以用AND/ORR/EOR的立即数形式表示
        if (islogicimm(imm)) {
            std::cout << std::setw(cout_len) << opcode  << " " << r0 << ", " << r1 << ", #" << imm << std::endl;
        }
        else {
            mov_print(r0, imm, "", true, "");
            bit_cal_print(r0,r1,0,r0,0,op);
        }
    } else {
        std::cout << std::setw(cout_len) << opcode << r0 << ", " << r1 << ", " << r2 << std::endl;
    }
}

//打印str ldr语句
//可以处理正/负立即数，但范围在-2048到2047，否则扩充语句
//type 1是带立即数偏移的寄存器间接寻址；type 2是带寄存器偏量的寄存器间接寻址；type 3是寄存器间接寻址，type 4是前索引寻址，type 5是后索引寻址
//如果目的寄存器是‘s’,则当作浮点访存
void str_print(std::string r0,std::string r1,int imm,std::string r2,int find_type)
{
    total_str_times+=1;
    std::string str_str="str";
        if(r1=="wsp"){
        r1="sp";
    }
    if(r1[0]=='w'){
        r1[0]='x';
    }
    if(find_type==1)// STR R0, [R1, #imm] 
    {
        if(imm==0)std::cout << std::setw(cout_len) << str_str << r0 << ", " << "["<<r1<< "]"<< std::endl;
        else if(((r0[0]=='x')||(r0[0]=='s'))&&(imm<=16380&&imm>=0&&(imm%4==0)))std::cout << std::setw(cout_len) << str_str<< r0 << ", " << "["<<r1<<", #"<< imm<< "]"<< std::endl;
        else if((r0[0]=='x')&&(imm<=32760&&imm>=0&&(imm%8==0)))std::cout << std::setw(cout_len) << str_str<< r0 << ", " << "["<<r1<<", #"<< imm<< "]"<< std::endl;
        else
        {
            std::string temp_reg = reg_stack.top();
            reg_stack.pop();
            add_print(temp_reg,sp_reg,imm,"",true,"");
            str_print(r0,temp_reg,0,"",3); 
            reg_stack.push(temp_reg);
        }
    }
    if(find_type==2)// STR R0, [R1, R2] 
    {
        std::cout << std::setw(cout_len) << str_str<< r0 << ", " << "["<<r1<<", "<< r2<< "]"<< std::endl;
    }
    if(find_type==3)// STR R0, [R1] 
    {
        std::cout << std::setw(cout_len) << str_str << r0 << ", " << "["<<r1<< "]"<< std::endl;
    }
    if(find_type==4){// STR R0, [R1, #imm]!: imm<=255&&imm>=-256
        // 输出地址计算指令：R1 = R1 + imm
        std::cout << std::setw(cout_len) << "add" << " " << r1 << ", " << r1 << ", #" << imm << std::endl;
        // 输出无回写的存储指令：将 R0 存储到 [R1]
        std::cout << std::setw(cout_len) << "str" << " " << r0 << ", " << "[" << r1 << "]" << std::endl;
    }
    if(find_type==5){// STR R0, [R1], #imm
        std::cout << std::setw(cout_len) << str_str<< r0 << ", " << "["<<r1<<"], #"<< imm<< std::endl;
    }
}

void ldr_print(std::string r0,std::string r1,int imm,std::string r2,int find_type)
{
    total_ldr_times+=1;
    std::string ldr_str="ldr";
    if(r1=="wsp"){
        r1="sp";
    }
    if(r1[0]=='w'){
        r1[0]='x';
    }
    if(find_type==1)// LDR R0, [R1, #imm] 
    {
        if(imm==0)std::cout << std::setw(cout_len) << ldr_str<< r0 << ", " << "["<<r1<< "]"<< std::endl;
       else if(((r0[0]=='x')||(r0[0]=='s'))&&(imm<=16380&&imm>=0&&(imm%4==0)))std::cout << std::setw(cout_len) << ldr_str<< r0 << ", " << "["<<r1<<", #"<< imm<< "]"<< std::endl;
        else if((r0[0]=='x')&&(imm<=32760&&imm>=0&&(imm%8==0)))std::cout << std::setw(cout_len) << ldr_str<< r0 << ", " << "["<<r1<<", #"<< imm<< "]"<< std::endl;
        else
        {
            std::string temp_reg = reg_stack.top();
            reg_stack.pop();
            add_print(temp_reg,sp_reg,imm,"",true,"");
            ldr_print(r0,temp_reg,0,"",3);
            reg_stack.push(temp_reg);    
        }
    }
    if(find_type==2)// LDR R0, [R1, R2] 
    {
        std::cout << std::setw(cout_len) << ldr_str << r0 << ", " << "["<<r1<<", "<< r2<< "]"<< std::endl;
    }
    if(find_type==3)// LDR R0, [R1] 
    {
        std::cout << std::setw(cout_len) << ldr_str << r0 << ", " << "["<<r1<< "]"<< std::endl;
    }
    if(find_type==4){// LDR R0, [R1, #imm]!: imm<=255&&imm>=-256
        std::cout << std::setw(cout_len) << ldr_str<< r0 << ", " << "["<<r1<<", #"<< imm<< "]!"<< std::endl;
    }
    if(find_type==5){// LDR R0, [R1], #imm
        std::cout << std::setw(cout_len) << ldr_str<< r0 << ", " << "["<<r1<<"], #"<< imm<< std::endl;
    }
}

//打印int比较后set 1/0的语句，arg1是目的寄存器r0，arg2是比较寄存器r1，arg3是立即数，arg4是比较寄存器r2，arg5表示比较类型
//可以处理正/负立即数，但范围在0-255，否则扩充语句
void cmp_set_print(std::string r0,std::string r1,uint64_t imm,std::string r2,bool cmpimm,const koopa_raw_binary_op_t op)
{
    bool imm_zero = false;
    if(cmpimm)
    {
        imm_zero = (imm==0);
        if(imm>=0&&imm<=0xfff)
        {
            std::cout << std::setw(cout_len) << "cmp" << r1 << ", #" << imm << std::endl;
        }
       else
       {
            mov_print(r0,imm,"",true,"");
            cmp_set_print(r0,r1,0,r0,false,op);
       }
    }
    else 
    {
        imm_zero=(r2=="wzr"||r2=="xzr");
        std::cout << std::setw(cout_len) << "cmp" << r1 << ", " << r2 << std::endl;
    }
    if(binary_for_br)return;
    switch (op)
    {
    case KOOPA_RBO_GT:// >
        mov_print(r0,1,"",true,"gt");
        mov_print(r0,0,"",true,"le");
        break;
    case KOOPA_RBO_LT:// <
        mov_print(r0,1,"",true,"lt");
        mov_print(r0,0,"",true,"ge");
        break;
    case KOOPA_RBO_LE:// <=
        mov_print(r0,1,"",true,"le");
        mov_print(r0,0,"",true,"gt");
        break;
    case KOOPA_RBO_GE:// >=
        mov_print(r0,1,"",true,"ge");
        mov_print(r0,0,"",true,"lt");
        break;
    case KOOPA_RBO_EQ:// ==
        mov_print(r0,1,"",true,"eq");
        mov_print(r0,0,"",true,"ne");
        break;
    case KOOPA_RBO_NOT_EQ:// !=
        if(imm_zero&&r0==r1)
        {
            mov_print(r0,1,"",true,"ne");
        }
        else
        {
        mov_print(r0,1,"",true,"ne");
        mov_print(r0,0,"",true,"eq");
        }
        break;
    default:
        break;
    }
}

//打印比较float后set 1/0的语句，arg1是目的寄存器r0，arg2是比较寄存器r1，arg3是立即数，arg4是比较寄存器r2，arg5表示比较类型
//寄存器都要是浮点寄存器
//可以处理正/负立即数，处理为mov到浮点寄存器中
void fcmp_set_print(std::string r0,std::string r1,float imm,std::string r2,bool cmpimm,const koopa_raw_binary_op_t op)
{
    if(cmpimm)
    {
        std::string temp_reg = sreg_stack.top();
        sreg_stack.pop();
        fmov_print(temp_reg,imm,"",true,"");
        fcmp_set_print(r0,r1,0,temp_reg,false,op);
        sreg_stack.push(temp_reg);
        return;
    }
    else 
    {
        std::string x1=reg_stack.top();
        std::string x11='w'+x1.substr(1);
        reg_stack.pop();
        std::string x2=reg_stack.top();
        reg_stack.pop();
        std::string x22='w'+x2.substr(1);
        std::cout << std::setw(cout_len) << "fmov " << x11 << ", " << r1 << std::endl;
        std::cout << std::setw(cout_len) << "fmov " << x22 << ", " << r2 << std::endl;
        std::cout << std::setw(cout_len) << "cmp " << x11 << ", " << x22 << std::endl;
        reg_stack.push(x1);
        reg_stack.push(x2);
    }
    if(binary_for_br)return;
    const char *condition = nullptr;
    switch (op)
    {
    case KOOPA_RBO_GT://>
        condition = "gt";
        break;
    case KOOPA_RBO_LT://<
        condition = "mi";
        break;
    case KOOPA_RBO_LE://<=
        condition = "ls";
        break;
    case KOOPA_RBO_GE://>=
        condition = "ge";
        break;
    case KOOPA_RBO_EQ://==
        condition = "eq";
        break;
    case KOOPA_RBO_NOT_EQ://!=
        condition = "ne";
        break;
    default:
        break;
    }
    if (condition != nullptr)
        std::cout << std::setw(cout_len) << "cset " << r0 << ", " << condition << std::endl;
}

//只打印cmp比较语句，arg1是比较寄存器r1，arg2是立即数，arg3是比较寄存器r2
void cmp_print(std::string r1,int imm,std::string r2,bool cmpimm)
{
    if(cmpimm)
    {
        if(imm>=0&&imm<=0xfff)
        {
            std::cout << std::setw(cout_len) << "cmp" << r1 << ", #" << imm << std::endl;
        }
       else
       {
            std::string temp_reg = reg_stack.top();
            reg_stack.pop();
            mov_print(temp_reg,imm,"",true,"");
            cmp_print(r1,0,temp_reg,false);
            reg_stack.push(temp_reg);
       }
    }
    else std::cout << std::setw(cout_len) << "cmp" << r1 << ", " << r2 << std::endl;
}

//载入全局变量地址，arg1是存放地址的目的寄存器r0，arg2是全局变量名
void gloabal_var_load(std::string r0,const char* name)
{
    /*
    ldr rd, =label
    */
    std::cout << std::setw(cout_len) << "ldr" << r0 << ", =" << name << std::endl;
}

//打印浮点数的mov语句，arg1是目的寄存器，arg2是立即数，arg3是源寄存器，没有改参数则置null
//32位数据的传递，考虑了立即数为正数/负数,以及条件码的情况
void fmov_print(std::string dstreg,float imm,std::string srcreg,bool imm2reg,std::string cond)
{
    //如果目的寄存器和源寄存器相同，则不需要mov
    if(dstreg == srcreg)return;
    total_mov_times+=1;
    if(imm2reg)
    {
        float2uint(imm);
        std::string tmp=reg_stack.top();
        reg_stack.pop();
        std::cout << std::setw(cout_len) << "movz "  << tmp << ", #" << fimm_low <<std::endl;
        std::cout << std::setw(cout_len) << "movk "  << tmp << ", #" << fimm_high << ", lsl #16" << std::endl;
        if(dstreg=="s16"){
            tmp="w"+tmp.substr(1);
            std::cout << std::setw(cout_len) << "fmov " << "s16" << ", " << "w19" << std::endl;
            tmp="x"+tmp.substr(1);
            reg_stack.push(tmp);
        }
        else{
            fmov_print(dstreg,0,tmp,false,cond);
            reg_stack.push(tmp);
        }
    }
    else{
        if(dstreg[0]=='x'){
            std::cout << std::setw(cout_len) << "fmov " << "w19" << ", " << srcreg << std::endl;
            if(cond==""){
                std::cout << std::setw(cout_len) << "sxtw " << dstreg << ", " << "w19" << std::endl;
            }
            else{
                std::cout << std::setw(cout_len) << "sxtw " << "x19" << ", " << "w19" << std::endl;
                std::cout << std::setw(cout_len) << "csel " << dstreg << ", " << "x19" << ", " << dstreg << ", " << cond << std::endl;
            }
            std::cout << std::setw(cout_len) << "mov " << "x19" << ", #0"<< std::endl;
        }
        else if(srcreg[0]=='x'){
            srcreg="w"+srcreg.substr(1);
            std::cout << std::setw(cout_len) << "fmov " << "s16" << ", " << srcreg << std::endl;
            if(cond==""){
                std::cout << std::setw(cout_len) << "fmov " << dstreg << ", " << "s16" << std::endl;
            }
            else{
            std::cout << std::setw(cout_len) << "csel " << dstreg << ", " << "s16" << ", " << dstreg << ", " << cond << std::endl;
            }
            fmov_print("s16", 0.0f, "", true, "");
        }
        else{
            if(cond==""){
                std::cout << std::setw(cout_len) << "fmov " << dstreg << ", " << srcreg << std::endl;
            }
            else{
                std::cout << std::setw(cout_len) << "csel " << dstreg << ", " << srcreg << ", " << dstreg << ", " << cond << std::endl;
            }
        }
    }
}

//处理浮点数和整形数的转化，arg1是目的寄存器，arg2是源寄存器，arg3是用来判断转化方向，没有改参数则置null
//另外两个寄存器操作数都必须是浮点寄存器（可以写一个判错的语句）
//32位数据的传递，没考虑条件码的情况
void fcvt_print(std::string r0,std::string r1,bool float2int)
{
    if(float2int)
        std::cout << std::setw(cout_len) << "fcvtzs" <<" "<< r0 << ", " << r1 << std::endl;
     else
        std::cout << std::setw(cout_len) << "scvtf" <<" "<< r0 << ", " << r1 << std::endl;

}
