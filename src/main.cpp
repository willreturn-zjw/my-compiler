#include <cassert>
#include <cstdio>
#include <iostream>
#include <memory>
#include <string>
#include <fstream>
#include <sstream>

#include "../include/front/variable.hpp"
#include "../include/front/ast.hpp"
#include "../include/front/basicblock.hpp"
#include "../include/front/funcmanager.hpp"

#include "../include/back/IRtoAsm.hpp"

// #include <chrono>

using namespace std;
Val_Table val_table;

extern FILE *yyin;
extern int yyparse(std::unique_ptr<BaseAST> &ast);


int now=0;
Btype var_type;
Btype func_ret_type;

std::vector<std::string> params_tmp;

BasicBlockManager bbm;//基本块管理工具
FuncManager funcm;//函数管理工具
ArrayManager arraym;//数组管理工具

std::string outpath;


int main(int argc, const char *argv[]) {

  string parsefilename;
  string outfilename;

  string input, output;
  bool mode=false;

  for (int i = 1; i < argc; ++i) {
      string arg = argv[i];
      if (arg == "-S") {
          mode = true;
      } else if (arg == "-o" && i + 1 < argc) {
          output = argv[++i];
      } else if (arg.size() > 2 && arg.substr(0, 2) == "-O") {
          continue;
      } else if (!arg.empty() && arg[0] != '-') {
          input = arg;
      }
  }


  // 打开输入文件, 并且指定 lexer 在解析的时候读取这个文件
  yyin = fopen(input.c_str(), "r");
  assert(yyin);
  if(yyin == nullptr) {
    cerr << "Failed to open input file: " << input <<" -"<<mode<< endl;
    return 1;
  }
  

  parsefilename= "parse.kp";

  std::ofstream outFile(parsefilename);
  if (!outFile) {
      std::cerr << "Failed to open file for writing.\n";
      return 1;
  }
  std::streambuf *coutBuf = std::cout.rdbuf();
  std::cout.rdbuf(outFile.rdbuf());           
  
  
  unique_ptr<BaseAST> ast;
  auto ret = yyparse(ast);
  assert(!ret);
  //sysy库的函数声明
  funcm.printSysyDeclFunc();
  //体
  ast->dump();

  std::cout.rdbuf(coutBuf); 
  outFile.close();


  if(mode){
    outfilename=output;
    IRanalysis(parsefilename,outfilename);

    return 0;
  }
  return 0;
}


