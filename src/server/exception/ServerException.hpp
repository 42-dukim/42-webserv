// ServerException.hpp
#ifndef SERVEREXCEPTION_HPP
#define SERVEREXCEPTION_HPP

#include <exception>
#include <string>

class ServerException : public std::exception {
	private:
		std::string _message;

	public:
		ServerException(const std::string& message) :
			_message("[Error] Server " + message + " Error") {}

		virtual const char* what() const throw() {
			return _message.c_str();
		}

		virtual ~ServerException() throw() {}
};

#endif
