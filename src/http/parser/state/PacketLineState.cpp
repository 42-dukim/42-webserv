// PacketLineState.cpp
#include "PacketLineState.hpp"

#include <sstream>

#include "../HttpParser.hpp"

void PacketLineState::parse(HttpParser* parser) {
	std::string line = parser->readLine();
	std::string t1, t2, rest;
	std::istringstream iss(line);
	iss >> t1 >> t2;
	std::getline(iss, rest);
	if (!rest.empty() && rest[0] == ' ') rest.erase(0, 1);

	if (HTTP::Method::to_value(t1) != HTTP::Method::UNKNOWN_METHOD) {
		HTTP::StartLine startLine = {HTTP::Method::to_value(t1), t2, rest};
		parser->_packet = new HttpPacket(startLine, Header(), Body());
	} else {
		HTTP::StatusLine statusLine = {t1, HTTP::StatusCode::to_value(t2), rest};
		parser->_packet = new HttpPacket(statusLine, Header(), Body());
	}
	_done = true;
}

void PacketLineState::handleNextState(HttpParser* parser) {
	if (_done) parser->changeState(new HeaderState());
}
