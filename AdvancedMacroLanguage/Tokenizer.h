#pragma once
#include "Stream.h"
#include "Tokens.h"
#include <string>
class Tokenizer
{
private:
	Stream& input;
	std::string errorToken;
public:
	Tokenizer(Stream& in) : input(in) {}

	/**
	* Gets the next token from the stream
	* Assumes that the stream is already starting at the first part of a token
	* Token can be preceded by whitespace
	*/
	Token getToken();
	static bool isOperator(char c);

	/**@return the string of characters that could not be indentified as a token*/
	inline std::string getInvalidToken() { return errorToken; }

	//Gets the string for the token
	const char* reverseLookup(Tokens t) const;
};

