#include "ConfigParser.hpp"

bool ConfigParser::validateArgument(int ac) {
	if (ac > 2) throw ConfigException("[Error] Invalid Argument");
	return true;
}

std::vector<std::string> ConfigParser::tokenize(const std::string& readFile) {
	std::string token;
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

bool ConfigParser::expectToken(const std::vector<std::string>& tokens, unsigned long i,
							   const std::string& expected) const {
	if (tokens.at(i) != expected)
		throw ConfigException("[emerg] Invalid configuration: expected '" + expected + "'");
	return true;
}

void ConfigParser::parseListen(const std::vector<std::string>& tokens, Config& config,
							   unsigned long& i) {
	char* end = NULL;
	long port = std::strtol(tokens.at(i).c_str(), &end, 10);
	if (*end != 0)
		throw ConfigException("[emerg] Invalid configuration: listen '" + tokens.at(i) + "'");
	else if (port < 0 || 65535 < port)
		throw ConfigException("[emerg] Invalid configuration: port range");
	config.setListen(port);
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseClientMaxBodySize(const std::vector<std::string>& tokens, Config& config,
										  unsigned long& i) {
	char* end = NULL;
	long long size = std::strtoll(tokens.at(i).c_str(), &end, 10);

	std::string unit(end);
	for (unsigned long i = 0; i < unit.size(); i++) unit[i] = tolower(unit[i]);
	if (unit == "k" || unit == "kb") {
		size *= 1024;
	} else if (unit == "m" || unit == "mb") {
		size *= 1024 * 1024;
	} else if (unit == "g" || unit == "gb") {
		size *= 1024 * 1024 * 1024;
	} else if (!unit.empty()) {
		throw ConfigException("[emerg] Invalid configuration: client_max_body_size '" +
							  tokens.at(i) + "'");
	}
	if (size < 0 || ConfFile::DEFAULT::LIMIT_CLIENT_MAX_BODY_SIZE < size)
		throw ConfigException("[emerg] Invalid configuration: client_max_body_size '" +
							  tokens.at(i) + "'");
	config.setClientMaxBodySize(size);
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseAutoIndex(const std::vector<std::string>& tokens, Config& config,
								  unsigned long& i) {
	std::string status = tokens.at(i);
	if (status == "on")
		config.setAutoIndex(true);
	else if (status == "off")
		config.setAutoIndex(false);
	else
		throw ConfigException("[emerg] Invalid configuration: autoindex value '" + status + "'");
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseServerName(const std::vector<std::string>& tokens, Config& config,
								   unsigned long& i) {
	config.setServerName(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseIndex(const std::vector<std::string>& tokens, Config& config,
							  unsigned long& i) {
	config.setIndex(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseRoot(const std::vector<std::string>& tokens, Config& config,
							 unsigned long& i) {
	config.setRoot(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseLocationRoot(const std::vector<std::string>& tokens, Config& config,
									 const std::string& url, unsigned long& i) {
	config.setLocationRoot(url, tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseLocationIndex(const std::vector<std::string>& tokens, Config& config,
									  const std::string& url, unsigned long& i) {
	config.setLocationIndex(url, tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void ConfigParser::parseLocationAllowMethods(const std::vector<std::string>& tokens, Config& config,
											 const std::string& url, unsigned long& i) {
	std::vector<std::string> tmpMethods;
	while (tokens.at(i) != ";") {
		tmpMethods.push_back(tokens[i++]);
	}
	config.setLocationAllowMethods(url, tmpMethods);
	expectToken(tokens, i, ";");
}

void ConfigParser::parseLocation(const std::vector<std::string>& tokens, Config& config,
								 unsigned long& i) {
	std::string url = tokens.at(i);
	config.initLocation(url);
	expectToken(tokens, ++i, "{");
	while (tokens.at(++i) != "}") {
		if (tokens.at(i) == "root")
			parseLocationRoot(tokens, config, url, ++i);
		else if (tokens.at(i) == "index")
			parseLocationIndex(tokens, config, url, ++i);
		else if (tokens.at(i) == "allow_methods")
			parseLocationAllowMethods(tokens, config, url, ++i);
		else
			throw ConfigException("[emerg] Invalid configuration: Unknown directive " +
								  tokens.at(i));
	}
	expectToken(tokens, i, "}");
}

Config ConfigParser::parseServer(const std::vector<std::string>& tokens, unsigned long& i) {
	Config config;
	expectToken(tokens, i, "server");
	expectToken(tokens, ++i, "{");
	while (tokens.at(++i) != "}") {
		if (tokens.at(i) == "listen")
			parseListen(tokens, config, ++i);
		else if (tokens.at(i) == "client_max_body_size")
			parseClientMaxBodySize(tokens, config, ++i);
		else if (tokens.at(i) == "autoindex")
			parseAutoIndex(tokens, config, ++i);
		else if (tokens.at(i) == "server_name")
			parseServerName(tokens, config, ++i);
		else if (tokens.at(i) == "index")
			parseIndex(tokens, config, ++i);
		else if (tokens.at(i) == "root")
			parseRoot(tokens, config, ++i);
		else if (tokens.at(i) == "location")
			parseLocation(tokens, config, ++i);
		else
			throw ConfigException("[emerg] Invalid configuration: Unknown directive " +
								  tokens.at(i));
	}
	expectToken(tokens, i, "}");
	return config;
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

void ConfigParser::loadFromFile(const char* filePath) {
	const FileInfo fileInfo = FileReader::readFile(filePath ? filePath : ConfFile::DEFAULT::PATH());
	if (fileInfo.error)
		throw ConfigException("[Error] File open failed: " + std::string(filePath));
	parse(tokenize(fileInfo.content));
	for (unsigned long i = 0; i < _configs.size(); i++) {
		ConfigValidator::validate(_configs[i]);
	}
}

const std::vector<Config>& ConfigParser::getConfigs() {
	return _configs;
}
