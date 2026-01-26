#include <fstream>

#include "../../include/back/IRtoAsm.hpp"
#define OPTIMIZE 1

extern std::unique_ptr<Program> program;
extern std::string outpath;

int IRanalysis(std::string inputfile, std::string outputfile)
{
    outpath = outputfile.substr(0, outputfile.find_last_of("."));

    std::ifstream in(inputfile);
    std::string str = "";
    std::string temp;
    while (getline(in, temp))
    {
        str += temp;
        str += "\n";
    }


    Builder builder(str);
    koopa_raw_program_t *raw = builder.build();
    std::cout << "build success!" << std::endl;

    Optimizer optimizer;


    std::ofstream out(outputfile);
    std::streambuf *coutbuf = std::cout.rdbuf();
    std::cout.rdbuf(out.rdbuf());

    // 进行活变量分析
    Live_Var(program.get());

    optimizer.init_used_by(program.get());
    raw = program->buildon();
    // 生成汇编代码
    Visit_top_program(*raw);
    std::cout.rdbuf(coutbuf);

    return 0;
}