#include "Parser.h"

class Lexer{
public:
    explicit Lexer(const std::stringstream& codeSource)
            :code {codeSource.str()},
             p {Tokens}
    {

    }

    void Lex()
    {
        while(peek().has_value()) //->Check
        {
            HandleToken(peek().value());
        }
        // -> handle a statement not having a semicolon
    }

private:
    std::string code;
    int currIndex= 0;
    std::string currTokenBuff;
    std::vector<Token> Tokens;
    Parser p {Tokens};

    std::optional<char> peek(int ahead=0)
    {
        if(currIndex+ahead<code.length() && currIndex+ahead>=0)
            return code[currIndex+ahead];
        return {};
    }

    void consume()
    {
        currTokenBuff.push_back(code.at(currIndex++));
    }

    void HandleToken(char c)
    {
        //const char* type =typeid(T).name();
        switch(c)
        {
            case ';' :
                consume();
                Tokens.push_back({.type = TokenType::_semiColon,.value = currTokenBuff});
                currTokenBuff.clear();
                for(Token x:Tokens){
                    std::cout<<x.value.value()<<","<<x.type<<std::endl;
                }
                p.SetTokens(Tokens);
                p.Parse();
                Tokens.clear();
                return;
            case '=' :
                consume();
                Tokens.push_back({.type = TokenType::_equals,.value = currTokenBuff});
                currTokenBuff.clear();
                return;
            case '+':
            case '-':
            case '*':
            case '/':
            case '^':
            case '%':
                consume();
                Tokens.push_back({.type = TokenType::_operator,.value = currTokenBuff});
                currTokenBuff.clear();
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
        else if(isdigit(c) || peek().value()=='-' || peek().value()=='.')
        {
            Handle_Int_Float_Double();
        }
        else if(isspace(c) || c == '\n')
        {
            consume();
            currTokenBuff.clear();
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
        currTokenBuff.clear();
        while(peek().has_value() && peek().value()!=opening)
        {
            switch(peek().value())
            {
                case '\\':
                    currIndex+=1;
                    if(peek().has_value() && peek().value() != opening)
                    {
                        char c = peek().value();
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
                                currTokenBuff.push_back('\\');
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
        Tokens.push_back({.type = TokenType::_stringLiteral,.value = currTokenBuff});
        consume(); //-> to remove the closing quotes
        currTokenBuff.clear();
    }

    //-> assumed that the current char is the beginning of the var/keyword and is valid (ie it is a  '_' or an alphabet
    inline void Handle_Var_KeyWord(){
        int alphabet= 0;
        alphabet += (peek().value() != '_');
        consume();
        while (peek().has_value()) {
            if (isalpha(peek().value()) || peek().value() == '_') {
                alphabet+= (peek().value() != '_');
                consume();
            }
            else if(isalnum(peek().value()) && alphabet){
                consume();
            }
            else
                break;
        }
        if (FUNCTIONAL_KEYWORDS.find(currTokenBuff) != FUNCTIONAL_KEYWORDS.end() ) {
            Tokens.push_back({.type = TokenType::_func_keyWord, .value = currTokenBuff});
        }
        else if(NON_FUNCTIONAL_KEYWORDS.find(currTokenBuff) != NON_FUNCTIONAL_KEYWORDS.end()){
            Tokens.push_back({.type = TokenType::_non_func_keyWord, .value = currTokenBuff});
        }
        else {
            Tokens.push_back({.type = TokenType::_variable, .value = currTokenBuff});
            if (Variables.find(currTokenBuff) != Variables.end())
                Variables.insert(currTokenBuff);
        }
        currTokenBuff.clear();
    }

    //-> assumed that the current char is the beginning of the int/float/double and is valid (ie it is a  '-' or '.' or a number

    inline void Handle_Int_Float_Double() {
        int nums = 0;
        std::string afterDecimal;
        bool decimalPoint = false;
        nums += isdigit(peek().value());
        consume();
        while (peek().has_value()) {
            if (isdigit(peek().value())) {
                if(decimalPoint)
                    afterDecimal+=peek().value();
                nums += 1;
                consume();
            }
            else if (peek().value() == '-') {
                if (!nums)
                    consume();
                else {
                    break;
                }
            }
            else if (peek().value() == '.') {
                if (!decimalPoint) {
                    if (!nums)
                        currTokenBuff.push_back('0');
                    decimalPoint = true;
                    consume();
                }
                else {
                    std::cerr << "Invalid decimal point:->" << currTokenBuff << peek(0).value() << std::endl;
                    exit(EXIT_FAILURE);
                }
            }
            else
                break;
        }
        if (!nums || currTokenBuff.back() == '.') {
            std::cerr << "Invalid decimal point/number/token:->" << currTokenBuff << std::endl;
            exit(EXIT_FAILURE);
        }

        [[maybe_unused]] TokenType type = _integerLiteral;
        if(afterDecimal.length()>7)   //-> check if there are any trailing zeroes to decide if it is float or double
            type=_doubleLiteral;
        else if(afterDecimal.length()<=7 && !afterDecimal.empty())
            type = _floatLiteral;

        Tokens.push_back({.type = type, .value = currTokenBuff});
        currTokenBuff.clear();
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

        while(peek().has_value()){
            switch(peek().value())
            {
                case '(' :
                    count[0] += 1;
                    break;
                case '{':
                    count[1] += 1;
                    break;
                case '[':
                    count[2] +=1;
                    break;
                case ')' :
                    count[0] -= 1;
                    break;
                case '}':
                    count[1] -= 1;
                    break;
                case ']':
                    count[2] -=1;
                    break;
            }
            if (count[0] == 0 && count[1] == 0 && count[2] == 0) {
                consume();
                Tokens.push_back({.type = TokenType::_expr, .value = currTokenBuff});
                currTokenBuff.clear();
                return;
            }

            if(peek().value() == ';'){
                std::cerr<<"Invalid Brackets:Close the opened brackets"<<std::endl;
                exit(EXIT_FAILURE);
            }
            consume();
        }
    }
};

