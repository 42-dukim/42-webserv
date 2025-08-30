#include "Body.hpp"

Body::Body() : _type(HTTP::ContentType::UNKNOWN_TYPE), _length(0) {}

Body::~Body() {}

Body::Body(const Body& copy) : _data(copy._data), _type(copy._type), _length(copy._length) {}

Body& Body::operator=(const Body& copy) {
	if (this != &copy) {
		this->_data = copy._data;
		this->_type = copy._type;
		this->_length = copy._length;
	}
	return (*this);
}

const std::vector<unsigned char>& Body::getData() const {
	return _data;
}

HTTP::ContentType::Value Body::getType() const {
	return _type;
}

size_t Body::getLength() const {
	return _length;
}

void Body::setType(HTTP::ContentType::Value type) {
	_type = type;
}

void Body::setLength(size_t length) {
	_length = length;
}

void Body::append(const char* data, size_t len) {
	_data.insert(_data.end(), reinterpret_cast<const unsigned char*>(data),
				 reinterpret_cast<const unsigned char*>(data) + len);
}
