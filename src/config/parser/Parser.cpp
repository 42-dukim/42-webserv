// Parser.cpp
#include "Parser.hpp"

#include <cctype>
#include <stdexcept>

#include "../../utils/file_utils.hpp"
#include "../../utils/str_utils.hpp"
#include "../Defaults.hpp"
#include "../exception/Exception.hpp"
#include "../validator/Validator.hpp"

using namespace config;

bool Parser::validateArgument(int ac) const {
	if (ac > 2) throw Exception("[Error] Invalid Argument");
	return true;
}

std::vector<std::string> Parser::tokenize(const std::string& readFile) {
	std::string token;
	std::vector<std::string> tokens;

	for (unsigned long i = 0; i < readFile.size(); i++) {
		char c = readFile[i];
		if (std::isspace(static_cast<unsigned char>(c))) {
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
		} else {
			token += c;
		}
	}
	if (!token.empty()) tokens.push_back(token);
	return tokens;
}

bool Parser::expectToken(const std::vector<std::string>& tokens, unsigned long i,
						 const std::string& expected) const {
	if (tokens.at(i) != expected) {
		throw Exception("[emerg] Invalid configuration: expected '" + expected + "'");
	}
	return true;
}

void Parser::parseListen(const std::vector<std::string>& tokens, Config& config, unsigned long& i) {
	char* end = NULL;
	long port = std::strtol(tokens.at(i).c_str(), &end, 10);
	if (*end != 0)
		throw Exception("[emerg] Invalid configuration: listen '" + tokens.at(i) + "'");
	else if (port < 0 || 65535 < port)
		throw Exception("[emerg] Invalid configuration: port range");
	config.setListen(port);
	expectToken(tokens, ++i, ";");
}

void Parser::parseClientMaxBodySize(const std::vector<std::string>& tokens, Config& config,
									unsigned long& i) {
	char* end = NULL;
	long long size = std::strtoll(tokens.at(i).c_str(), &end, 10);

	std::string unit(end);
	for (unsigned long j = 0; j < unit.size(); j++)
		unit[j] = static_cast<char>(std::tolower(unit[j]));
	if (unit == "k" || unit == "kb") {
		size *= 1024;
	} else if (unit == "m" || unit == "mb") {
		size *= 1024 * 1024;
	} else if (unit == "g" || unit == "gb") {
		size *= 1024 * 1024 * 1024;
	} else if (!unit.empty()) {
		throw Exception("[emerg] Invalid configuration: client_max_body_size '" + tokens.at(i) +
						"'");
	}
	if (size < 0 || defaults::LIMIT_CLIENT_MAX_BODY_SIZE < size)
		throw Exception("[emerg] Invalid configuration: client_max_body_size '" + tokens.at(i) +
						"'");
	config.setClientMaxBodySize(size);
	expectToken(tokens, ++i, ";");
}

void Parser::parseAutoIndex(const std::vector<std::string>& tokens, Config& config,
							unsigned long& i) {
	std::string status = tokens.at(i);
	if (status == "on")
		config.setAutoIndex(true);
	else if (status == "off")
		config.setAutoIndex(false);
	else
		throw Exception("[emerg] Invalid configuration: autoindex value '" + status + "'");
	expectToken(tokens, ++i, ";");
}

void Parser::parseServerName(const std::vector<std::string>& tokens, Config& config,
							 unsigned long& i) {
	config.setServerName(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void Parser::parseIndex(const std::vector<std::string>& tokens, Config& config, unsigned long& i) {
	config.setIndex(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void Parser::parseRoot(const std::vector<std::string>& tokens, Config& config, unsigned long& i) {
	config.setRoot(tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void Parser::parseErrorPage(const std::vector<std::string>& tokens, Config& config,
							unsigned long& i) {
	char* end = NULL;
	std::vector<long> statusCodes;
	long statusCode = std::strtol(tokens.at(i++).c_str(), &end, 10);
	if (*end != 0)
		throw Exception("[emerg] Invalid configuration: error_page '" + tokens.at(i) + "'");
	statusCodes.push_back(statusCode);
	for (; i < tokens.size(); ++i) {
		statusCode = std::strtol(tokens.at(i).c_str(), &end, 10);
		if (*end != 0) break;
		statusCodes.push_back(statusCode);
	}
	for (size_t j = 0; j < statusCodes.size(); ++j) {
		config.setErrorPage(static_cast<int>(statusCodes[j]), tokens.at(i));
	}
	expectToken(tokens, ++i, ";");
}

void Parser::parseLocationRoot(const std::vector<std::string>& tokens, Config& config,
							   const std::string& url, unsigned long& i) {
	config.setLocationRoot(url, tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void Parser::parseLocationIndex(const std::vector<std::string>& tokens, Config& config,
								const std::string& url, unsigned long& i) {
	config.setLocationIndex(url, tokens.at(i));
	expectToken(tokens, ++i, ";");
}

void Parser::parseLocationAllowMethods(const std::vector<std::string>& tokens, Config& config,
									   const std::string& url, unsigned long& i) {
	std::vector<std::string> tmpMethods;
	while (tokens.at(i) != ";") {
		tmpMethods.push_back(tokens[i++]);
	}
	config.setLocationAllowMethods(url, tmpMethods);
	expectToken(tokens, i, ";");
}

void Parser::parseLocation(const std::vector<std::string>& tokens, Config& config,
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
			throw Exception("[emerg] Invalid configuration: Unknown directive " + tokens.at(i));
	}
	expectToken(tokens, i, "}");
}

Config Parser::parseServer(const std::vector<std::string>& tokens, unsigned long& i) {
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
		else if (tokens.at(i) == "error_page")
			parseErrorPage(tokens, config, ++i);
		else if (tokens.at(i) == "location")
			parseLocation(tokens, config, ++i);
		else
			throw Exception("[emerg] Invalid configuration: Unknown directive " + tokens.at(i));
	}
	expectToken(tokens, i, "}");
	return config;
}

void Parser::parse(const std::vector<std::string>& tokens) {
	try {
		unsigned long i = 0;
		expectToken(tokens, i, "http");
		expectToken(tokens, ++i, "{");
		while (tokens.at(++i) != "}") {
			_configs.push_back(parseServer(tokens, i));
		}
		expectToken(tokens, i, "}");
	} catch (const std::out_of_range&) {
		throw Exception("[emerg] Invalid configuration");
	}
}

void Parser::loadFromFile(const char* filePath) {
	const FileInfo fileInfo = readFile(filePath ? filePath : defaults::PATH());
	if (fileInfo.error) throw Exception("[Error] confFile open failed");
	parse(tokenize(fileInfo.content));
	Validator::validate(_configs);
}

const std::vector<Config>& Parser::getConfigs() const {
	return _configs;
}
