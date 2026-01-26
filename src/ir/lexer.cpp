
#include "../../include/ir/datadef.hpp"

bool operator==(const KindValue& lhs, const KindValue& rhs){
    if(std::holds_alternative<int>(lhs) && std::holds_alternative<int>(rhs)){
      return std::get<int>(lhs) == std::get<int>(rhs);
    }
    if(std::holds_alternative<float>(lhs) && std::holds_alternative<float>(rhs)){
      return std::get<float>(lhs) == std::get<float>(rhs);
    }
    if(std::holds_alternative<std::string>(lhs) && std::holds_alternative<std::string>(rhs)){
      return std::get<std::string>(lhs) == std::get<std::string>(rhs);
    }
    if(std::holds_alternative<Keyword>(lhs) && std::holds_alternative<Keyword>(rhs)){
      return std::get<Keyword>(lhs) == std::get<Keyword>(rhs);
    }
    if(std::holds_alternative<BinaryOp>(lhs) && std::holds_alternative<BinaryOp>(rhs)){
      return std::get<BinaryOp>(lhs) == std::get<BinaryOp>(rhs);
    }
    if(std::holds_alternative<char>(lhs) && std::holds_alternative<char>(rhs)){
      return std::get<char>(lhs) == std::get<char>(rhs);
    }
    if(std::holds_alternative<std::monostate>(lhs) && std::holds_alternative<std::monostate>(rhs)){
      return true;
    }
    return false;
}
bool operator!=(const KindValue& lhs, const KindValue& rhs){
    if(lhs==rhs)return false;
    return true;
}
 

std::ostream& operator<<(std::ostream& os, TokenKind kind) {
    switch (kind) {
        case TokenKind::Int:
            os << "Int";
            break;
        case TokenKind::Float:
            os << "Float";
                break;
        case TokenKind::Symbol:
            os << "Symbol";
            break;
        case TokenKind::Keyword:
            os << "Keyword";
            break;
        case TokenKind::BinaryOp:
            os << "BinaryOp";
            break;
        case TokenKind::Other:
            os << "Other";
            break;
        case TokenKind::End:
            os << "End";
            break;
        default:
            os << "Unknown";
            break;
    }
    return os;
}

Token::Token():kind(TokenKind::Int),value(0),str(" "){}
Token::Token(TokenKind k ,KindValue v, std::string  s):
    kind(k),value(v),str(s){}


Lexer::Lexer():read(""),pos(0),ifline_feed(0),last_char(' '),now_token(){}
Lexer::Lexer(std::string input){
    read=input;
    pos=0;
    ifline_feed=0;
    last_char=' ';
    now_token=new Token();
    this->next_char();
}

void Lexer::next_char(){
    // while (std::isspace(this->read[this->pos])){
    //     this->pos+=1;
    // }
    this->last_char=(this->pos < this->read.length() ) ?  this->read[this->pos] : EOF;
    this->pos+=1;
    
}


std::unique_ptr<Token> Lexer::next_token(){
    this->ifline_feed=0;
    if(this->now_token!=nullptr){
        delete this->now_token;
        this->now_token=nullptr;
    }
    while (std::isspace(this->last_char)){
        if(this->last_char=='\n'){
            this->ifline_feed=1;
        }
        this->next_char();
        
    }
        
    char c =this->last_char;
    if(c=='/'){
        this->handle_comment();
    }else if(c=='@' || c=='%'){ //symbols
        this->handle_symbol();
    }else if(isalpha(c)){ //关键字或操作数
        this->handle_keyword();
    }else if( isdigit(c) || c=='-'){ //数字
        this->handle_integer();
    }else if(c=='#'){ //float
        this->handle_float();
    }else if(c==EOF){ //文件末尾
        this->now_token=new Token(TokenKind::End,std::monostate{},"");
    }else{ //others
        this->next_char();
        // std::cout<<"other"<<c<<std::endl;
        this->now_token=new Token(TokenKind::Other,c,std::string(1, c));
    }
    // std::cout<<this->now_token->str<<std::endl;
    return std::make_unique<Token> (*(this->now_token));
}


/// Handles comments.
void Lexer::handle_comment(){
    // skip '/'
    this->next_char();
    // check if is block comment
    if(this->last_char=='*') {
        //self.handle_block_comment(span)
        // skip '*'
        this->next_char();
        // read until there is '*/' in stream
        bool end=false;
        while ( (this->last_char!=EOF)&&!(end&&this->last_char=='/') ){
            end= (this->last_char=='*');
            this->next_char();
        }
        // check unclosed block comment
        if(this->last_char==EOF){//到达文件末尾，还没匹配到闭合
            std::cout<<"comment unclosed at EOF!"<<std::endl;
        }else{
            this->next_char();
            this->next_token();
        }

    } else if (this->last_char=='/') {
        // skip the current line
        while((this->last_char!='\r') && (this->last_char!='\n'))
            this->next_char();
        // return the next token
        this->next_token();      
    } else {
        std::cout<<"Error"<<std::endl;
    }
}

/// Handles symbols.
void Lexer::handle_symbol(){
    std::string sym;
    sym+=this->last_char;
    this->next_char();
    if(isdigit(this->last_char)){
        if(sym[0]=='@')std::cout<<"Error"<<std::endl;
        sym=sym+this->last_char;
        this->next_char();
        if(sym[1]!='0'){
            while(isdigit(this->last_char)){
                sym=sym+this->last_char;
                this->next_char();
            }
        }
    }else{
        while((isalnum(this->last_char) || this->last_char=='_')){
            sym=sym+this->last_char;
            this->next_char();
        }
    }
    // std::cout<<sym<<std::endl;
    if(sym.length()==1)std::cout<<"Error"<<std::endl;
    else this->now_token=new Token(TokenKind::Symbol,sym,sym);

}

/// Handles keywords or operands.
void Lexer::handle_keyword(){
    std::string keyword;
    while( isalnum(this->last_char)  ){
        keyword=keyword+this->last_char;
        this->next_char();
    }
    // std::cout<<keyword<<std::endl;
    auto it = KEYWORDS.find(keyword);
    if (it != KEYWORDS.end()) {
        this->now_token=new Token(TokenKind::Keyword,it->second,keyword);
    }else {
        auto it = BINARY_OPS.find(keyword);
        if (it != BINARY_OPS.end()) {
            this->now_token=new Token(TokenKind::BinaryOp,it->second,keyword);
        }else   std::cout<<"Error"<<std::endl;
    }
}

// Handles integer literals.
void Lexer::handle_integer(){
    std::string num;
    //read to string
    num+=this->last_char;
    this->next_char();
    while (isdigit(this->last_char)){
        num+= this->last_char;
        this->next_char();
    }
    int i=std::stoi(num);
    this->now_token=new Token(TokenKind::Int,i,num);
}

// Handles float literals.
void Lexer::handle_float(){
    std::string num;
    //read to string
    this->next_char();
    while (this->last_char=='0' || this->last_char=='1'){
        num+= this->last_char;
        this->next_char();
    }
    // 确保输入字符串是32位长
    if (num.length() != 32)  std::cout<<"The binary float string must be 32 bits long."<<std::endl;
    // 使用std::bitset将二进制字符串转换为uint32_t
    std::bitset<32> floatbit(num);  
    uint32_t floatuint = floatbit.to_ulong();    
    // 使用类型重解释将uint32_t转换为float
    float f = *reinterpret_cast<float*>(&floatuint);
   
    this->now_token=new Token(TokenKind::Float,f,std::to_string(f) );

}







 
 
