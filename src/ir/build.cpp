#include "../../include/ir/build.hpp"
#include "../../include/ir/koopaAST.hpp"

Builder::Builder() : parser(std::make_unique<Parser>()) {}
Builder::Builder(std::string input) : parser(std::make_unique<Parser>(input)) {}


koopa_raw_program_t * Builder::build() {

    koopa_raw_program_t * raw=new koopa_raw_program_t();
    //将raw的values和funcs两个slice切片初始化好
    raw->values.kind = KOOPA_RSIK_VALUE;
    raw->funcs.kind = KOOPA_RSIK_FUNCTION;
    //使用两个vector存储用parser分析得到的ast来buildon后的内容
    std::vector<koopa_raw_value_data_t *> values_buf;
    std::vector<koopa_raw_function_data_t * > funcs_buf;
    //启动parser进行语法分析，返回ast，根据返回ast种类分别存储到vector中
    while (this->parser->cur_token->kind!=TokenKind::End){
        std::unique_ptr<AstBase> ast=this->parser->parser_next();
        // std::cout<<ast->kind<<std::endl;
        switch (ast->kind){
        case AstKind::GlobalDef : {//全局变量定义
            // std::cout<<"global var def"<<std::endl;
            GlobalDef *globaldef=static_cast <  GlobalDef * >(ast.get());
            if(globaldef)values_buf.push_back(globaldef->build_on());
            else std::cout<<"cast error"<<std::endl;
            break;
        }
        case AstKind::FunDecl :{ // 函数声明
            // std::cout<<"func decl"<<std::endl;
            FunDecl *fundecl=static_cast <  FunDecl * >(ast.get());
            if(fundecl)funcs_buf.push_back(fundecl->build_on());
            else std::cout<<"cast error"<<std::endl;
            break;
        }
        case AstKind::FunDef :{//函数定义
           
            FunDef *fundef=static_cast <  FunDef * >(ast.get());
            if(fundef)funcs_buf.push_back(fundef->build_on());
            else std::cout<<"cast error"<<std::endl;
            break;
        }
        default:{
            std::cout<<"ast kind error!"<<std::endl;
            break;
        }}
    }

    ///bb : 通过bbtable 的used_by填充koopa.h数据结构的used
    for (auto& pair : bbtable) {
        for(auto & item : pair.second){  //second还是一个map     是 std::vector<BasicBlock *>
            // item.first是函数名  , second是 std::unique_ptr<SymInfo> syminfo
            //kind
            item.second->bb_addr->used_by.kind = KOOPA_RSIK_VALUE; 
            //len 
            int len = item.second->used_by.size();
            item.second->bb_addr->used_by.len =len;
            //buffer
            if(len>0){
                item.second->bb_addr->used_by.buffer = new const void*[len];
                for(size_t i =0 ; i<len; ++i){
                    item.second->bb_addr->used_by.buffer[i] = static_cast<const void*>( (item.second->used_by)[i] );
                }
            }else item.second->bb_addr->used_by.buffer =nullptr;
        }    
    }
    
    /// value : 通过symtable 的used_by填充koopa.h数据结构的used
    for (auto& pair : symtable) {
        for(auto & item : pair.second){  //second还是一个map
            // item.first是函数名   , second是 std::unique_ptr<SymInfo> syminfo
            //kind
            item.second->sym_addr->used_by.kind = KOOPA_RSIK_VALUE; 
            //len 
            int len = item.second->used_by.size();
            item.second->sym_addr->used_by.len =len;
            //buffer
            if(len>0){
                item.second->sym_addr->used_by.buffer = new const void*[len];
                for(size_t i =0 ; i<len; ++i){
                    item.second->sym_addr->used_by.buffer[i] = static_cast<const void*>( (item.second->used_by)[i] );
                }
            }else  item.second->sym_addr->used_by.buffer =nullptr;
        }
    }


    // 分配给buffer 空间，并将vector的转移到buffer
    raw->values.buffer = new const void*[values_buf.size()];
    raw->funcs.buffer  = new const void*[funcs_buf.size()];
    raw->values.len=values_buf.size();
    raw->funcs.len=funcs_buf.size();
    // 将vector中的指针复制到动态分配的数组中
    for (size_t i = 0; i < values_buf.size(); ++i) 
        raw->values.buffer[i] = static_cast<const void*>(values_buf[i]);
    for (size_t i = 0; i < funcs_buf.size(); ++i) 
        raw->funcs.buffer[i] = static_cast<const void*>(funcs_buf[i]);

    return raw;
}

 