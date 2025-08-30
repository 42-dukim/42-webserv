// Header.cpp
#include "Header.hpp"

#include "../../utils/str_utils.hpp"

// 빈 문자열을 돌려줄 때 사용할 정적 객체
static const std::string EMPTY_STRING = "";

Header::Header() {}

Header::~Header() {}

Header::Header(const Header& copy) : _headers(copy._headers) {}

Header& Header::operator=(const Header& copy) {
	if (this != &copy) {
		this->_headers = copy._headers;
	}
	return (*this);
}

const std::map<std::string, std::string>& Header::getHeaders() const {
	return _headers;
}

const std::string& Header::get(std::string key) const {
	std::map<std::string, std::string>::const_iterator it = _headers.find(to_lower(key));
	return (it != _headers.end()) ? it->second : EMPTY_STRING;
}

void Header::set(std::string key, std::string value) {
	_headers[to_lower(key)] = value;
}
