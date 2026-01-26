#include"../../include/front/funcmanager.hpp"

extern Btype var_type;
void FuncManager::add_Func(string name, Btype type){
    //name为函数名称   type为函数返回值类型
    decl_func_table[name] = type;
}

Btype FuncManager::get_Functype(string name){
    if(decl_func_table.find(name) != decl_func_table.end()){
        return decl_func_table[name];
    }
    else{
        throw std::runtime_error("error: function \"" + name + "\" is not declared");
    }
}

void FuncManager::EnterStack(string name){
    func_stack.push_back(name);
}

//打印函数调用时使用  ,  从func_stack末尾弹出 num个函数参数并打印
void FuncManager::PopStack(int num){
    auto it = func_stack.end()-num;
    auto i=it;
    for(;i!=func_stack.end();i++){
        if(i!=it) //在第i个函数名前打印一个 ',' ,但是输出的第一个(i==it)前面没',' 
            std::cout<<",";
        std::cout<<*i;
    }
    func_stack.erase(it,func_stack.end());
}

void FuncManager::printSysyDeclFunc(){
    //get 
    std::cout<<"decl @getint():i32"<<std::endl;
    add_Func("getint", Btype::BINT);
    std::cout<<"decl @getch():i32"<<std::endl;
    add_Func("getch",Btype::BINT);
    std::cout<<"decl @getarray(*i32):i32"<<std::endl;
    add_Func("getarray",Btype::BINT);
    std::cout<<"decl @getfloat():f32"<<std::endl;
    add_Func("getfloat", Btype::BFLOAT);
    std::cout<<"decl @getfarray(*f32):i32"<<std::endl;
    add_Func("getfarray",Btype::BINT);

    //put
    std::cout<<"decl @putint(i32)"<<std::endl;
    add_Func("putint",Btype::BVOID);
    std::cout<<"decl @putch(i32)"<<std::endl;
    add_Func("putch",Btype::BVOID);
    std::cout<<"decl @putarray(i32,*i32)"<<std::endl;
    add_Func("putarray",Btype::BVOID);
    std::cout<<"decl @putfloat(f32)"<<std::endl;
    add_Func("putfloat",Btype::BVOID);
    std::cout<<"decl @putfarray(i32,*f32)"<<std::endl;
    add_Func("putfarray",Btype::BVOID);

    //time
    std::cout<<"decl @_sysy_starttime(i32)"<<std::endl;
    add_Func("_sysy_starttime",Btype::BVOID);
    std::cout<<"decl @_sysy_stoptime(i32)"<<std::endl;
    add_Func("_sysy_stoptime",Btype::BVOID);

    std::cout<<std::endl;
}
