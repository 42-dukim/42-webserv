// BodyState.cpp
#include "BodyState.hpp"

#include <algorithm>

void BodyState::parse(HttpParser* parser) {
	if (_remain == 0) {
		_done = true;
		return;
	}

	std::string chunk = parser->readBytes(_remain);
	parser->_packet->appendBody(chunk.data(), chunk.size());
	_remain = 0;
	_done = true;
}

void BodyState::handleNextState(HttpParser* parser) {
	if (_done) parser->changeState(new DoneState());
}
