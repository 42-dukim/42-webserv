// HttpParser.cpp
#include "HttpParser.hpp"

#include <sstream>

#include "exception/HttpParserException.hpp"

HttpParser::HttpParser(const std::string& rawData) :
	_currentState(new PacketLineState()), _rawData(rawData), _pos(0), _packet(NULL) {}

HttpParser::~HttpParser() {
	delete _currentState;
	if (_packet) delete _packet;
}

void HttpParser::changeState(ParseState* newState) {
	if (_currentState) {
		delete _currentState;
	}
	_currentState = newState;
}

void HttpParser::parse() {
	while (true) {
		_currentState->parse(this);
		_currentState->handleNextState(this);
		if (dynamic_cast<DoneState*>(_currentState)) break;
	}

	// Done 이후 추가 바이트가 남아있으면(파이프라이닝 미지원) 오류 처리
	if (_pos != _rawData.size())
		throw HttpParserException("Unexpected extra bytes after HTTP message",
								  HTTP::StatusCode::BadRequest);
}

HttpPacket HttpParser::getResult() {
	return *_packet;
}

std::string HttpParser::readLine() {
	size_t next = _rawData.find("\r\n", _pos);
	if (next == std::string::npos)
		throw HttpParserException("Malformed request: Invalid line ending",
								  HTTP::StatusCode::BadRequest);
	std::string line = _rawData.substr(_pos, next - _pos);
	_pos = next + 2;

	return line;
}

std::string HttpParser::readBytes(size_t n) {
	if (_pos + n > _rawData.size())
		throw HttpParserException("Malformed request: Body incomplete",
								  HTTP::StatusCode::BadRequest);
	std::string chunk = _rawData.substr(_pos, n);
	_pos += n;

	return chunk;
}

size_t HttpParser::remaining() const {
	return _rawData.size() - _pos;
}
