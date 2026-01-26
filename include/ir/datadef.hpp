#pragma once
#ifndef DATADEF_HPP
#define DATADEF_HPP

#include <variant>
#include <string>
#include <cctype>
#include <iostream>
#include <memory>
#include <unordered_map>
#include <fstream>
 
#include <cstdint>
#include <cstring>
#include <bitset>

#include "koopa.h"
 
enum class BinaryOp {
  /// Not equal to.
  NotEq,
  /// Equal to.
  Eq,
  /// Greater than.
  Gt,
  /// Less than.
  Lt,
  /// Greater than or equal to.
  Ge,
  /// Less than or equal to.
  Le,
  /// Addition.
  Add,
  /// Subtraction.
  Sub,
  /// Multiplication.
  Mul,
  /// Division.
  Div,
  /// Modulo.
  Mod,
  /// Bitwise AND.
  And,
  /// Bitwise OR.
  Or,
  /// Bitwise XOR.
  Xor,
  /// Shift left logical.
  Shl,
  /// Shift right logical.
  Shr,
  /// Shift right arithmetic.
  Sar
};
//字符与BinaryOp的映射
static const std::unordered_map<std::string, BinaryOp> BINARY_OPS = {
    {"ne" , BinaryOp::NotEq},
    {"eq" , BinaryOp::Eq},
    {"gt" , BinaryOp::Gt},
    {"lt" , BinaryOp::Lt},
    {"ge" , BinaryOp::Ge},
    {"le" , BinaryOp::Le},
    {"add" , BinaryOp::Add},
    {"sub" , BinaryOp::Sub},
    {"mul" , BinaryOp::Mul},
    {"div" , BinaryOp::Div},
    {"mod" , BinaryOp::Mod},
    {"and" , BinaryOp::And},
    {"or" , BinaryOp::Or},
    {"xor" , BinaryOp::Xor},
    {"shl" , BinaryOp::Shl},
    {"shr" , BinaryOp::Shr},
    {"sar" , BinaryOp::Sar}
};
//BinaryOP与koopa_raw_binary_op的映射
static const std::unordered_map<BinaryOp, koopa_raw_binary_op> OPTORAWOP = {
    { BinaryOp::NotEq , KOOPA_RBO_NOT_EQ },
    { BinaryOp::Eq    , KOOPA_RBO_EQ     },
    { BinaryOp::Gt    , KOOPA_RBO_GT     },
    { BinaryOp::Lt    , KOOPA_RBO_LT     },
    { BinaryOp::Ge    , KOOPA_RBO_GE     },
    { BinaryOp::Le    , KOOPA_RBO_LE     },
    { BinaryOp::Add   , KOOPA_RBO_ADD     },
    { BinaryOp::Sub   , KOOPA_RBO_SUB     },
    { BinaryOp::Mul   , KOOPA_RBO_MUL     },
    { BinaryOp::Div   , KOOPA_RBO_DIV     },
    { BinaryOp::Mod   , KOOPA_RBO_MOD     },
    { BinaryOp::And   , KOOPA_RBO_AND     },
    { BinaryOp::Or    , KOOPA_RBO_OR     },
    { BinaryOp::Xor   , KOOPA_RBO_XOR     },
    { BinaryOp::Shl   , KOOPA_RBO_SHL     },
    { BinaryOp::Shr   , KOOPA_RBO_SHR     },
    { BinaryOp::Sar   , KOOPA_RBO_SAR     }
};


//***********************Keyword关键字******************************************* */
//对应rust的token.rs中定义
enum class Keyword{
  /// Keyword `i32`.
  I32,
  /// Keyword `f32`.
  F32,
  /// Keyword `undef`.
  Undef,
  /// Keyword `zeroinit`.
  ZeroInit,
  /// Keyword `global`.
  Global,
  /// Keyword `alloc`.
  Alloc,
  /// Keyword `load`.
  Load,
  /// Keyword `store`.
  Store,
  /// Keyword `getptr`.
  GetPtr,
  /// Keyword `getelemptr`.
  GetElemPtr,
  /// Keyword `br`.
  Br,
  /// Keyword `jump`.
  Jump,
  /// Keyword `call`.
  Call,
  /// Keyword `ret`.
  Ret,
  /// Keyword `fun`.
  Fun,
  /// Keyword `decl`.
  Decl
};
static const std::unordered_map<std::string, Keyword> KEYWORDS = {
        {"i32", Keyword::I32},
        {"f32", Keyword::F32},
        {"undef", Keyword::Undef},
        {"zeroinit", Keyword::ZeroInit},
        {"global", Keyword::Global},
        {"alloc" , Keyword::Alloc},
        {"load" , Keyword::Load},
        {"store" , Keyword::Store},
        {"getptr" , Keyword::GetPtr},
        {"getelemptr" , Keyword::GetElemPtr},
        {"br" , Keyword::Br},
        {"jump" , Keyword::Jump},
        {"call" , Keyword::Call},
        {"ret" , Keyword::Ret},
        {"fun" , Keyword::Fun},
        {"decl" , Keyword::Decl}
};

//***********************Token******************************************* */
using KindValue = std::variant<
    int ,   // 对应Rust的Int(i32)
    float,  // 对应Rust的Float(f32)
    std::string ,    // 对应Rust的Symbol(String)
    Keyword,        // 对应Rust的Keyword(Keyword)
    BinaryOp,       // 对应Rust的BinaryOp(BinaryOp)
    char,           // 对应Rust的Other(char)
    std::monostate  // 对应Rust的End
>;
//Overloading the '==' operator
bool operator==(const KindValue& lhs, const KindValue& rhs);
bool operator!=(const KindValue& lhs, const KindValue& rhs);
 

enum class TokenKind {
    Int,    /// Integer literal.
    Float,  /// Float literal.
    Symbol, /// Symbol (identifier like `@id` or `%id`).
    Keyword, /// Keyword.
    BinaryOp, /// Binary operator.
    Other,   /// Other characters.
    End        /// End of file.
};
std::ostream& operator<<(std::ostream& os, TokenKind kind);

//对应rust的token.rs中定义
class Token{
 public:
    TokenKind kind;
    KindValue value;
    std::string  str;
    Token();
    Token(TokenKind k ,KindValue v, std::string  s);
};


//***********************Lexer******************************************* */
class Lexer{
 public:
    std::string read;
    int  pos;
    int  ifline_feed;
    char last_char;
    Token * now_token;

    Lexer();
    Lexer(std::string input);
    void next_char(); //读取一个字符，设置last_char

    void handle_comment();
    void handle_symbol();
    void handle_keyword();
    // void handle_number();

    void handle_integer();
    void handle_float();


    std::unique_ptr<Token> next_token(); 
};




#endif // DATADEF_HPP
