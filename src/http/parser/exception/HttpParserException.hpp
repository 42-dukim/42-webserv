// HttpParserException.hpp
#ifndef HTTP_PARSE_EXCEPTION_HPP
#define HTTP_PARSE_EXCEPTION_HPP

#include <stdexcept>
#include <string>

#include "../../types/HttpEnums.hpp"

class HttpParserException : public std::exception {
	private:
		std::string _message;
		HTTP::StatusCode::Value _statusCode;

	public:
		HttpParserException(const std::string& message, HTTP::StatusCode::Value statusCode) :
			_message(message), _statusCode(statusCode) {}
		virtual ~HttpParserException() throw() {}

		virtual const char* what() const throw() {
			return _message.c_str();
		}

		HTTP::StatusCode::Value getStatusCode() const {
			return _statusCode;
		}
};

#endif
