#include "Evaluator.h"

Token Evaluator::evaluate(std::vector<Token>& tokens)
{
    if (tokens.empty()) return Tokens::invalid;
    if (tokens.size() == 1) {
        //TODO variable resolution
        return tokens[0];
    }
    Token& operation = tokens[tokens.size() - 1];
    Token res;
    res.setType(largestType(tokens.begin(), tokens.begin() + tokens.size() - 1));
    if (res.getType() != Tokens::invalid) {
        for (int i = 0; i < tokens.size() - 1; ++i) {
            tokens[i].setType(res.getType());
            tokens[i].setVar(convert(tokens[i].getData(), res.getType()));
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
        default:
            error = "Invalid operation";
            res.setType(Tokens::invalid);
        }
    }
    return res;
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
                std::get<float>(t) = d;
                break;
            case Tokens::lit_long:
                std::get<long long>(t) = d;
                break;
            case Tokens::lit_dbl:
                std::get<double>(t) = d;
                break;
            case Tokens::lit_str:
                std::get<std::string>(t) = std::to_string(d);
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
                std::get<float>(t) = d;
                break;
            case Tokens::lit_dbl:
                std::get<double>(t) = d;
                break;
            case Tokens::lit_str:
                std::get<std::string>(t) = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
        else if (std::holds_alternative<float>(data)) {
            float d = std::get<float>(data);
            switch (type) {
            case Tokens::lit_dbl:
                std::get<double>(t) = d;
                break;
            case Tokens::lit_str:
                std::get<std::string>(t) = std::to_string(d);
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
                std::get<std::string>(t) = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
        else if (std::holds_alternative<short>(data)) {
            short d = std::get<short>(data);
            switch (type) {
            case Tokens::lit_int:
                std::get<long>(t) = d;
                break;
            case Tokens::lit_float:
                std::get<float>(t) = d;
                break;
            case Tokens::lit_long:
                std::get<long long>(t) = d;
                break;
            case Tokens::lit_dbl:
                std::get<double>(t) = d;
                break;
            case Tokens::lit_str:
                std::get<std::string>(t) = std::to_string(d);
                break;
            default:
                t = d;
            }
        }
    }, data);
    return t;
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
        error = "Type id " + std::to_string((uint16_t)type) + " unsupported as an operand for addition"; \
        t.setType(Tokens::invalid); \
        break; \
    } \
    return t; \
}

MATH_OPERATOR_DEF(sub, -);
MATH_OPERATOR_DEF(mul, *);
MATH_OPERATOR_DEF(div, /);
