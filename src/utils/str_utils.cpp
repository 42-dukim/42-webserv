#include "str_utils.hpp"

std::string int_tostr(int num) {
	std::stringstream s;

	s << num;
	return (s.str());
}

int str_toint(std::string str) {
	std::stringstream s;
	int				  num;

	s << str;
	s >> num;
	return (num);
}
