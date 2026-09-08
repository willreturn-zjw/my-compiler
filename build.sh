set -e
set -x

mkdir -p build

SRC_DIR="src"
FRONT_DIR="$SRC_DIR/front"

BISON_CPP="$FRONT_DIR/sysy.tab.cpp"
BISON_H="$FRONT_DIR/sysy.tab.h"
LEX_CPP="$FRONT_DIR/sysy.lex.cpp"

echo "Generating parser with Bison..."
bison -d -o $BISON_CPP $FRONT_DIR/sysy.y

echo "Generating lexer with Flex..."
flex -o $LEX_CPP $FRONT_DIR/sysy.l

for dir in `ls -d src/*/`; do
    dname=`basename $dir`
    for f in `ls $dir/*.cpp`; do
        echo "Compiling: $f"
        if [[ "$f" == "$BISON_CPP" ]]; then
            clang++ -std=c++17 -O2 -c -o "build/front-sysy.tab.o" $f
        elif [[ "$f" == "$LEX_CPP" ]]; then
            clang++ -std=c++17 -O2 -c -o "build/front-sysy.lex.o" $f
        else
            clang++ -std=c++17 -O2 -c -o "build/$dname-`basename $f .cpp`.o" $f
        fi
    done
done

echo "Compiling: src/main.cpp"
clang++ -std=c++17 -O2 -c -o build/main.o src/main.cpp

echo "Linking..."
link_cmd="clang++ -lm -o test/compiler build/*.o"

link_output=$($link_cmd 2>&1)
link_status=$?

if [ $link_status -ne 0 ]; then
    echo "链接失败！错误详情：" >&2
    echo "$link_output" >&2
    
    undefined_symbols=$(echo "$link_output" | grep "undefined reference to")
    if [ -n "$undefined_symbols" ]; then
        echo -e "\n未定义的符号：" >&2
        echo "$undefined_symbols" >&2
        
        missing_functions=$(echo "$undefined_symbols" | grep -o 'undefined reference to `[^`]*' | grep -o '[^`]*$')
        echo -e "\n可能缺失的实现：" >&2
        for func in $missing_functions; do
            echo "- $func" >&2
        done
    fi
    
    exit $link_status
fi
