// HttpPacket.cpp
#include "HttpPacket.hpp"

HttpPacket::HttpPacket(const HTTP::StartLine& startLine, const Header& header, const Body& body) :
	_startLine(startLine), _header(header), _body(body), _isRequest(true) {}

HttpPacket::HttpPacket(const HTTP::StatusLine& statusLine, const Header& header, const Body& body) :
	_statusLine(statusLine), _header(header), _body(body), _isRequest(false) {}

const Header& HttpPacket::getHeader() const {
	return _header;
}

const Body& HttpPacket::getBody() const {
	return _body;
}

const HTTP::StartLine& HttpPacket::getStartLine() const {
	if (!_isRequest) throw std::logic_error("HttpPacket: not a request.");
	return _startLine;
}

const HTTP::StatusLine& HttpPacket::getStatusLine() const {
	if (_isRequest) throw std::logic_error("HttpPacket: not a response.");
	return _statusLine;
}

bool HttpPacket::isRequest() const {
	return _isRequest;
}

void HttpPacket::addHeader(const std::string& key, const std::string& value) {
	_header.set(key, value);
}

void HttpPacket::appendBody(const char* data, size_t len) {
	_body.append(data, len);
}

void HttpPacket::applyBodyLength(size_t len) {
	_body.setLength(len);
}

void HttpPacket::applyBodyType(HTTP::ContentType::Value type) {
	_body.setType(type);
}
