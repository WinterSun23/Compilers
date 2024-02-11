#include "Lexer.h"

int main()
{
    std::stringstream code;
    std::fstream fileHandle;
    fileHandle.open("sourceCode.ash",std::ios::in);

    if(fileHandle.is_open())
    {
        code<<fileHandle.rdbuf();
        fileHandle.close();
    }
    else
    {
        std::cerr << "Unable to open file" << std::endl;
    }

    Lexer l {code};
    l.Lex();
}





