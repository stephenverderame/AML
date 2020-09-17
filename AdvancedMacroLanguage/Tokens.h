#pragma once
#include <string>
#include <variant>
#include <memory>
#include <type_traits>
constexpr short max_token_length = 100;
enum class TokenCategory { //must be <= 16 categories
	functions, literals, operators, control_flow, keywords, syntax
};
enum class Tokens : uint16_t {
	//standard functions
	func_print,
	func_exec,
	func_rand,
	func_lil_endian,

	//literals
	lit_section_start = (uint16_t)TokenCategory::literals << 12, //sections to quickly determine what type of token something is
	lit_short,
	lit_int,
	lit_long,
	lit_float,
	lit_dbl,
	lit_str,	
	lit_var,

	//operators
	op_section_start = (uint16_t)TokenCategory::operators << 12,
	op_plus,
	op_mul,
	op_minus,
	op_div,
	op_exp,
	op_test,
	op_gr,
	op_gre,
	op_le,
	op_lee,
	op_or,
	op_and,
	op_bit_or,
	op_bit_and,
	op_xor,
	op_mod,
	op_sh_left,
	op_sh_right,
	op_ne,
	op_bool_xor,

	//control flow
	ct_section_start = (uint16_t)TokenCategory::control_flow << 12,
	ct_if,
	ct_for,
	ct_while,
	ct_elseif,
	ct_else,

	//keywords
	kw_section_start = (uint16_t)TokenCategory::keywords << 12,
	kw_decl,

	//misc
	sx_section_start = (uint16_t)TokenCategory::syntax << 12,
	end_stment,
	end_block,
	start_block,
	end_expr,
	start_expr,
	sx_void,
	sx_comma,
	invalid = ~0
};
constexpr inline TokenCategory categoryOf(Tokens t) {
	return (TokenCategory)((uint16_t)t >> 12);
}
/**/
constexpr inline int precedence(Tokens t) {
	switch (t) {
	case Tokens::op_exp:
		return 10;
	case Tokens::op_div:
	case Tokens::op_mul:
	case Tokens::op_mod:
		return 9;
	case Tokens::op_minus:
	case Tokens::op_plus:
		return 8;
	case Tokens::op_and:
	case Tokens::op_bit_and:
	case Tokens::op_bit_or:
	case Tokens::op_xor:
	default: //functions
		return 0;
	}
}
using TokenData = std::variant<std::string, double, float, long long, long, short>;
//Represents a language token
class Token {
private:
	Tokens type;
	TokenData data;
public:
//	Token(Tokens type, TokenData& data) : type(type), data(data) {}
	Token(Tokens type) : type(type) {}
	Token() : type(Tokens::invalid) {}
	inline Tokens getType() const { return type; }
	inline TokenCategory getCategory() const { return categoryOf(type); }
	inline void setType(Tokens t) { type = t; }
	inline const std::string& getStr() const { return std::get<std::string>(data); }
	inline const double& getDbl() const { return std::get<double>(data); }
	inline const float getFlt() const { return std::get<float>(data); }
	inline const long long& getLng() const { return std::get<long long>(data); }
	inline long getInt() const { return std::get<long>(data); }
	inline short getShort() const { return std::get<short>(data); }
	inline void setVar(const TokenData&& d) { data = d; }
	inline TokenData getData() { return data; }
	inline void setData(const double& t)
	{
		data = t;
	}
	inline void setData(const std::string& t)
	{
		data = t;
	}
	inline void setData(const std::string&& t)
	{
		data = t;
	}
	inline void setData(const long long& t)
	{
		data = t;
	}
	inline void setData(const long t)
	{
		data = t;
	}
	inline void setData(const float t)
	{
		data = t;
	}
	inline void setData(const short t)
	{
		data = t;
	}
	inline void setData(const int t)
	{
		data = (long)t;
	}
	//Gets string representation of token.
	//Returns emptry string if token is not a literal
	std::string literalValue() const;

};