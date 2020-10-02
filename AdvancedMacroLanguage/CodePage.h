#pragma once
//Stores executable parse trees
//Similar to the evaluator but for parse trees instead of tokens
#include <memory>
#include <vector>
class CodePage
{
private:
	struct code;
	code* page;
	//Invariant: page is null iff CodePage was moved

	static unsigned long uid;
public:
	CodePage();
	~CodePage();
	/**
	* Adds a parse tree to storage
	* For code that cannot be referred to later
	* @return Token to be used to refer to the stored code
	*/
	class Token add(std::vector<class ParseTree>&& pt);

	/**
	* Clears toppmost temporary code storage
	* Should be called upon exiting a scope
	* Requires that pop be called the same amount of times as push
	*/
	void popTemp();

	/**Creates a new scope for temporary code storage*/
	void pushTemp();



	/**
	* Evaluates stored code
	* @return the resultant token of executing the code or Invalid on error
	* @throw evaluator exception if error occurs in evaluation
	*/
	class Token eval(const class Token& t, class Evaluator& e) throw(class evaluator_exception);
};

