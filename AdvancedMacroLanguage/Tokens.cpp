#include "Tokens.h"

std::string Token::literalValue() const
{
    switch (type) {
    case Tokens::lit_str:
    case Tokens::lit_var:
        return getStr();
    case Tokens::lit_dbl:
        return std::to_string(getDbl());
    case Tokens::lit_float:
        return std::to_string(getFlt());
    case Tokens::lit_int:
        return std::to_string(getInt());
    case Tokens::lit_long:
        return std::to_string(getLng());
    case Tokens::lit_short:
        return std::to_string(getShort());
    default:
        return "";
    }
}
