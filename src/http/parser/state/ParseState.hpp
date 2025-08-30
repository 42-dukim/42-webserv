// ParserState.hpp
#ifndef PARSESTATE_HPP
#define PARSESTATE_HPP

#include <string>

class HttpParser;

// 추상 상태 클래스
class ParseState {
	public:
		virtual ~ParseState() {};
		virtual void parse(HttpParser*) = 0;
		virtual void handleNextState(HttpParser*) = 0;
};

#endif
