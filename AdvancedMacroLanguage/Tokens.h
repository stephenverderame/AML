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
	invalid = ~0
};
constexpr inline TokenCategory categoryOf(Tokens t) {
	return (TokenCategory)((uint16_t)t >> 12);
}
constexpr inline int precedence(Tokens t) {
	switch (t) {
	case Tokens::op_exp:
		return 3;
	case Tokens::op_div:
	case Tokens::op_mul:
		return 2;
	case Tokens::op_minus:
	case Tokens::op_plus:
		return 1;
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
	inline const std::string& getStr() const { return std::get<0>(data); }
	inline const double& getDbl() const { return std::get<1>(data); }
	inline const float getFlt() const { return std::get<2>(data); }
	inline const long long& getLng() const { return std::get<3>(data); }
	inline long getInt() const { return std::get<4>(data); }
	inline short getShort() const { return std::get<5>(data); }
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