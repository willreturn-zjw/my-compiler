#pragma once
#ifndef PARSER_HPP
#define PARSER_HPP

#include "datadef.hpp"
#include "koopaAST.hpp"
#include <tuple>
//*********************** Parser ******************************************* */
class Parser{
 public:
    std::unique_ptr<Lexer> lexer;
    std::unique_ptr<Token> cur_token;
    Parser();
    Parser(std::string input);

    std::unique_ptr<AstBase> parser_next();

    void next_token();
    int expect(TokenKind tk, KindValue value);
    std::string read_symbol();
    BinaryOp read_binaryop();


    template <typename Func>
    std::vector<std::unique_ptr<AstBase>> parse_ty_list(Func func);
    template <typename Func>
    std::vector< std::tuple<std::string,std::unique_ptr<AstBase>> > parse_params_list(Func func);

    std::unique_ptr<AstBase> parse_type();
    std::unique_ptr<AstBase> parse_int_type();
    std::unique_ptr<AstBase> parse_float_type();
    std::unique_ptr<AstBase> parse_array_type();
    std::unique_ptr<AstBase> parse_pointer_type();
    std::unique_ptr<AstBase> parse_fun_type();

    std::unique_ptr<AstBase> parse_value();


    std::unique_ptr<AstBase> parse_init();
    std::unique_ptr<AstBase> parse_aggregate();


    std::unique_ptr<AstBase> parse_global_def();
    std::unique_ptr<AstBase> parse_fun_def();
    std::unique_ptr<AstBase> parse_fun_decl();


    std::unique_ptr<AstBase> parse_block();
    std::unique_ptr<AstBase> parse_symbol_def();
    std::unique_ptr<AstBase> parse_store();
    std::unique_ptr<AstBase> parse_fun_call();
    std::unique_ptr<AstBase> parse_branch();
    std::unique_ptr<AstBase> parse_jump();
    std::unique_ptr<AstBase> parse_return(); 

    std::unique_ptr<AstBase> parse_mem_decl();
    std::unique_ptr<AstBase> parse_load();
    std::unique_ptr<AstBase> parse_get_pointer();
    std::unique_ptr<AstBase> parse_get_element_pointer();
    std::unique_ptr<AstBase> parse_binary_expr();

};


#endif // PARSER_HPP




