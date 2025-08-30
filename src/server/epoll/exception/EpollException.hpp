// EpollException.hpp
#ifndef EPOLLEXCEPTION_HPP
#define EPOLLEXCEPTION_HPP

#include <exception>
#include <string>

class EpollException : public std::exception {
	private:
		std::string _message;

	public:
		EpollException(const std::string& message) :
			_message("[Error] Epoll " + message + " Error") {}

		virtual const char* what() const throw() {
			return _message.c_str();
		}

		virtual ~EpollException() throw() {}
};

#endif
