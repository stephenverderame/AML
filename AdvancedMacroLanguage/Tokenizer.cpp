#include "Tokenizer.h"
#include "CompileTimeHash.h"
#include <sstream>
constexpr Tuple<const char*, Tokens> tokenList[] = {
    {"print", Tokens::func_print}, {"random", Tokens::func_rand}, {"+", Tokens::op_plus}, {"-", Tokens::op_minus}, {"/", Tokens::op_div},
    {"**", Tokens::op_exp}, {"*", Tokens::op_mul}, {"<", Tokens::op_le}, {"<=", Tokens::op_lee}, {">", Tokens::op_gr}, {">=", Tokens::op_gre},
    {"==", Tokens::op_test}, {"&&", Tokens::op_and}, {"||", Tokens::op_or}, {"|", Tokens::op_bit_or}, {"&", Tokens::op_bit_and}, {"%", Tokens::op_mod},
    {"^", Tokens::op_xor}, {"!=", Tokens::op_ne}, {"<<", Tokens::op_sh_left}, {">>", Tokens::op_sh_right}, {"^^", Tokens::op_bool_xor},
    {"isLittleEndian", Tokens::func_lil_endian}
};
static constexpr CompileTimeHash<const char*, Tokens> tokenHash(tokenList, sizeof(tokenList) / sizeof(Tuple<const char *, Tokens>), Tokens::invalid);

Token Tokenizer::getToken()
{
    Token t;
    char c = fgetc(input);
    errorToken = c;
    while (c != EOF && (c == ' ' || c == '\r' || c == '\n' || c == '\t')) //ignore leading whitespaces
        c = fgetc(input);
    if (c == '-' || isdigit(c) || c == '.') {
        char buf[max_token_length + 1];
        short i = 0;
        bool floating = false;
        if (c == '-') {
            char c2 = fgetc(input);
            if (isdigit(c2) || (c2 == '.' && (floating = true))) {
                buf[i++] = c;
                buf[i++] = c2;
                c = fgetc(input);
            }
            else {
                ungetc(c2, input);
                goto handleOperators; //I think this is my first goto 
            }
        }
        do {
            buf[i++] = c;
            c = fgetc(input);
        } while (i < max_token_length && (isdigit(c) || (c == '.' && (floating = true)))); //sets floating to true if c == '.' Loops if c is a digit or c == '.'
        buf[i] = '\0';
        if (floating && c == 'f') {
            t.setData(std::stof(buf));
            t.setType(Tokens::lit_float);
        }
        else if (floating) {
            t.setData(std::stod(buf));
            t.setType(Tokens::lit_dbl);
        }
        else if (c == 'L') {
            t.setData(std::stoll(buf));
            t.setType(Tokens::lit_long);
        }
        else {
            t.setData(std::stoi(buf));
            t.setType(Tokens::lit_int);
        }
        if(c != 'f' && c != 'L') ungetc(c, input);

    }
    else if (c == '"' || c == '\'') {
        std::stringstream ss;
        char lastC = c;
        bool escaped = false;
        while ((c = fgetc(input)) != EOF && ((c != '"' && c != '\'') || lastC == '\\')) {
            if(c != '\\' && lastC != '\\') ss << c;
            if (lastC == '\\' && !escaped) {
                switch (c) {
                case 'n':
                    ss << '\n';
                    break;
                case 't':
                    ss << '\t';
                    break;
                case 'r':
                    ss << '\r';
                    break;
                case '\\':
                    ss << '\\';
                    escaped = true;
                    break;
                case '\'':
                    ss << '\'';
                    break;
                case '\"':
                    ss << '"';
                    break;
                case 'v':
                    ss << '\v';
                    break;
                case 'b':
                    ss << '\b';
                    break;
                default:
                    ss << c;
                }
            }
            else if (lastC == '\\') {
                ss << c;
                escaped = false;
            }
            lastC = c;
        } 
        t.setData(ss.str());
        t.setType(Tokens::lit_str);
    }
    else if (isOperator(c)){
        handleOperators:
        char buf[max_token_length + 1];
        short i = 0;
        do {
            buf[i++] = c;
            c = fgetc(input);
        } while (i < max_token_length && c != EOF && isOperator(c));
        buf[i] = '\0';
        Tokens type;
        t.setType(tokenHash.getifValid(buf, type) ? type : Tokens::invalid);
        if (t.getType() == Tokens::invalid) errorToken = buf;
        ungetc(c, input);
    }
    else if (isalpha(c)) {
        char buf[max_token_length + 1];
        short i = 0;
        do {
            buf[i++] = c;
            c = fgetc(input);
        } while (i < max_token_length && c != EOF && (isalnum(c) || c == '_'));
        buf[i] = '\0';
        Tokens type;
        if (tokenHash.getifValid(buf, type)) {
            t.setType(type);
        }
        else if (i == max_token_length && c != EOF && (isalnum(c) || c == '_')) {
            t.setType(Tokens::invalid);
            errorToken = buf;
        }
        else {
            t.setType(Tokens::lit_var);
            t.setData(buf);
        }
        ungetc(c, input);
    }
    else if (c == '{') t.setType(Tokens::start_block);
    else if (c == '}') t.setType(Tokens::end_block);
    else if (c == ';') t.setType(Tokens::end_stment);
    else if (c == '(') t.setType(Tokens::start_expr);
    else if (c == ')') t.setType(Tokens::end_expr);
    else if (c == ',') t.setType(Tokens::sx_comma);
    return t;
}
bool Tokenizer::isOperator(char c)
{
    switch (c) {
    case '*':
    case '+':
    case '-':
    case '&':
    case '!':
    case '%':
    case '^':
    case '|':
    case '=':
    case '.':
    case '>':
    case '<':
    case '/':
        return true;
    default:
        return false;
    }
}

const char* Tokenizer::reverseLookup(Tokens t) const
{
    for (auto& token : tokenList) {
        if (token.v == t) return token.k;
    }
    return "";
}


