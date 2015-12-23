#ifndef __intbasic_token_h__
#define __intbasic_token_h__

#include <string>

struct token {
	std::string stringValue;
	int intValue = 0;

	token() = default;
	~token() = default;

	token(const token &) = default;
	token(token &&) = default;

	token(int i) : intValue(i)
	{}

	token(const std::string &s) : stringValue(s)
	{}

	token(std::string &&s) : stringValue(std::move(s))
	{}


	token &operator=(const token &) = default;
	token &operator=(token &&) = default;
};

#endif