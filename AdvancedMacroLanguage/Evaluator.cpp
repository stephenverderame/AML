#include "Evaluator.h"
#include <time.h>
#include <unordered_map>
#include <stack>
#include "Stream.h"
#include "CodePage.h"
//Linked stack of scopes
//Invariant, root is the smallest scope, scopes are deleted as they are exited
struct Evaluator::data {
    std::unordered_map<std::string, Token> scope;
    data* child;

    data() : child(nullptr) {}

};
Token Evaluator::evaluate(std::vector<Token>& tokens)
{
    if (tokens.empty()) return Tokens::invalid;
    switch (tokens[tokens.size() - 1].getCategory()) {
    case TokenCategory::literals:
        return tokens[0];
//        return evalLit(tokens[0]);
    case TokenCategory::functions:
        return evalFunc(tokens);
    case TokenCategory::keywords:
        return evalKeys(tokens);
    case TokenCategory::operators:
        return evalOp(tokens);
    case TokenCategory::control_flow:
        return evalControl(tokens);
    }
    error = "Unrecognized token category";
    return Tokens::invalid;
}

Evaluator::Evaluator(Stream& outputStream, CodePage& code) : str(outputStream), code(&code)
{
    vars = new data();
}

Evaluator::~Evaluator()
{
    delete vars;
}

void Evaluator::newScope()
{
    data* newScope = new data();
    newScope->child = vars;
    vars = newScope;
}

void Evaluator::popScope()
{
    data* top = vars;
    vars = vars->child;
    delete top;
}

Tokens Evaluator::largestType(std::vector<Token>::iterator argBegin, std::vector<Token>::iterator argEnd)
{
    Tokens t = (Tokens)0;
    //TODO: compatability checking
    for (auto it = argBegin; it != argEnd; ++it) {
        if ((uint16_t)it->getType() > (uint16_t)t)
            t = it->getType();
    }
    return t;
}

TokenData Evaluator::convert(const TokenData& data, Tokens type) const
{
    TokenData t;
    std::visit([&type, &data, this, &t](TokenData&& var) {
        if (std::holds_alternative<long>(data)) {
            long d = std::get<long>(data);
            switch (type) {
            case Tokens::lit_float:
                t = (float)d;
                break;
            case Tokens::lit_long:
                t = (long long)d;
                break;
            case Tokens::lit_dbl:
                t = (double)d;
                break;
            case Tokens::lit_str:
                t = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
        else if (std::holds_alternative<std::string>(data)) {
            t = std::get<std::string>(data);
        }
        else if (std::holds_alternative<long long>(data)) {
            long long d = std::get<long long>(data);
            switch (type) {
            case Tokens::lit_float:
                t = (float)d;
                break;
            case Tokens::lit_dbl:
                t = (double)d;
                break;
            case Tokens::lit_str:
                t = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
        else if (std::holds_alternative<float>(data)) {
            float d = std::get<float>(data);
            switch (type) {
            case Tokens::lit_dbl:
                t = (double)d;
                break;
            case Tokens::lit_str:
                t = std::to_string(d);
                break;
            default:
                t = d;
                break;
            }
        }
        else if (std::holds_alternative<double>(data)) {
            double d = std::get<double>(data);
            switch (type) {
            case Tokens::lit_str:
                t = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
        else if (std::holds_alternative<short>(data)) {
            short d = std::get<short>(data);
            switch (type) {
            case Tokens::lit_int:
                t = (long)d;
                break;
            case Tokens::lit_float:
                t = (float)d;
                break;
            case Tokens::lit_long:
                t = (long long)d;
                break;
            case Tokens::lit_dbl:
                t = (double)d;
                break;
            case Tokens::lit_str:
                t = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
    }, data);
    return t;
}

Token Evaluator::evalLit(Token& t)
{
	switch (t.getType()) {
	case Tokens::lit_var:
    {
        data* scope = vars;
        while (scope != nullptr) {
            if (scope->scope.find(t.getStr()) != scope->scope.end())
                return scope->scope[t.getStr()];
            else scope = scope->child;
        }
        error = "Variable " + t.getStr() + " is undefined";
        return Tokens::invalid;
    }
	case Tokens::lit_code:
		return code->eval(t, *this);
	default:
		return t;
	}
    return Tokens::invalid;
}

Token Evaluator::evalOp(std::vector<Token>& tokens)
{
    Token& operation = tokens[tokens.size() - 1];
    Tokens t = operation.getType();
    Token res;
    if(t != Tokens::op_eq) resolveLiterals(tokens);
    res.setType(largestType(tokens.begin(), tokens.begin() + tokens.size() - 1));
    if (res.getType() != Tokens::invalid) {
        for (int i = 0; i < tokens.size() - 1; ++i) {
            if (tokens[i].getType() != res.getType()) {
                tokens[i].setType(res.getType());
                tokens[i].setVar(convert(tokens[i].getData(), res.getType()));
            }
        }
        size_t arguments = tokens.size() - 1;
        switch (operation.getType()) {
        case Tokens::op_plus:
            if (arguments != 2) error = "Invalid number of arguments for operator +";
            else res = add(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_minus:
            if (arguments != 2) error = "Invalid number of arguments for operator -";
            else res = sub(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_mul:
            if (arguments != 2) error = "Invalid number of arguments for operator *";
            else res = mul(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_div:
            if (arguments != 2) error = "Invalid number of arguments for operator /";
            else res = div(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_exp:
            if (arguments != 2) error = "Invalid number of arguments for operator **";
            else res = power(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_test:
            if (arguments != 2) error = "Invalid number of arguments for operator ==";
            else res = test(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_ne:
            if (arguments != 2) error = "Invalid number of arguments for operator !=";
            else res = not_test(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_and:
            if (arguments != 2) error = "Invalid number of arguments for operator &&";
            else res = bool_and(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_or:
            if (arguments != 2) error = "Invalid number of arguments for operator ||";
            else res = bool_or(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_bit_and:
            if (arguments != 2) error = "Invalid number of arguments for operator &";
            else res = bit_and(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_bit_or:
            if (arguments != 2) error = "Invalid number of arguments for operator |";
            else res = bit_or(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_xor:
            if (arguments != 2) error = "Invalid number of arguments for operator ^";
            else res = bit_xor(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_bool_xor:
            if (arguments != 2) error = "Invalid number of arguments for operator ^^";
            else res = bool_xor(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_sh_left:
            if (arguments != 2) error = "Invalid number of arguments for operator <<";
            else res = shift_left(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_sh_right:
            if (arguments != 2) error = "Invalid number of arguments for operator >>";
            else res = shift_right(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_gr:
            if (arguments != 2) error = "Invalid number of arguments for operator >";
            else res = greater(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_gre:
            if (arguments != 2) error = "Invalid number of arguments for operator >=";
            else res = greater_eq(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_le:
            if (arguments != 2) error = "Invalid number of arguments for operator <";
            else res = less(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_lee:
            if (arguments != 2) error = "Invalid number of arguments for operator <=";
            else res = less_eq(tokens[0].getData(), tokens[1].getData(), res.getType());
            break;
        case Tokens::op_eq:
            if (arguments != 2) error = "Invalid number of arguments for operator =";
            else {
                res = tokens[1];
                data* scope = vars;
                while (scope != nullptr) {
                    if (scope->scope.find(tokens[0].getStr()) != scope->scope.end()) {
                        scope->scope[tokens[0].getStr()] = tokens[1];
                        break;
                    }
                    else scope = scope->child;
                }
            }
            break;
        default:
            error = "Invalid operation";
            res.setType(Tokens::invalid);
        }
    }
    return res;
}

Token Evaluator::evalFunc(std::vector<Token>& tokens)
{
    Token& operation = tokens[tokens.size() - 1];
    Tokens t = operation.getType();
    Token res;
	size_t arguments = tokens.size() - 1;
    resolveLiterals(tokens);
	switch (operation.getType()) {
	case Tokens::func_print:
        for (size_t i = 0; i < arguments; ++i) {
            if (tokens[i].getType() != Tokens::invalid) {
                if (tokens[i].getType() == Tokens::lit_str) fputs(tokens[i].literalValue().c_str(), str);
                else {
                    std::string s = tokens[i].literalValue();
                    fputs(s.c_str(), str);
                }
            }
        }
		res.setType(Tokens::sx_void);
		break;
	case Tokens::func_rand:
		if (arguments == 2) { //min and max
			srand(clock());
			TokenData min = convert(tokens[0].getData(), Tokens::lit_int), max = convert(tokens[1].getData(), Tokens::lit_int);
			res.setData(rand() % (std::get<long>(max) - std::get<long>(min)) + std::get<long>(min));
			res.setType(Tokens::lit_int);
		}
		else { //0 to 1
			srand(clock());
			res.setData(rand() / (double)RAND_MAX);
			res.setType(Tokens::lit_dbl);
		}
		break;
	case Tokens::func_lil_endian:
	{
		long num = 1;
		if (*(char*)&num == 1) res.setData((short)1);
		else res.setData((short)0);
		res.setType(Tokens::lit_short);
		break;
	}
    default:
        error = "Invalid operation";
        res.setType(Tokens::invalid);
	}
    return res;
}

Token Evaluator::evalKeys(std::vector<Token>& tokens)
{
    Token& operation = tokens[tokens.size() - 1];
    Tokens t = operation.getType();
    Token res;
	size_t arguments = tokens.size() - 1;
	switch (operation.getType()) {
	case Tokens::kw_decl:
		if (arguments != 1) error = "Invalid number of arguments for operator decl";
        else {
            vars->scope[tokens[0].getStr()];
            res = tokens[0];
        }
		break;
	case Tokens::kw_true:
		res.setType(Tokens::lit_short);
		res.setData((short)1);
		break;
	case Tokens::kw_false:
		res.setType(Tokens::lit_short);
		res.setData((short)0);
		break;
    case Tokens::kw_exec:
    {
        newScope();
        for (int i = 1; i < tokens.size() - 1; ++i) {
            vars->scope["args_" + std::to_string(i - 1)] = tokens[i];
        }
        Token aLength = Tokens::lit_int;
        aLength.setData((long)tokens.size() - 2);
        vars->scope["args_length"] = aLength;
        if (tokens[0].getType() == Tokens::lit_var) {
            res = evalLit(tokens[0]);
            res = evalLit(res);
        }
        else
            res = evalLit(tokens[0]);
        popScope();
        break;
    }
    case Tokens::kw_return:
        if (arguments > 1) error = "Too many arguments for return";
        else {
            res.setType(Tokens::kw_return);
            res.setData("return_value");
            vars->scope["return_value"] = tokens[0];
        }
        break;
	default:
		error = "Invalid operation";
		res.setType(Tokens::invalid);
	}
    return res;
}

Token Evaluator::evalControl(std::vector<Token>& tokens)
{
    Token& operation = tokens[tokens.size() - 1];
    Tokens t = operation.getType();
    Token res;
    size_t arguments = tokens.size() - 1;
    switch (operation.getType()) {
    case Tokens::ct_if:
        tokens[0] = evalLit(tokens[0]);
        if (std::get<long>(convert(tokens[0].getData(), Tokens::lit_int)) != 0)
            evalLit(tokens[1]);
        return Tokens::sx_void;
        break;
    }
    return Tokens::invalid;

}

Token Evaluator::add(TokenData&& a, TokenData&& b, Tokens type) const
{
    Token t;
    t.setType(type);
    switch (type) {
    case Tokens::lit_short:
        t.setData((short)(std::get<short>(a) + std::get<short>(b)));
        break;
    case Tokens::lit_int:
        t.setData(std::get<long>(a) + std::get<long>(b));
        break;
    case Tokens::lit_long:
        t.setData(std::get<long long>(a) + std::get<long long>(b));
        break;
    case Tokens::lit_float:
        t.setData(std::get<float>(a) + std::get<float>(b));
        break;
    case Tokens::lit_dbl:
        t.setData(std::get<double>(a) + std::get<double>(b));
        break;
    case Tokens::lit_str:
        t.setData(std::get<std::string>(a) + std::get<std::string>(b));
        break;
    default:
        error = "Type id " + std::to_string((uint16_t)type) + " unsupported as an operand for addition";
        t.setType(Tokens::invalid);
        break;
    }
    return t;
}
#define MATH_OPERATOR_DEF(NAME, OP) \
Token Evaluator::NAME(TokenData&& a, TokenData&& b, Tokens type) const \
{ \
    Token t; \
    t.setType(type); \
    switch (type) { \
    case Tokens::lit_short: \
        t.setData((short)(std::get<short>(a) OP std::get<short>(b))); \
        break; \
    case Tokens::lit_int: \
        t.setData(std::get<long>(a) OP std::get<long>(b)); \
        break; \
    case Tokens::lit_long: \
        t.setData(std::get<long long>(a) OP std::get<long long>(b)); \
        break; \
    case Tokens::lit_float: \
        t.setData(std::get<float>(a) OP std::get<float>(b)); \
        break; \
    case Tokens::lit_dbl: \
        t.setData(std::get<double>(a) OP std::get<double>(b)); \
        break; \
    default: \
        error = "Type id " + std::to_string((uint16_t)type) + " unsupported as an operand for operator '" #OP "'"; \
        t.setType(Tokens::invalid); \
        break; \
    } \
    return t; \
}

#define CUSTOM_OPERATOR_DEF(NAME, FUNC, FUNCSTR) \
Token Evaluator::NAME(TokenData&& a, TokenData&& b, Tokens type) const \
{ \
    Token t; \
    t.setType(type); \
    switch (type) { \
    case Tokens::lit_short: \
        t.setData((short)(FUNC(std::get<short>(a), std::get<short>(b)))); \
        break; \
    case Tokens::lit_int: \
        t.setData((long)(FUNC(std::get<long>(a), std::get<long>(b)))); \
        break; \
    case Tokens::lit_long: \
        t.setData((long long)(FUNC(std::get<long long>(a), std::get<long long>(b)))); \
        break; \
    case Tokens::lit_float: \
        t.setData((float)(FUNC(std::get<float>(a), std::get<float>(b)))); \
        break; \
    case Tokens::lit_dbl: \
        t.setData((double)(FUNC(std::get<double>(a), std::get<double>(b)))); \
        break; \
    case Tokens::lit_str: \
        t.setData(FUNCSTR(std::get<std::string>(a), std::get<std::string>(b))); \
        break; \
    default: \
        error = "Type id " + std::to_string((uint16_t)type) + " unsupported as an operand for operator '" #FUNC "'"; \
        t.setType(Tokens::invalid); \
        break; \
    } \
    return t; \
}
#define IGNORE_FUNC(A, B) "Error"
#define CUSTOM_MATH_OPERATOR_DEF(NAME, FUNC) CUSTOM_OPERATOR_DEF(NAME, FUNC, IGNORE_FUNC)

MATH_OPERATOR_DEF(sub, -);
MATH_OPERATOR_DEF(mul, *);
MATH_OPERATOR_DEF(div, /);
#define BOOL_EQ(A, B) A == B
#define BOOL_UNEQ(A, B) A != B
CUSTOM_OPERATOR_DEF(test, BOOL_EQ, BOOL_EQ);
CUSTOM_OPERATOR_DEF(not_test, BOOL_UNEQ, BOOL_UNEQ);
MATH_OPERATOR_DEF(bool_and, &&);
MATH_OPERATOR_DEF(bool_or, ||);

#define BIT_AND(A, B) ((long long)(A) & (long long)(B))
#define BIT_OR(A, B) ((long long)(A) | (long long)(B))
#define BIT_XOR(A, B) ((long long)(A) ^ (long long)(B))
#define MOD(A, B) ((long long)(A) % (long long)(B))
#define SHIFTL(A, B) ((long long)(A) << (long long)(B))
#define SHIFTR(A, B) ((long long)(A) >> (long long)(B))
CUSTOM_MATH_OPERATOR_DEF(bit_and, BIT_AND);
CUSTOM_MATH_OPERATOR_DEF(bit_or, BIT_OR);
CUSTOM_MATH_OPERATOR_DEF(bit_xor, BIT_XOR);
CUSTOM_MATH_OPERATOR_DEF(power, pow);
CUSTOM_MATH_OPERATOR_DEF(mod, MOD);
CUSTOM_MATH_OPERATOR_DEF(shift_left, SHIFTL);
CUSTOM_MATH_OPERATOR_DEF(shift_right, SHIFTR);
#define BOOL_LESS(A, B) A < B
CUSTOM_OPERATOR_DEF(less, BOOL_LESS, BOOL_LESS);
#define BOOL_GREATER(A, B) A > B
CUSTOM_OPERATOR_DEF(greater, BOOL_GREATER, BOOL_GREATER)
#define BOOL_LESS_EQ(A, B) A <= B
CUSTOM_OPERATOR_DEF(less_eq, BOOL_LESS_EQ, BOOL_LESS_EQ)
#define BOOL_GREATER_EQ(A, B) A >= B
CUSTOM_OPERATOR_DEF(greater_eq, BOOL_GREATER_EQ, BOOL_GREATER_EQ)
#define BOOL_XOR(A, B) ((A || B) && (!A || !B))
CUSTOM_MATH_OPERATOR_DEF(bool_xor, BOOL_XOR);

