// BodyState.hpp
#ifndef BODYSTATE_HPP
#define BODYSTATE_HPP

#include <string>

#include "../HttpParser.hpp"
#include "ParseState.hpp"

class BodyState : public ParseState {
	public:
		BodyState(size_t remain) : _done(false), _remain(remain) {}

		virtual void parse(HttpParser*);
		virtual void handleNextState(HttpParser*);

	private:
		bool _done;
		size_t _remain;
};

#endif
