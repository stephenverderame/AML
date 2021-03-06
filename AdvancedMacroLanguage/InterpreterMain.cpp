#include "Stream.h"
#include <string>
#include "Tokenizer.h"
#include "ParseTree.h"
#include "Evaluator.h"
#include "CodePage.h"
/**
* Adds a new tree to the code page. Called when a { is detected in the input stream
* Returns when a } occurs
* @return token referring to the parse tree
*/
Token innerScope(Tokenizer& tokenizer, CodePage& code);
int main(int argc, char ** args) {
	/*Interpreter arguments:
		in: the file to read from
		out: the file to write to
	*/
	const char* input = nullptr, * output = nullptr;
	constexpr char symbol = '#';
	for (int i = 0; i < argc; ++i) {
		const char* id;
		if ((id = strstr(args[i], "in:")) != NULL) {
			input = id + 3;
		}
		else if ((id = strstr(args[i], "out:")) != NULL) {
			output = id + 4;
		}
	}
	Stream strIn = make_stream(input, streamMode::input);
	Stream strOut = make_stream(output, streamMode::output);
	Tokenizer tokenizer(strIn);
	char lastC = 0;
	char c;
	int lineCount = 0;
	CodePage cp;
	Evaluator global(strOut, cp);
	while ((c = fgetc(strIn)) != EOF) { //the interpreter loop
		switch (c) {
		case symbol:
		{
			char c2 = fgetc(strIn);
			if (c2 == symbol) {
				Token t;
				int brackets = 0;
				ParseTree pt;
				do {
					t = tokenizer.getToken();
					if (t.getType() == Tokens::start_block) t = innerScope(tokenizer, cp);
					pt.addToken(t);
				} while (t.getType() != Tokens::invalid && (t.getType() != Tokens::end_stment || brackets > 0));
				if (t.getType() == Tokens::invalid) {
					fprintf(stderr, "\n\033[1;31mInvalid token: '%s' at line: %d\n\033[1;0m", tokenizer.getInvalidToken().c_str(), lineCount);
					if (tokenizer.getInvalidToken().size() >= max_token_length)
						fprintf(stderr, "\033[1;31mMaximum token length is %d characters\n\033[1;0m", max_token_length);
				}
				else {
					try {
						Token res = pt.evaluate(global);
/*						if (res.getType() != Tokens::sx_void)
							fputs(res.literalValue().c_str(), strOut);*/
					}
					catch (evaluator_exception& e) {
						fprintf(stderr, "\033[1;31mEvaluator exception: '%s' at line: %d\n\033[1;0m", e.what(), lineCount);
					}
//					while ((c = fgetc(strIn)) == '\n' || c == '\r' || c == '\t');
//					fputc(c, strOut);
#ifdef _DEBUG
					printf("\n\n\n");
					pt.inorderTraversal([&tokenizer](const Token& t) {
						if (t.getCategory() == TokenCategory::literals)
							printf("%s ", t.literalValue().c_str());
						else
							printf("%s ", tokenizer.reverseLookup(t.getType()));
						});
					printf("\n");
#endif
				}
			}
			else {
				fputc(c, strOut);
				fputc(c2, strOut);
			}
			break;
		}
		case '\n':
			fputc(c, strOut);
			++lineCount;
			break;
		default:
			fputc(c, strOut);
		}
	}
	return 0;
}

Token innerScope(Tokenizer& tokenizer, CodePage& code)
{
	Token t;
	std::vector<ParseTree> treeList;
	while (t.getType() != Tokens::end_block) {
		ParseTree pt;
		do {
			t = tokenizer.getToken();
			if (t.getType() == Tokens::start_block) t = innerScope(tokenizer, code);
			if (t.getType() == Tokens::end_block) goto dblBreak;
			pt.addToken(t);
		} while (t.getType() != Tokens::end_stment && t.getType() != Tokens::invalid && t.getType() != Tokens::end_block);
		treeList.push_back(std::move(pt));
	}
	dblBreak:
	return code.add(std::move(treeList));
}
