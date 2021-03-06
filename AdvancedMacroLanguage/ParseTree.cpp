#include "ParseTree.h"
#include <vector>
#include <cassert>
#include "Evaluator.h"
struct ParseTree::node
{
	//Invariant: children.size() >= 2
	node* parent;
	Token data;
	bool isSubtree; //denotes that the tree is the root of a subtree and precedence should be ignores
	std::vector<node*> children;
	node() : parent(nullptr), data(Tokens::invalid), isSubtree(false) {
		children.resize(2, nullptr);
	}
	node(const Token& t) : node() {
		data = t;
	}
	~node() {
		for (node* n : children)
			if (n != nullptr) delete n;
	}
};

ParseTree::ParseTree() : root(new node()), next(root)
{
}

ParseTree::~ParseTree()
{
	if(root != nullptr) delete root;
}

ParseTree::ParseTree(ParseTree&& other)
{
	subtrees = std::move(other.subtrees);
	root = other.root;
	next = other.next;
	other.root = nullptr;
}

ParseTree& ParseTree::operator=(ParseTree&& other)
{
	subtrees = std::move(other.subtrees);
	if (root != nullptr) delete root;
	root = other.root;
	next = other.next;
	other.root = nullptr;
	return *this;
}

void ParseTree::addToken(const Token& t)
{
	switch (t.getCategory()) {
	case TokenCategory::literals:
	{
		next->data = t;
		moveUp(next);
		next = balanceNode(next);
		break;
	}
	case TokenCategory::syntax:
		switch (t.getType()) {
		case Tokens::start_expr:
			next->isSubtree = true;
			subtrees.push(next); //begins new subtree with next as the root
			break;
		case Tokens::end_expr:
			subtrees.pop();
			moveUp(next);
			break;
		case Tokens::sx_comma: //list, creates new child
			moveDown(next);
			break;
		}
		break;
	default: //expected: operators and functions
		if (next->data.getType() != Tokens::invalid) { //next is already defined
			auto newNode = new node(t);
			if (!subtrees.empty() && next == subtrees.top()) {
				newNode->isSubtree = true;
				next->isSubtree = false;
				subtrees.pop();
				subtrees.push(newNode);
			}
			newNode->children[0] = next;
			if (next->parent == nullptr) {
				root = newNode;
				newNode->parent = nullptr;
			}
			else {
				int pIndex = 0;
				for (; pIndex < next->parent->children.size(); ++pIndex)
					if (next->parent->children[pIndex] == next) break;
				newNode->parent = next->parent;
				next->parent->children[pIndex] = newNode;
			}
			next->parent = newNode;
			next = newNode;
			moveDown(newNode);
		}
		else {
			next->data = t;
			moveDown(next);
		}
		break;
	}
}

Token ParseTree::evaluate(Evaluator& e)
{
	if (!subtrees.empty()) throw evaluator_exception("Missing " + std::to_string(subtrees.size()) + " closing scope token(s). (')' or '}')");
	if (root == nullptr || root->data.getType() == Tokens::invalid) {
		if (root != nullptr && root->children[0] != nullptr && root->children[0]->data.getType() != Tokens::invalid)
			root = root->children[0];
		else
			throw evaluator_exception("Parse tree missing root");
	}
	root = balanceNode(root);
	root = balanceNode(root); //twice to check both sides
	return evaluate(root, e);
}

void ParseTree::moveUp(node* n)
{
	if (n->parent == nullptr) { //n is the root
		auto newRoot = new node();
		if (!subtrees.empty() && subtrees.top() == n) {
			newRoot->isSubtree = n->isSubtree;
			n->isSubtree = false; //always keep the isSubtree flag at the root of the subtree
			subtrees.pop();
			subtrees.push(newRoot);
		}
		newRoot->children[0] = root;
		root->parent = newRoot;
		root = newRoot;
		next = newRoot;
	}
	else if (n == getSubTreeRoot()) {
		auto newRoot = new node();
		newRoot->isSubtree = true;
		n->isSubtree = false; //always keep the isSubtree flag at the root of the subtree
		newRoot->parent = n->parent;
		int index = 0;
		for (; index < n->parent->children.size(); ++index)
			if (n->parent->children[index] == n) break;
		n->parent->children[index] = newRoot;
		newRoot->children[0] = n;
		n->parent = newRoot;
		subtrees.pop();
		subtrees.push(newRoot);
		next = newRoot;
	}
	else {
		next = n->parent;
	}
}

void ParseTree::moveDown(node* n, int childIndex)
{
	if (childIndex == -1) {
		for (childIndex = 0; childIndex < n->children.size() && n->children[childIndex] != nullptr
			&& n->children[childIndex]->data.getType() != Tokens::invalid; ++childIndex);
	}
	if (n->children.size() <= childIndex) n->children.resize(childIndex + 1);
	if (n->children[childIndex] == nullptr) {
		auto newNode = new node();
		newNode->parent = n;
		n->children[childIndex] = newNode;
		next = newNode;
	}
	else {
		next = n->children[childIndex];
	}
}

void ParseTree::rotateRight(node*  pivot)
{
	auto gparent = pivot->parent->parent;
	auto currentRight = pivot->children[1];
	pivot->children[1] = pivot->parent;
	pivot->children[1]->parent = pivot;
	pivot->children[1]->children[0] = currentRight;
	pivot->children[1]->children[0]->parent = pivot->children[1];
	if (gparent == nullptr) {		
		pivot->parent = nullptr;
		root = pivot;
	}
	else {
		int gIndex = 0;
		for (; gIndex < gparent->children.size(); ++gIndex)
			if (gparent->children[gIndex] == pivot->parent) break;
		pivot->parent = gparent;
		gparent->children[gIndex] = pivot;		
	}
}
void ParseTree::rotateLeft(node* pivot)
{
	auto gparent = pivot->parent->parent;
	auto currentLeft = pivot->children[0];
	pivot->children[0] = pivot->parent;
	pivot->children[0]->parent = pivot;
	pivot->children[0]->children[1] = currentLeft;
	pivot->children[0]->children[1]->parent = pivot->children[0];
	if (gparent == nullptr) {
		pivot->parent = nullptr;
		root = pivot;
	}
	else {
		int gIndex = 0;
		for (; gIndex < gparent->children.size(); ++gIndex)
			if (gparent->children[gIndex] == pivot->parent) break;
		pivot->parent = gparent;
		gparent->children[gIndex] = pivot;
	}
}
#ifdef _DEBUG
void ParseTree::inorder(node* n, std::function<void(const Token&)>& f) const
{
	if (n != nullptr) {
		inorder(n->children[0], f);
		f(n->data);
		inorder(n->children[1], f);
	}
}
ParseTree::node* ParseTree::getSubTreeRoot() const
{
	if (subtrees.empty()) return root;
	else return subtrees.top();
}
Token ParseTree::evaluate(node* n, Evaluator& e)
{
	if (n != nullptr) {
		std::vector<Token> expression;
//		if (n->isScope) e.newScope();
		for (node* nc : n->children)
			if (nc != nullptr && nc->data.getType() != Tokens::invalid) expression.push_back(evaluate(nc, e));
//		if (n->isScope) e.popScope();
		expression.push_back(n->data);
		Token&& ev = e.evaluate(expression);
		if (ev.getType() == Tokens::invalid) throw evaluator_exception(e.getError());
		return ev;
	}
	return Tokens::invalid;
}
ParseTree::node* ParseTree::balanceNode(node* next)
{
	if (next != nullptr && next->children[0] != nullptr && !next->children[0]->isSubtree && next->data.getCategory() != TokenCategory::literals && next->children[0]->data.getCategory() != TokenCategory::literals &&
		precedence(next->data.getType()) > precedence(next->children[0]->data.getType())) {

		node* newNext = next->children[0]; //child and parent is switching. Must switch the next ptr otherwise next will point to the child and the move up operation never happened
		rotateRight(next->children[0]);
		return newNext;
	}
	else if (next != nullptr && next->children[1] != nullptr && !next->children[1]->isSubtree && next->data.getCategory() != TokenCategory::literals && next->children[1]->data.getCategory() != TokenCategory::literals &&
		precedence(next->data.getType()) > precedence(next->children[1]->data.getType())) {

		node* newNext = next->children[1]; //child and parent is switching. Must switch the next ptr otherwise next will point to the child and the move up operation never happened
		rotateLeft(next->children[1]);
		return newNext;
	}
	return next;
}
void ParseTree::inorderTraversal(std::function<void(const Token&)> f) const
{
	inorder(root, f);
}
#endif