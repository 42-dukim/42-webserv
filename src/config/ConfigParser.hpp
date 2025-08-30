// ConfigParser.hpp
#ifndef CONFIGPARSER_HPP
#define CONFIGPARSER_HPP

#include <cstdlib>
#include <exception>
#include <fstream>
#include <sstream>
#include <string>

#include "ConfigValidator.hpp"
#include "exception/ConfigException.hpp"
#include "model/Config.hpp"
#include "types/ConfFile.hpp"

class ConfigParser {
	private:
		std::vector<Config> _configs;

		std::string readFromFile(const char*);
		std::vector<std::string> tokenize(const std::string&);
		bool expectToken(const std::vector<std::string>&, unsigned long, const std::string&) const;
		void parseAutoIndex(const std::vector<std::string>&, Config&, unsigned long&);
		void parseListen(const std::vector<std::string>&, Config&, unsigned long&);
		void parseClientMaxBodySize(const std::vector<std::string>&, Config&, unsigned long&);
		void parseServerName(const std::vector<std::string>&, Config&, unsigned long&);
		void parseIndex(const std::vector<std::string>&, Config&, unsigned long&);
		void parseRoot(const std::vector<std::string>&, Config&, unsigned long&);
		void parseLocationRoot(const std::vector<std::string>&, Config&, const std::string&,
							   unsigned long&);
		void parseLocationIndex(const std::vector<std::string>&, Config&, const std::string&,
								unsigned long&);
		void parseLocationAllowMethods(const std::vector<std::string>&, Config&, const std::string&,
									   unsigned long&);
		void parseLocation(const std::vector<std::string>&, Config&, unsigned long&);
		Config parseServer(const std::vector<std::string>&, unsigned long&);
		void parse(const std::vector<std::string>&);

	public:
		ConfigParser() {}
		~ConfigParser() {}

		bool validateArgument(int);
		void loadFromFile(const char*);
		const std::vector<Config>& getConfigs();
};

#endif
