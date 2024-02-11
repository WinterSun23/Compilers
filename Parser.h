#include "GenerateAssembly.h"

class NodeEvaluator{
private:
    int currIndex=0;
    std::vector<Token> Tokens;
    std::string expr;
public:
    static NodeExpr& evaluateExpr(NodeExpr& exprNode){ //-> changes the passed node's exprValue
        int exprValue=0;
        int i=0;
        for(char x:exprNode.expr){
            if(!isdigit(x)){
                std::cerr<<"Invalid argument"<<std::endl;
                exit(EXIT_FAILURE);
            }
            exprValue = exprValue*10 + int(x)-48;
            i+=1;
        }
        exprNode.exprValue = exprValue;
        return exprNode;
    }

    std::optional<char> peek(int ahead=1)
    {
        if(currIndex+ahead<expr.length() && currIndex+ahead>=0)
            return expr[currIndex+ahead];
        return {};
    }

    void consume()
    {
        expr.push_back(expr.at(currIndex++));
    }

    void HandleToken(char c)
    {
        //const char* type =typeid(T).name();
        switch(c)
        {
            case '=' :
                consume();
                Tokens.push_back({.type = TokenType::_equals,.value = expr});
                expr.clear();
                return;
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
            case '%':
                consume();
                Tokens.push_back({.type = TokenType::_operator,.value = expr});
                expr.clear();
                return;
            case '(' :
            case '{' :
            case '[' :
                HandleBrackets(c);
                return;
            case '\"':
            case '\'':
                HandleStringLiteral(c);
                return;
            default:
                break;
        }

        if(isalpha(c) || c=='_')
        {
            Handle_Var_KeyWord();
        }
        else if(isdigit(c) || peek(0).value()=='-' || peek(0).value()=='.')
        {
            Handle_Int_Float_Double();
        }
        else if(isspace(c) || c == '\n')
        {
            consume();
            expr.clear();
        }
        else
        {
            std::cerr<<"Invalid statement"<<std::endl;
            exit(EXIT_FAILURE);
        }
    }

    //-> assumed that the current character was -> " or '
    inline void HandleStringLiteral(char opening){
        consume();
        while(peek(0).has_value() && peek(0).value()!=opening)
        {
            switch(peek(0).value())
            {
                case '\\':
                    currIndex+=1;
                    if(peek(0).has_value() && peek(0).value() != opening)
                    {
                        char c = peek(0).value();
                        switch(c)
                        {
                            case '\'':
                            case '\"':
                            case '\\':
                                consume();
                                break;
                            case 'n':
                            case 't':
                            case 'r':
                                expr.push_back('\\');
                                consume();
                                break;
                            default:
                                std::cerr << "Invalid Syntax:Invalid character escaped:->"<<"\\"<<c<<std::endl;
                                exit(EXIT_FAILURE);
                        }
                    }
                    else
                    {
                        std::cerr << R"(Invalid Syntax:Slash must be escaped: -> "\")";
                        exit(EXIT_FAILURE);
                    }
                    continue;
            }
            consume();
        }
        Tokens.push_back({.type = TokenType::_stringLiteral,.value = expr});
        consume(); //-> to remove the closing quotes
        expr="";
    }

    //-> assumed that the current char is the beginning of the var/keyword and is valid (ie it is a  '_' or an alphabet
    inline void Handle_Var_KeyWord(){
        int alphabet= 0;
        alphabet += (peek(0).value() != '_');
        consume();
        while (peek(0).has_value()) {
            if (isalpha(peek(0).value()) || peek(0).value() == '_') {
                alphabet+= (peek(0).value() != '_');
                consume();
            }
            else if(isalnum(peek(0).value()) && alphabet){
                consume();
            }
            else
                break;
        }
        if (FUNCTIONAL_KEYWORDS.find(expr) != FUNCTIONAL_KEYWORDS.end() ) {
            Tokens.push_back({.type = TokenType::_func_keyWord, .value = expr});
            expr.clear();
        }
        else if(NON_FUNCTIONAL_KEYWORDS.find(expr) != NON_FUNCTIONAL_KEYWORDS.end()){
            Tokens.push_back({.type = TokenType::_non_func_keyWord, .value = expr});
            expr.clear();
        }
        else {
            Tokens.push_back({.type = TokenType::_variable, .value = expr});
            expr.clear();
            if (Variables.find(expr) != Variables.end())
                Variables.insert(expr);
        }
    }

    //-> assumed that the current char is the beginning of the int/float/double and is valid (ie it is a  '-' or '.' or a number

    inline void Handle_Int_Float_Double() {
        int nums = 0;
        int afterDecimal=0;
        bool decimalPoint = false;
        nums += isdigit(peek(0).value());
        consume();
        while (peek(0).has_value()) {
            if (isdigit(peek(0).value())) {
                if(decimalPoint)
                    afterDecimal+=1;
                nums += 1;
                consume();
            }
            else if (peek(0).value() == '-') { //-> include all the operators later
                if (!nums)
                    consume();
                else {
                    break;
                }
            }
            else if (peek(0).value() == '.') {
                if (!decimalPoint) {
                    if (!nums)
                        expr.push_back('0');
                    decimalPoint = true;
                    consume();
                }
                else {
                    std::cerr << "Invalid decimal point:->" << expr << peek(0).value() << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else
                break;
        }
        if (!nums || expr.back() == '.') {
            std::cerr << "Invalid decimal point/number/token:->" << expr << std::endl;
            exit(EXIT_FAILURE);
        }

        TokenType type = _integerLiteral;
        if(afterDecimal>7)   //-> check ift there are any trailing zeroes to decide if it is float or double
            type=_doubleLiteral;
        else if(afterDecimal<=7 && afterDecimal)
            type = _floatLiteral;

        Tokens.push_back({.type = type, .value = expr});
        expr.clear();
    }

    inline void HandleBrackets(char opening){
        int count[] = {0,0,0};
        consume();
        switch(opening)
        {
            case '(' :
                count[0]+=1;
                break;
            case '{':
                count[1]+=1;
                break;
            case '[':
                count[2]+=1;
                break;
            default:
                break;
        }

        while(peek(0).has_value()){
            switch(peek(0).value())
            {
                case ')' : {
                    count[0] -= 1;
                    break;
                }
                case '}': {
                    consume();
                    count[1] -= 1;
                    bool yes1 = true;
                    for (int x: count) {
                        if (x != 0) {
                            yes1 = false;
                        }
                    }
                    if (yes1) {
                        Tokens.push_back({.type = TokenType::_flowerBrackets, .value = expr});
                        expr.clear();
                        return;
                    }
                    break;
                }
                case ']':
                    consume();
                    count[2]-=1;
                    bool yes2 = true;
                    for(int x:count){
                        if(x!=0){
                            yes2 = false;
                        }
                    }
                    if(yes2){
                        Tokens.push_back({.type = TokenType::_squareBrackets,.value = expr});
                        expr.clear();
                        return;
                    }
                    break;
            }
            if (count[0] == 0 && count[1] == 0 && count[2] == 0) {
                consume();
                Tokens.push_back({.type = TokenType::_expr, .value = expr});
                expr.clear();
                return;
            }

            if(peek(0).value() == ';'){
                std::cerr<<"Invalid Brackets:Close the opened brackets"<<std::endl;
                exit(EXIT_FAILURE);
            }
            consume();
        }
    }
};

class NodeGenerator{
public:
    static NodeExit genExitNode(std::vector<Token> tokens){
        std::string exprBuff;
        for(int i=1;i<tokens.size()-1;i++){
            exprBuff.append(tokens[i].value.value());
        }
        NodeExpr exprNode{.expr = exprBuff};
        NodeExit exitNode{.exitValue = NodeEvaluator::evaluateExpr(exprNode)};
        return exitNode;
    }
};

class Parser{
public:
    explicit Parser(std::vector<Token> tokens)
        :Tokens {std::move(tokens)}
    {

    }

    void SetTokens(std::vector<Token> tokens)
    {
        Tokens = std::move(tokens);
    }
    void Parse()
    {
        if(Tokens[0].value.value() == "exitProg")
        {
            NodeExit exitNode = NodeGenerator::genExitNode(Tokens);
            generator._exitProg(exitNode);
        }
        else if (Tokens[0].value.value() == "print")
        {
            generator._accessVar(Tokens[1].value.value());
            generator._print(1);
        }

        else if(Tokens[0].type  == TokenType::_non_func_keyWord){
            generator._allocateVar(Tokens);
        }
    }



private:
    std::vector<Token> Tokens;
    GenerateAssembly generator;
};



