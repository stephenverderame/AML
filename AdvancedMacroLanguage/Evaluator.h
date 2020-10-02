#pragma once
//Computes arrangement of tokens
//Stores variables
#include "Tokens.h"
#include <vector>
#include <unordered_map>
class Evaluator
{
private:
	mutable std::string error;
	struct data;
	data* vars;
	//Linked stack of scopes of variables
	//Invariant: vars is not null
	FILE* str;
	//STR is not an owned resource
	class CodePage* code;
	//code is not an ownded resource
public:
	/**
	* Evaluates an expression, which is required to be in postfix notation
	* @return a token of the largest type. Can be void. Will return Token::invalid and set the error string on error
	*/
	Token evaluate(std::vector<Token>& tokens);

	std::string getError() const { return error; }

	/**@param outputStream   the stream to the output file. Used for functions such as print*/
	Evaluator(class Stream& outputStream, class CodePage& code);
	~Evaluator();

	/**Creates a new scope and sets it to the root of the scope stack (lowest)*/
	void newScope();
	/**
	* Deletes the lowest scope from the stack
	* Requires that pop is called only as many times as newScope. No more, no less
	*/
	void popScope();

	/**
	* Evaluates a single literal token
	* Requires the token be a literal
	* For usage of things such as: variable resolution, code execution, etc
	* @param t a non-function Token passed as a paramater to vector evaluate
	* @return a token result or invalid on error
	*/
	Token evalLit(Token& t);


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


	/**
	* Evaluates an operator expression
	* Requires t be in postfix order
	* @param t an array of arguments with the last being the operation
	* @return a token result or invalid on error
	*/
	Token evalOp(std::vector<Token>& t);

	/**
	* Evaluates a function expression
	* Requires t be in postfix order
	* @param t an array of arguments with the last being the operation
	* @return a token result or invalid on error
	*/
	Token evalFunc(std::vector<Token>& t);

	/**
	* Evaluates an keyword expression
	* Requires t be in postfix order
	* @param t an array of arguments with the last being the operation
	* @return a token result or invalid on error
	*/
	Token evalKeys(std::vector<Token>& t);

	/**
	* Resolves all tokens. Replaces variables with their value and executes stored code
	* Should be called prior to needing their value (a function call, operator etc)
	* Resulting value is stored in-place
	* @param t an array of tokens. Non literals will not be changed
	*/
	inline void resolveLiterals(std::vector<Token>& t) {
		for (Token& t : t)
			t = evalLit(t);
	}

	/**
	* Evaluates a control flow expression
	* @param t an array of arguments with the last being the control flow keyword
	* @return a token result or invalid
	*/
	Token evalControl(std::vector<Token>& t);




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
	MATH_OPERATOR(power);
	MATH_OPERATOR(test);
	MATH_OPERATOR(bool_and);
	MATH_OPERATOR(bool_or);
	MATH_OPERATOR(bit_and);
	MATH_OPERATOR(bit_or);
	MATH_OPERATOR(bit_xor);
	MATH_OPERATOR(mod);
	MATH_OPERATOR(less);
	MATH_OPERATOR(greater);
	MATH_OPERATOR(less_eq);
	MATH_OPERATOR(greater_eq);
	MATH_OPERATOR(shift_left);
	MATH_OPERATOR(shift_right);
	MATH_OPERATOR(not_test);
	MATH_OPERATOR(bool_xor);
};

