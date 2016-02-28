#ifndef __lisp_parser_h__
#define __lisp_parser_h__

#include "lemon_base.h"
#include <cstdio>
#include <memory>
#include <string>

struct Token {

	Token() = default;
	Token(const Token &) = default;
	Token(Token &&) = default;

	Token(int i) : intValue(i)
	{}
	Token(const std::string &s) : stringValue(s)
	{}
	Token(std::string &&s) : stringValue(std::move(s))
	{}


	Token& operator=(const Token &) = default;
	Token& operator=(Token &&) = default;

	int intValue = 0;
	std::string stringValue;
};

class mexpr_parser : public lemon_base<Token>{
	public:
	static std::unique_ptr<mexpr_parser> create();

	using lemon_base::parse;
	void parse(int major) { parse(major, Token{}); }

	template<class T>
	void parse(int major, T &&t) { parse(major, Token(std::forward<T>(t))); }

	protected:
	virtual void parse_failure() final override {
		fail = true;
		//printf("Fail!\n");
	}
	virtual void parse_accept() final override {
		//printf("Accept!\n");
	}

	virtual void syntax_error(int yymajor, token_type &yyminor) final override {
		printf("Syntax Error!\n");
		error++;
	}

private:
	bool fail = false;
	unsigned error = 0;

};




#endif