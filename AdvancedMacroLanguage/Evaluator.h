#pragma once
//Computes arrangement of tokens
//Stores variables
#include "Tokens.h"
#include <vector>
class Evaluator
{
private:
	mutable std::string error;
public:
	/**
	* Evaluates an expression, which is required to be in postfix notation
	* @return a token of the largest type. Can be void. Will return Token::invalid and set the error string on error
	*/
	Token evaluate(std::vector<Token>& tokens);

	std::string getError() const { return error; }

private:
	/**
	* The largest type is the return type of an operation of two different type.
	* Ex long long + short will return a long long
	* In general from smallest to largest: short -> int -> long -> float -> double -> string
	* Invalid is returned if two types aren't compatible as operands (ie. int[] and string)
	* @return the "largest" type in the argument list or invalid on error
	*/
	Tokens largestType(std::vector<Token>::iterator argBegin, std::vector<Token>::iterator argEnd);

	/**
	* @param data the data to convert
	* @param type the type to convert to
	* @return a variant with the proper representation of data as type
	*/
	TokenData convert(const TokenData& data, Tokens type) const;



#define MATH_OPERATOR(NAME) \
	Token NAME(TokenData&& a, TokenData&& b, Tokens type) const
	//OPERATORS -----------------------------------------------------------------------------------------
	/**
	* For all operator functions
	* Requires that a and b be of type type
	* @param a, b, c ... or list    the operands
	* @param type    the type of the operands
	* @return the resulting object of the operation or invalid if error
	*/
	Token add(TokenData&& a, TokenData&& b, Tokens type) const;

	MATH_OPERATOR(sub);
	MATH_OPERATOR(div);
	MATH_OPERATOR(mul);
};

