
%option nounput
%option noinput

%{

#include <cstdlib>
#include <string>

#include "sysy.tab.hpp"

using namespace std;

%}

linefeed      [\n]
WhiteSpace    [ \t\r]*
LineComment   "//".* 
MulLineComment  "/*"([^*]|\*+[^*/])*\*+"/"



Identifier    [a-zA-Z_][a-zA-Z0-9_]*

/*整数 10,8,16进制支持*/
Decimal       [1-9][0-9]*
Octal         0[0-7]*
Hexadecimal   0[xX][0-9a-fA-F]+

/*浮点数*/
Decimal_f ([0-9]*[\.][0-9]+|[0-9]+[\.][0-9]*)   
Octal_f 0[0-7]+[\.][0-7]+   
Hexadecimal_f 0[xX][0-9a-fA-F]+[\.][0-9a-fA-F]+   

/*十进制·浮点数·科学计数法   合法: 0e2 , .0e2 , 0.e2     不合法:.e3  */
Scientific_decf   ([0-9]+(\.[0-9]*)?|(\.[0-9]+))([eE][+-]?[0-9]+)  

/*十六进制浮点数 (科学)计数法*/
SciHex_decf    0[xX]([0-9a-fA-F]+(\.[0-9a-fA-F]*)?|(\.[0-9a-fA-F]+))([pP][+-]?[0-9]+)  

Operater   "+"|"-"|"*"|"/"|"="|">"|"<"|"!"
%%

{linefeed}    { yylineno++; }
{WhiteSpace}    { }
{LineComment}   { }
{MulLineComment} { }

"int"           { return INT; }
"return"        { return RETURN; }
"void"          { return VOID; }
"float"         { return FLOAT; }//这不是有float了吗
"const"         { return CONST;}

"if"            { return IF; }
"else"          { return ELSE; }
"while"         { return WHILE; }

"break"         { return BREAK; }
"continue"      { return CONTINUE; }

{Identifier}    { yylval.str_val = new string(yytext); return IDENT; }

{Decimal}|{Octal}|{Hexadecimal}   {
	yylval.int_val = strtol(yytext, nullptr, 0); return INT_CONST; 
}

{Decimal_f}|{Octal_f}|{Hexadecimal_f}|{Scientific_decf}|{SciHex_decf}   {
	yylval.float_val = strtof(yytext, nullptr); return FLOAT_CONST;
}
      

{Operater}      { return yytext[0]; } 

.               { return yytext[0]; }

%%

int yywrap()
{
	return 1;
}
