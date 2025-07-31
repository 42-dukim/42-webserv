#include "ConfigParser.hpp"

bool ConfigParser::validateArgument(int ac) {
	if (ac == 2)
		return true;
	else
		throw ConfigException("[Error] Invalid Argument");
	return false;
}

std::string ConfigParser::readFromFile(char* filePath) {
	std::ifstream configFile(filePath);
	if (!configFile.is_open())
		throw ConfigException("[Error] File open failed: " +
							  std::string(filePath));
	std::ostringstream oss;
	oss << configFile.rdbuf();
	return oss.str();
}

std::vector<std::string> ConfigParser::tokenize(std::string& readFile) {
	std::string				 token;
	std::vector<std::string> tokens;

	for (unsigned long i = 0; i < readFile.size(); i++) {
		char c = readFile[i];
		if (std::isspace(c)) {
			if (!token.empty()) {
				tokens.push_back(token);
				token.clear();
			}
		} else if (c == ';' || c == '{' || c == '}') {
			if (!token.empty()) {
				tokens.push_back(token);
				token.clear();
			}
			tokens.push_back(std::string(1, c));
		} else if (c == '#') {
			while (i < readFile.size() && readFile[i] != '\n') i++;
		} else
			token += c;
	}
	if (!token.empty()) tokens.push_back(token);
	return tokens;
}

bool ConfigParser::expectToken(const std::vector<std::string>& tokens,
							   unsigned long				   i,
							   const std::string&			   expected) const {
	if (tokens.at(i) != expected)
		throw ConfigException("[emerg] Invalid configuration: expected '" +
							  expected + "'");
	return true;
}

void ConfigParser::parseListen(const std::vector<std::string>& tokens,
							   Config& Config, unsigned long& i) {
	char* end = NULL;
	long  port = std::strtol(tokens.at(i).c_str(), &end, 10);
	if (*end != 0)
		throw ConfigException("[emerg] Invalid configuration: Listen '" +
							  tokens.at(i) + "'");
	else if (port < 0 || 65535 < port)
		throw ConfigException("[emerg] Invalid configuration: port range");
	Config.setListen(port);
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseAutoIndex(const std::vector<std::string>& tokens,
								  Config& Config, unsigned long& i) {
	std::string status = tokens.at(i);
	if (status == "on")
		Config.setAutoIndex(true);
	else if (status == "off")
		Config.setAutoIndex(false);
	else
		throw ConfigException(
			"[emerg] Invalid configuration: autoindex value '" + status + "'");
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseServerName(const std::vector<std::string>& tokens,
								   Config& Config, unsigned long& i) {
	Config.setServerName(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseIndex(const std::vector<std::string>& tokens,
							  Config& Config, unsigned long& i) {
	Config.setIndex(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseRoot(const std::vector<std::string>& tokens,
							 Config& Config, unsigned long& i) {
	Config.setRoot(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseLocationRoot(const std::vector<std::string>& tokens,
									 Config& Config, const std::string& url,
									 unsigned long& i) {
	Config.setLocationRoot(url, tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseLocationIndex(const std::vector<std::string>& tokens,
									  Config& Config, const std::string& url,
									  unsigned long& i) {
	Config.setLocationIndex(url, tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseLocationAllowMethods(
	const std::vector<std::string>& tokens, Config& Config,
	const std::string& url, unsigned long& i) {
	std::vector<std::string> tmpMethods;
	while (tokens.at(i) != ";") {
		tmpMethods.push_back(tokens[i++]);
	}
	Config.setLocationAllowMethods(url, tmpMethods);
	expectToken(tokens, i, ";");
}

void ConfigParser::parseLocation(const std::vector<std::string>& tokens,
								 Config& Config, unsigned long& i) {
	std::string url = tokens.at(i);
	Config.initLocation(url);
	expectToken(tokens, ++i, "{");
	while (tokens.at(++i) != "}") {
		if (tokens.at(i) == "root")
			parseLocationRoot(tokens, Config, url, ++i);
		else if (tokens.at(i) == "index")
			parseLocationIndex(tokens, Config, url, ++i);
		else if (tokens.at(i) == "allow_methods")
			parseLocationAllowMethods(tokens, Config, url, ++i);
		else
			throw ConfigException(
				"[emerg] Invalid configuration: Unknown directive " +
				tokens.at(i));
	}
	expectToken(tokens, i, "}");
}

Config ConfigParser::parseServer(const std::vector<std::string>& tokens,
								 unsigned long&					 i) {
	Config Config;
	expectToken(tokens, i, "server");
	expectToken(tokens, ++i, "{");
	while (tokens.at(++i) != "}") {
		if (tokens.at(i) == "listen")
			parseListen(tokens, Config, ++i);
		else if (tokens.at(i) == "autoindex")
			parseAutoIndex(tokens, Config, ++i);
		else if (tokens.at(i) == "server_name")
			parseServerName(tokens, Config, ++i);
		else if (tokens.at(i) == "index")
			parseIndex(tokens, Config, ++i);
		else if (tokens.at(i) == "root")
			parseRoot(tokens, Config, ++i);
		else if (tokens.at(i) == "location")
			parseLocation(tokens, Config, ++i);
		else
			throw ConfigException(
				"[emerg] Invalid configuration: Unknown directive " +
				tokens.at(i));
	}
	expectToken(tokens, i, "}");
	return Config;
}

void ConfigParser::parse(const std::vector<std::string>& tokens) {
	try {
		unsigned long i = 0;
		expectToken(tokens, i, "http");
		expectToken(tokens, ++i, "{");
		while (tokens.at(++i) != "}") {
			_configs.push_back(parseServer(tokens, i));
		}
		expectToken(tokens, i, "}");
	} catch (const std::out_of_range& e) {
		throw ConfigException("[emerg] Invalid configuration");
	}
}

void ConfigParser::loadFromFile(char* filePath) {
	std::string readFile = readFromFile(filePath);
	parse(tokenize(readFile));
	for (unsigned long i = 0; i < _configs.size(); i++) {
		ConfigValidator::validate(_configs[i]);
	}
}

const std::vector<Config>& ConfigParser::getConfigs() { return _configs; }
