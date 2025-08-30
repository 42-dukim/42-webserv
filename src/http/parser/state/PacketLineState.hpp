// PacketLineState.hpp
#ifndef PACKETLINESTATE_HPP
#define PACKETLINESTATE_HPP

#include <string>

#include "ParseState.hpp"

class PacketLineState : public ParseState {
	public:
		PacketLineState() : _done(false) {}

		virtual void parse(HttpParser*);
		virtual void handleNextState(HttpParser*);

	private:
		bool _done;
};

#endif
