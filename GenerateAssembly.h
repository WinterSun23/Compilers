#include<iostream>
#include<fstream>
#include<string>
#include <vector>
#include <sstream>
#include <optional>
#include <unordered_set>
#include <variant>

std::unordered_set<std::string> FUNCTIONAL_KEYWORDS {"exitProg","print"};
std::unordered_set<std::string> NON_FUNCTIONAL_KEYWORDS {"int","float","double","bool","string","char","array"};
std::unordered_set<std::string> Variables;

struct NodeExpr{
    std::string expr;
    std::variant<int,bool> exprValue;
};

struct NodeExit{
    std::string exitExpr;
    NodeExpr exitValue;
};

enum TokenType{
    _integerLiteral,
    _floatLiteral,
    _doubleLiteral,
    _stringLiteral,
    _semiColon,
    _equals,
    _parenthesis,
    _flowerBrackets,
    _squareBrackets,
    _variable,
    _func_keyWord,
    _non_func_keyWord,
    _expr,
    _operator
};

struct Token{
    TokenType type;
    std::optional<std::string> value;
};

struct Variable{
    std::string name;
    int offset;
};

std::vector<Variable> _Variables;

class GenerateAssembly{
public:
    GenerateAssembly() {
        fileHandle.open("assemblySource.asm", std::ios::out);
        if (fileHandle.is_open()) {
            fileHandle << "global _start" << std::endl;
            fileHandle << "_start:" << std::endl;
        }
        else
        {
            std::cerr<<"Unable to write to assembly file"<<std::endl;
            exit(EXIT_FAILURE);
        }
    }

    void _exitProg(NodeExit& exitNode)
    {
        fileHandle<<"   mov rax, 60"<<std::endl;
        fileHandle<<"   mov rdi,"<< std::get<int>(exitNode.exitValue.exprValue)<<std::endl; //-> can throw error if an int value is not found;
        fileHandle<<"   syscall"<<std::endl;
        fileHandle<<std::endl;
    }

    void _print(const std::string& printExpr)
    {

        fileHandle<<"   sub rsp ,"<<printExpr.length()<<std::endl;
        for(int i=0;i<printExpr.length();i++)
        {
            fileHandle<<"   mov byte[rsp+"<<printExpr.length()-i<<"], "<<"'"<<printExpr[printExpr.length()-1-i]<<"'"<<std::endl;
        }
        fileHandle<<"   lea rsi, [rsp+1]"<<std::endl;
        fileHandle<<"   mov rax, 1"<<std::endl;
        fileHandle<<"   mov rdi, 1"<<std::endl;
        fileHandle<<"   mov rdx, "<<printExpr.length()<<std::endl;
        fileHandle<<"   syscall"<<std::endl;
        fileHandle<<std::endl;
    }

    void _print(int length)
    {
        fileHandle<<"   mov rsi, rsp"<<std::endl;
        fileHandle<<"   mov rax, 1"<<std::endl;
        fileHandle<<"   mov rdi, 1"<<std::endl;
        fileHandle<<"   mov rdx, "<<length<<std::endl;
        fileHandle<<"   syscall"<<std::endl;
        fileHandle<<std::endl;
    }

    void _allocateVar(std::vector<Token> tokens){
        _Variables.push_back({.name = tokens[1].value.value(),.offset = stackPointer++});
        fileHandle<<"   push "<<tokens[3].value.value()<<std::endl;
    }
    void _accessVar(const std::string& varName){ //->puts a copy of the required value and puts it at the top of the stack
        for(const Variable& x: _Variables){
            if(x.name ==varName){
                fileHandle<<"   push QWORD [rsp+"<<(stackPointer-x.offset-1)*8<<"]"<<std::endl;
            }
        }
    }

    ~GenerateAssembly()
    {
        fileHandle<<"   mov rax, 60"<<std::endl;
        fileHandle<<"   mov rdi, 0"<<std::endl;
        fileHandle<<"   syscall"<<std::endl;
        fileHandle.close();
    }
private:
    std::fstream fileHandle;
    int stackPointer =0;
};