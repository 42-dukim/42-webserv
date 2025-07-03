#ifndef CONFIGPARSER_HPP
#define CONFIGPASER_HPP

#include <string>

#include "Config.hpp"

class ConfigParser {
	public:
		Config parseConfig(std::string configPath);
};

#endif