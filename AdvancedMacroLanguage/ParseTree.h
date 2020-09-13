#pragma once
#include "CheapPtr.h"
#include "Tokens.h"
#ifdef _DEBUG
#include <functional>
#endif
#include <stack>
class evaluator_exception : public std::exception
{
private:
	const char* msg;
	std::string m;
public:
	evaluator_exception(const char* msg) : msg(msg) {}
	evaluator_exception(const std::string& s) {
		m = s;
		msg = m.c_str();
	}
	evaluator_exception(const std::string&& s) {
		m = s;
		msg = m.c_str();
	}
	const char* what() const override {
		return msg;
	}
};
//AST for the language
class ParseTree
{
	//Invariants: root and next != nullptr
private:
	struct node;
	node* root;
	node* next; //the node that will be set on the next addToken() call
	std::stack<node*> subtrees; //when inside an expression () or block {}
public:
	ParseTree();
	~ParseTree();

	/**
	* Adds a new token to the tree
	* Tree ordering is online
	*/
	void addToken(const Token& t);

	/**
	* Evaluates the tree from the root
	* @returns resultant Token of largest size
	* @throw evaluator exception
	*/
	Token evaluate(class Evaluator& e) throw(evaluator_exception);
#ifdef _DEBUG
	void inorderTraversal(std::function<void(const Token&)> f) const;
#endif
private:
	
	/**
	* Moves to the parent of n
	* If n is the root, creates a new root and makes n its left child
	* Bubbles up and preserves the isSubTree property
	*If n is the root of the subtree, follows the same process as if it were the root
	*/
	void moveUp(node* n);

	/**
	* Moves down the tree to the child on n
	* Creates a new node if necessary
	* @param childIndex    which child to move to. If childIndex >= children.size(), creates more children
	*/
	void moveDown(node* n, int childIndex);

	//requires pivot is left child of parent
	void rotateRight(node* pivot);
	//pivot is right child of parent
	void rotateLeft(node* pivot);
#ifdef _DEBUG
	void inorder(node* n, std::function<void(const Token&)>& f) const;
#endif

	/**@return  root of the deepest active subtreee. If no subtree is "open", returns the root*/
	node* getSubTreeRoot() const;

	/**
	* Performs a postorder depth-first transversal of the tree and at each node passes the evaluation of its children and its data in postfix order to the evaluator
	* @see Evaluator::evaluate
	* @throw evaluator_exception on error
	*/
	Token evaluate(node* n, class Evaluator& e) throw(evaluator_exception);

	/**
	* Performs a single left or right rotation as necessary to put the node in the correct spot
	* Will check node and its children, left than right. Only does one rotation so call again to ensure both children are ordered correctly
	* @param parent    the node to check. Will check this nodes children
	* @return the new root of the subtree or next if nothing changed
	*/
	node* balanceNode(node* parent);
};

