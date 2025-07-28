// DoneState.hpp
#ifndef DONESTATE_HPP
#define DONESTATE_HPP

#include <string>

#include "../HttpParser.hpp"
#include "ParseState.hpp"

class DoneState : public ParseState {
	public:
		virtual void parse(HttpParser*, const std::string&);
		virtual void handleNextState(HttpParser*);
};

#endif
