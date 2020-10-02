#include "CodePage.h"
#include "ParseTree.h"
#include "Tokens.h"
#include <unordered_map>
#include <string>
#include "Evaluator.h"
unsigned long CodePage::uid = 0;
struct CodePage::code {
	code* parent;
	std::unordered_map<long, std::vector<ParseTree>> data;
	code() : parent(nullptr) {};
};

CodePage::CodePage() : page(new code())
{
}

CodePage::~CodePage()
{
	if (page != nullptr) delete page;
}

Token CodePage::add(std::vector<ParseTree>&& pt)
{
	page->data.emplace(uid, std::forward<std::vector<ParseTree>>(pt));
	Token t = Tokens::lit_code;
	t.setData((long)(uid++));
	return t;
}

void CodePage::popTemp()
{
	if (page != nullptr) {
		code* old = page;
		page = page->parent;
		delete old;
	}
}

void CodePage::pushTemp()
{
	code* scope = new code();
	scope->parent = page;
	page = scope;
}
/*
Token CodePage::add(ParseTree&& pt, std::string& key)
{
	pimpl->globals.emplace(key, pt);
	Token t = Tokens::lit_code;
	t.setData(key);
	return t;
}*/

Token CodePage::eval(const Token& t, Evaluator& e)
{
	if (std::holds_alternative<long>(t.getData())) {
		code* p = page;
		while (p != nullptr) {
			if (p->data.find(t.getInt()) != p->data.end()) {
				e.newScope();
				auto& list = p->data[t.getInt()];
				for (ParseTree& p : list) {
					Token t = p.evaluate(e);
					if (t.getType() == Tokens::kw_return) {
						Token v = Tokens::lit_var;
						v.setData(t.getStr());
						Token r = e.evalLit(v);
						e.popScope();
						return r;
					}
				}
				e.popScope();
				return Tokens::sx_void;
			}
			p = page->parent;
		}
	}
	return Tokens::invalid;
}
