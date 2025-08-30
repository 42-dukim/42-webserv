// str_utils.hpp
#ifndef STR_UTILS_HPP
#define STR_UTILS_HPP

#include <algorithm>
#include <cctype>
#include <sstream>

std::string int_tostr(int);
int str_toint(const std::string&);
std::string to_lower(const std::string&);

#endif
