
#include "../../include/ir/parser.hpp"

std::string cur_func_name;

// Parser类构造方法
Parser::Parser():lexer(std::make_unique<Lexer>("")),cur_token(nullptr){}
Parser::Parser(std::string input){
    lexer = std::make_unique<Lexer>(input);
    cur_token = lexer->next_token();
}


// 词法分析取下一个token
void Parser::next_token(){
    this->cur_token = this->lexer->next_token();
    // std::cout<<cur_token->str<<std::endl;
}

// 匹配以跳过指定token。即若类型与值匹配，则再取一个token，否则返回0
int Parser::expect(TokenKind tk , KindValue value){
    if(this->cur_token->kind==tk && this->cur_token->value==value  ){
        this->next_token();  //  next token
        return 1;
    }else  return 0;// std::cerr<<"expected "<<tk<<", found "<<this->cur_token->kind<<':''<<this->cur_token->str <<std::endl;
    return 0;
}

// 读取当前Symbol类型的token的变量名
std::string Parser::read_symbol(){
    if(this->cur_token->kind==TokenKind::Symbol){
        std::string ret = this->cur_token->str;
        this->next_token();  //  next tokene
        return ret;
    }
    else {
        std::cout<<"expected a symbol, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        return "";
    }
}
BinaryOp Parser:: read_binaryop(){
    if(this->cur_token->kind==TokenKind::BinaryOp){
        BinaryOp op = std::get<BinaryOp>(this->cur_token->value);
        this->next_token();  //  next tokene
        return op;
    }
    else {
        std::cout<<"expected a symbol, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        return BinaryOp::Eq;
    }
}



///*****语法分析 one time*********parse_next********************************************************* */
std::unique_ptr<AstBase>  Parser::parser_next(){
    // std::cout<<this->cur_token->str << std::endl;
    // 若类型为End，则分析到文件末尾，返回End Ast
    if(cur_token->kind == TokenKind::End){
        std::cout<<"parser finish"<<std::endl;
        return std::make_unique<End>(  AstKind::End);
    }else if(cur_token->kind == TokenKind::Keyword){
        switch(std::get<Keyword>(cur_token->value)){
        case Keyword::Global: //识别到 global，parse_global_def分析全局变量定义语法
            return this->parse_global_def();
            break;
        case Keyword::Fun: // 识别到 fun，parse_fun_def分析函数<定义>语法
            return this->parse_fun_def();
            break;
        case Keyword::Decl: // 识别到 decl，parse_fun_decl分析函数<声明>语法
            return this->parse_fun_decl();
            break;
        default:
            std::cout<<"expected global definition/declaration, found"<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
            break;       
        } 
    }else std::cout<<"expected global definition/declaration, found"<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;

    return nullptr; // or throw an exception here
}

///************************************************************************************************** */
// ****parse_next的三大方法**********三种次顶层语法分析：全局变量定义、函数定义及声明********************* */
///************************************************************************************************** */ 
std::unique_ptr<AstBase> Parser:: parse_global_def(){
    // skip 'global'
    this->next_token();
    // get symbol name
    std::string name=this->read_symbol();
    // check & skip '= alloc'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('=') ))==0)return nullptr;
    if(this->expect(TokenKind::Keyword, KindValue(Keyword::Alloc) )==0)return nullptr;
    // get type 
    std::unique_ptr<AstBase> ty=this->parse_type();
    // check & skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char>(',') ))==0)return nullptr;
    // get initializer  
    std::unique_ptr<AstBase> init=this->parse_init();
    // create global memory declaration
    std::unique_ptr<AstBase> value=std::make_unique<GlobalDecl>(AstKind::GlobalDecl,std::move(ty),std::move(init));
    // create global symbol definition
    return std::make_unique<GlobalDef>(AstKind::GlobalDef,std::move(name),std::move(value));
}
std::unique_ptr<AstBase> Parser:: parse_fun_def(){
    // skip 'fun'
    this->next_token();
    // get function name
    std::string name=this->read_symbol();
    cur_func_name=name.substr(1); //设置当前函数名 不包括@
    // get parameters
    std::vector< std::tuple<std::string,std::unique_ptr<AstBase>> > params=this->parse_params_list(
        [this]{return this->parse_type();});
    // get return type
    std::unique_ptr<AstBase> ret=nullptr;
    if(this->cur_token->kind==TokenKind::Other && this->cur_token->value==KindValue(static_cast<char> (':') )){
        this->next_token(); // skip ':'
        ret=this->parse_type();
    }
    // check & skip '{'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('{')))==0)return nullptr;
    //get basic blocks
    std::vector<std::unique_ptr<AstBase>> bbs;
    while(this->cur_token->kind!=TokenKind::Other || this->cur_token->value!=KindValue(static_cast<char> ('}'))){
        //while(token != other'}'  )
        bbs.push_back(this->parse_block()); 
    }
    if(!bbs.empty() ){
        Block * lastBB=static_cast< Block *>(bbs.back().get());
        if(lastBB->stmts.empty()){
            if( ret == nullptr )lastBB->stmts.push_back( std::make_unique<Return>(AstKind::Return,nullptr) );
            else {
                std::unique_ptr<AstBase> ret_value = std::make_unique<IntVal>(AstKind::IntVal,0 ) ;
                lastBB->stmts.push_back( std::make_unique<Return>(AstKind::Return,std::move(ret_value)) ); // void函数的末尾基本块的最后一条指令是return，需要修正
            }
        }else{
            if(lastBB->stmts.back()->kind!=AstKind::Return){
                //void函数的末尾基本块的最后一条指令不是return，需要修正
                if( ret == nullptr )lastBB->stmts.push_back( std::make_unique<Return>(AstKind::Return,nullptr) );
                else {
                    std::unique_ptr<AstBase> ret_value = std::make_unique<IntVal>(AstKind::IntVal,0 ) ;
                    lastBB->stmts.push_back( std::make_unique<Return>(AstKind::Return,std::move(ret_value)) ); // void函数的末尾基本块的最后一条指令是return，需要修正
                }
            }
        }
        
    }
    
    //skip '}'
    this->next_token();  
    // create function definition、
    if (!bbs.empty()){
        if(ret==nullptr)return std::make_unique<FunDef>(AstKind::FunDef,name,std::move(params),nullptr, std::move(bbs));
        else return std::make_unique<FunDef>(AstKind::FunDef,name,std::move(params), std::move(ret), std::move(bbs));
    }else {
        std::cout<<"expected at least one basic block in function definition"<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        return nullptr;
    }

}
std::unique_ptr<AstBase> Parser:: parse_fun_decl(){
    //skip 'fun'
    this->next_token();
    //get function name
    std::string name=this->read_symbol();
    //get parameters
    std::vector<std::unique_ptr<AstBase>> params=this->parse_ty_list([this]{return this->parse_type();});
    // get return type
    if(this->cur_token->kind==TokenKind::Other && this->cur_token->value==KindValue(static_cast<char> (':') )){
        this->next_token(); // skip ':'
        std::unique_ptr<AstBase> ty=this->parse_type();
        return std::make_unique<FunDecl>(AstKind::FunDecl,name, std::move(params), std::move(ty));
    }
    return     std::make_unique<FunDecl>(AstKind::FunDecl,name,std::move( params), nullptr);
}



// 进行一系列的函数func操作，将func()返回值push入vector然后返回
template <typename Func>
std::vector<std::unique_ptr<AstBase>> Parser:: parse_ty_list(Func func){
    // check & eat left bracket
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('(') ))==0)return {};
    // get items
    std::vector<std::unique_ptr<AstBase>> items;
    if(this->cur_token->kind!=TokenKind::Other || this->cur_token->value!=KindValue(static_cast<char> (')') ) ){
        while(1){
            items.push_back(func());// get item
            if(this->cur_token->kind!=TokenKind::Other || this->cur_token->value!=KindValue(static_cast<char> (',') ))break;
            this->next_token(); // skip ','
        }
    }    
    // check & skip ')'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (')') ))==0)return {};    
    return items;
}
// 进行一系列的函数func操作和取变量名，将 tuple(func()返回值和变量名) push入vector然后返回
template <typename Func>
std::vector< std::tuple<std::string,std::unique_ptr<AstBase>> > Parser:: parse_params_list(Func func){
    // check & eat left bracket
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('(') ))==0)return {};
    // get items
    std::vector< std::tuple<std::string,std::unique_ptr<AstBase>> > items;
    if(this->cur_token->kind!=TokenKind::Other || this->cur_token->value!=KindValue(static_cast<char> (')') ) ){
        while(1){
            std::string name=this->read_symbol();
            name = name[0]+cur_func_name +'_'+name.substr(1 );
            // check & skip ':'
            this->expect(TokenKind::Other, KindValue(static_cast<char> (':') ));
            items.push_back(std::make_tuple(name,func()));// get item
            if(this->cur_token->kind!=TokenKind::Other || this->cur_token->value!=KindValue(static_cast<char> (',') ))break;
            this->next_token(); // skip ','
        }
    }    
    // check & skip ')'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (')') ))==0)return {};    
    return items;
}

///************************************************************************************************************************* 
//******分析数据类型type的方法parse_type*****及其调用的五大方法（具体分析）***************************************************** 
/// Parses types.*********************************************************************************************************
//*************************************************************************************************************************
std::unique_ptr<AstBase> Parser:: parse_type(){
    if(this->cur_token->kind==TokenKind::Keyword){
        if(this->cur_token->value==KindValue(Keyword::I32))      return this->parse_int_type();
        else if(this->cur_token->value==KindValue(Keyword::F32)) return this->parse_float_type();
    }else if(this->cur_token->kind==TokenKind::Other){
        if(this->cur_token->value==KindValue(static_cast<char> ('[') ) )       return this->parse_array_type();
        else if(this->cur_token->value==KindValue(static_cast<char> ('*') ) )  return this->parse_pointer_type();
        else if(this->cur_token->value==KindValue(static_cast<char> ('(') ))  return this->parse_fun_type();
    }
    std::cout<< "expected type, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
    return nullptr;
}
std::unique_ptr<AstBase> Parser:: parse_int_type(){
    this->next_token();
    return std::make_unique<IntType>(AstKind::IntType);
}
std::unique_ptr<AstBase> Parser:: parse_float_type(){
    this->next_token();
    return std::make_unique<FloatType>(AstKind::FloatType);
}
std::unique_ptr<AstBase> Parser:: parse_array_type(){
    // skip '['
    this->next_token();
    // get base type
    std::unique_ptr<AstBase> base=this->parse_type();
    // check & skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',') ))==0)return nullptr;
    // get length
    if(this->cur_token->kind!=TokenKind::Int){
        std::cout<<"expected int, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        return nullptr;
    }
    int len=std::get<int>(this->cur_token->value) ;
    this->next_token();
    // check & skip ']'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (']') ))==0)return nullptr;
    return std::make_unique<ArrayType>(AstKind::ArrayType, std::move(base),len);
}
std::unique_ptr<AstBase> Parser:: parse_pointer_type(){
    //skip '*'
    this->next_token();
    // get base type
    std::unique_ptr<AstBase> base=this->parse_type();
    return std::make_unique<PointerType>(AstKind::PointerType, std::move( base));
}
std::unique_ptr<AstBase> Parser:: parse_fun_type(){
    // get parameter type list
    std::vector<std::unique_ptr<AstBase>> params=this->parse_ty_list([this]{return this->parse_type();});
    //get return type
    if(this->cur_token->kind==TokenKind::Other && this->cur_token->value==KindValue(static_cast<char> (':') )){
        this->next_token(); // skip ':'
        std::unique_ptr<AstBase> ty=this->parse_type();
        return std::make_unique<FunType>(AstKind::FunType, std::move(params), std::move(ty));
    }
    return std::make_unique<FunType>(AstKind::FunType,std::move(params), nullptr);
}

// Parses values.
std::unique_ptr<AstBase> Parser:: parse_value(){
    std::unique_ptr<AstBase> ret=nullptr;
    switch (this->cur_token->kind){
    case TokenKind::Symbol: // symbol reference
        ret= std::make_unique<SymbolRef>(AstKind::SymbolRef, this->cur_token->str);
        break;
    case TokenKind::Int:    // integer literal
        ret= std::make_unique<IntVal>(AstKind::IntVal, std::get<int>(this->cur_token->value) );
        break;
    case TokenKind::Float: // float literal
        ret= std::make_unique<FloatVal>(AstKind::FloatVal, std::get<float>(this->cur_token->value) );
        break;
    case TokenKind::Keyword: //undefined value
        if(this->cur_token->value==KindValue(Keyword::Undef))   // undefined
            ret= std::make_unique<UndefVal>(AstKind::UndefVal);
        else std::cout<<"expected undefined, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        break;
    default:
        std::cout<<"expected value, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        break;
    }
    this->next_token();
    return ret;

}


///*******init初始化器的分析***********************************************************************************
// Parses initializers.
std::unique_ptr<AstBase> Parser:: parse_init(){
    std::unique_ptr<AstBase> ast;
    switch (this->cur_token->kind){
    case TokenKind::Symbol: // symbol reference
        return std::make_unique<SymbolRef>(AstKind::SymbolRef, this->read_symbol());
        break;
    case TokenKind::Int:    // integer literal
        ast = std::make_unique<IntVal>(AstKind::IntVal, std::get<int>(this->cur_token->value) );
        this->next_token();
        return ast;     break;
    case TokenKind::Float: // float literal
        ast = std::make_unique<FloatVal>(AstKind::FloatVal, std::get<float>(this->cur_token->value) );
        this->next_token();
        return ast;     break;
    case TokenKind::Keyword: 
        if(this->cur_token->value==KindValue(Keyword::Undef)) {  // undefined value
            ast = std::make_unique<UndefVal>(AstKind::UndefVal);
            this->next_token();
            return ast;     
        }else if (this->cur_token->value==KindValue( Keyword::ZeroInit)) {  // zero initializer
            ast = std::make_unique<ZeroInit>(AstKind::ZeroInit);
            this->next_token();
            return ast; 
        }else std::cout<<"expected undefined or zero initializer, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        break;
    case TokenKind::Other:  // aggregate
        if(this->cur_token->value==KindValue(static_cast<char> ('{') ))  return this->parse_aggregate();
        else std::cout<<"expected '{' , found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        break;
    default:
        std::cout<<"expected initializer, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        break;
    }
    return nullptr;
}
std::unique_ptr<AstBase> Parser:: parse_aggregate(){
    //skip '{'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('{') ) )==0)return nullptr;
    //get elements
    std::vector<std::unique_ptr<AstBase>> elems;
    elems.push_back(this->parse_init());
    while (this->cur_token->kind==TokenKind::Other && this->cur_token->value==KindValue(static_cast<char> (',') )){
        this->next_token();
        elems.push_back(this->parse_init());
    }
    // check & skip '}'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('}') ))==0)
        std::cout<<"expected '}', found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
    return std::make_unique<Aggregate>(AstKind::Aggregate,std::move( elems));
}


///*****************************************************************************************************************
// ****************parse_fun_def 的下属方法分析（更具体）***********************************************************
///分析基本块 Basic Block **************************************************************************************************** 
std::unique_ptr<AstBase> Parser:: parse_block(){
    // get block name
    std::string name=this->read_symbol();
    // get parameters
    std::vector< std::tuple<std::string,std::unique_ptr<AstBase>> > params=this->parse_params_list(
        [this]{return this->parse_type();});
    // check & skip ':'
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (':') ) )==0)return nullptr;
    // get statements
    std::vector<std::unique_ptr<AstBase>>  stmts;
    bool exit_flag=false;
    while (!exit_flag){
        if(this->cur_token->kind==TokenKind::Symbol)stmts.push_back(this->parse_symbol_def());
        else if(this->cur_token->kind==TokenKind::Keyword){
            if(this->cur_token->value==KindValue(Keyword::Store))stmts.push_back(this->parse_store());
            else if(this->cur_token->value==KindValue(Keyword::Call))stmts.push_back(this->parse_fun_call());
            else if(this->cur_token->value==KindValue(Keyword::Br)){
                exit_flag=true;
                stmts.push_back(this->parse_branch());
            }else if(this->cur_token->value==KindValue(Keyword::Jump)){
                exit_flag=true;
                stmts.push_back(this->parse_jump());
            }else if(this->cur_token->value==KindValue(Keyword::Ret)){
                exit_flag=true; 
                stmts.push_back(this->parse_return());
                // if(this->cur_token->kind==TokenKind::Keyword && 
                //     this->cur_token->value==KindValue(Keyword::Ret)){//如果下一个还是ret

                // }
            }else std::cout<<"expected Keyword of store, call, br, jump, or ret, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
        }else if( (this->cur_token->kind==TokenKind::Other && this->cur_token->value==KindValue(static_cast<char>('}')))
                || this->cur_token->kind==TokenKind::End )  exit_flag=true;
        else std::cout<<"expected statement, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
    }
    return std::make_unique<Block>(AstKind::Block, name, std::move(params), std::move(stmts));
}

// Parses stores.
std::unique_ptr<AstBase> Parser:: parse_store(){
    // skip 'Store'
    this->next_token();
    // get value
    std::unique_ptr<AstBase> value;
    if(this->cur_token->kind!=TokenKind::Symbol) value=this->parse_init();
    else { //用符号当值
        std::string symbol = this->read_symbol();
        if(symbol.substr(0,1)=="@" && !(symbol.length()>=12 && symbol.substr(0,11)=="@__filed0__" ) ){ //不是全局变量 !(symbol.length()>=12 && symbol.substr(0,11)=="@__filed0__" )
            symbol = symbol[0]+cur_func_name +'_'+symbol.substr(1);
        }     
        value=std::make_unique<SymbolRef>(AstKind::SymbolRef,symbol );
        // this->next_token(); // skip symbol
    }
    // check & skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',')))==0) return nullptr;
    // get symbol name
    std::string name=this->read_symbol();
    if(name.substr(0,4)!="%ptr" && !(name.length()>=12 && name.substr(0,11)=="@__filed0__" ) ){
        name = name[0]+cur_func_name +'_'+name.substr(1);
    }
    return std::make_unique<Store>(AstKind::Store, std::move(value), name);
}

// Parses function calls.
std::unique_ptr<AstBase> Parser:: parse_fun_call(){
    //skip 'call'
    this->next_token();
    // get function name
    std::string fun=this->read_symbol();
    // get arguments
    std::vector<std::unique_ptr<AstBase>> args=this->parse_ty_list([this]{return this->parse_value();});
    // create function call
    return std::make_unique<FunCall>(AstKind::FunCall, fun, std::move(args));
}

// Parses branches.
std::unique_ptr<AstBase> Parser:: parse_branch(){
    //skip 'branch'
    this->next_token();
    //get condition
    std::unique_ptr<AstBase> cond=this->parse_value();
    // check & skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',')))==0) return nullptr;
    // get true target basic block
    std::string tbb_name=this->read_symbol();
    // get true target basic block arguments
    std::vector<std::unique_ptr<AstBase>> targs=this->parse_ty_list([this]{return this->parse_value();});
    // check & skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',')))==0) return nullptr;
    // get false target basic block
    std::string fbb_name=this->read_symbol();
    // get false target basic block arguments
    std::vector<std::unique_ptr<AstBase>> fargs=this->parse_ty_list([this]{return this->parse_value();});
    // create branch
    return std::make_unique<Branch>(AstKind::Branch, std::move(cond), tbb_name, std::move(targs), fbb_name, std::move(fargs));
}

// Parses jumps.
std::unique_ptr<AstBase> Parser:: parse_jump(){
    // skip 'jump'
    this->next_token();
    // get target basic block
    std::string target=this->read_symbol();
    // get target basic block arguments
    std::vector<std::unique_ptr<AstBase>> args=this->parse_ty_list([this]{return this->parse_value();});
    // create jump
    return std::make_unique<Jump>(AstKind::Jump, target, std::move(args));
}

// Parses returns.
std::unique_ptr<AstBase> Parser:: parse_return(){
    // skip 'ret'
    this->next_token();
    // get value
    std::unique_ptr<AstBase> value=nullptr;
    if(this->lexer->ifline_feed!=1 &&     
        (this->cur_token->kind!=TokenKind::Other ||this->cur_token->value!=KindValue(static_cast<char> ('}')))){ //没换行，确定有返回值
        value=this->parse_value();
    }
    return std::make_unique<Return>(AstKind::Return, std::move(value));    
}


std::unique_ptr<AstBase> Parser:: parse_symbol_def(){
    // get symbol name
    std::string name=this->read_symbol();
    
    // check & skip '='
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> ('=') ))==0) return nullptr;
    // get value
    std::unique_ptr<AstBase> value=nullptr;
    if(this->cur_token->kind==TokenKind::Keyword) {
        if(this->cur_token->value==KindValue(Keyword::Alloc)) {
            name = name[0]+cur_func_name +'_'+name.substr(1 );
            value=this->parse_mem_decl();
        }else if(this->cur_token->value==KindValue(Keyword::Load)) {
            value=this->parse_load();
        }else if(this->cur_token->value==KindValue(Keyword::GetPtr)) {
            value=this->parse_get_pointer();
        }else if(this->cur_token->value==KindValue(Keyword::GetElemPtr)) {
            value=this->parse_get_element_pointer();
        }else if(this->cur_token->value==KindValue(Keyword::Call)) {
            value=this->parse_fun_call();
        }else std::cout<<"expected expression, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
    }else if(this->cur_token->kind==TokenKind::BinaryOp){
        value=this->parse_binary_expr();
    }else std::cout<<"expected expression, found "<<this->cur_token->kind<<":\'"<<this->cur_token->str<<'\''<<std::endl;
    return std::make_unique<SymbolDef>(AstKind::SymbolDef, name, std::move(value));
}

// Parses memory declarations
std::unique_ptr<AstBase> Parser:: parse_mem_decl(){
    //skip 'Alloc'
    this->next_token();
    //get type
    std::unique_ptr<AstBase> ty=this->parse_type();
    return std::make_unique<MemDecl>(AstKind::MemDecl, std::move(ty));
}

// Parses loads.
std::unique_ptr<AstBase> Parser:: parse_load(){
    // skip 'Load'
    this->next_token();
    // get symbol name
    std::string symbol=this->read_symbol();
    if(symbol.substr(0,4)!="%ptr" && !(symbol.length()>=12 && symbol.substr(0,11)=="@__filed0__" ) ) {
        symbol = symbol[0]+cur_func_name +'_'+symbol.substr(1 );
    }
    return std::make_unique<Load>(AstKind::Load, symbol);
}

// Parses pointer calculations.
std::unique_ptr<AstBase> Parser:: parse_get_pointer(){
    // skip 'GetPtr'
    this->next_token();
    //get symbol name
    std::string symbol=this->read_symbol();
    if(symbol.substr(0,4)!="%ptr" && !(symbol.length()>=12 && symbol.substr(0,11)=="@__filed0__" ) ) {
        symbol = symbol[0]+cur_func_name +'_'+symbol.substr(1);
    }
    // check and skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',')))==0) return nullptr;
    // get value
    std::unique_ptr<AstBase> value=this->parse_value();
    // create get pointer
    return std::make_unique<GetPointer>(AstKind::GetPointer, symbol, std::move(value));
}

// Parses element pointer calculations.
std::unique_ptr<AstBase> Parser:: parse_get_element_pointer(){
    // skip 'GetElemPtr'
    this->next_token();
    // get symbol name
    std::string symbol=this->read_symbol();
    if(symbol.substr(0,4)!="%ptr" && !(symbol.length()>=12 && symbol.substr(0,11)=="@__filed0__" ) ) {
        symbol = symbol[0]+cur_func_name +'_'+symbol.substr(1);
    }
    // check and skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',')))==0) return nullptr;
    // get value
    std::unique_ptr<AstBase> value=this->parse_value();
    // create get element pointer
    return std::make_unique<GetElementPointer>(AstKind::GetElementPointer, symbol, std::move(value));
}
std::unique_ptr<AstBase> Parser:: parse_binary_expr(){
    // get operator
    BinaryOp op= this->read_binaryop();
    // get lhs 
    std::unique_ptr<AstBase> lhs=this->parse_value();
    // check & skip ','
    if(this->expect(TokenKind::Other, KindValue(static_cast<char> (',')))==0) return nullptr;
    // get rhs
    std::unique_ptr<AstBase> rhs=this->parse_value();
    // create binary expression
    return std::make_unique<BinaryExpr>(AstKind::BinaryExpr, op, std::move(lhs), std::move(rhs));
}




