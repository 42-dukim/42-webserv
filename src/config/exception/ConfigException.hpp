// ConfigException.hpp
#ifndef CONFIG_EXCEPTION_HPP
#define CONFIG_EXCEPTION_HPP

#include <exception>
#include <iostream>

class ConfigException : public std::exception {
	private:
		std::string _exception;

	public:
		ConfigException(std::string e) {
			_exception = e;
		}
		~ConfigException() throw() {}
		virtual const char* what() const throw() {
			return _exception.c_str();
		}
};

#endif
