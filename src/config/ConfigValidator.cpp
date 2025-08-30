#include "ConfigValidator.hpp"

void ConfigValidator::validate(const Config& config) {
	if (config.getListen() < 0) {
		throw ConfigException("nginx: [emerg] no \"listen\" directive in server block");
	}

	const std::map<std::string, ConfigLocation>& locations = config.getLocation();
	if (locations.empty()) {
		throw ConfigException("nginx: [emerg] no \"location\" block defined");
	}

	for (std::map<std::string, ConfigLocation>::const_iterator it = locations.begin();
		 it != locations.end(); ++it) {
		const std::string& path = it->first;
		const ConfigLocation& loc = it->second;

		if (loc._root.empty()) {
			throw ConfigException("nginx: [emerg] no \"root\" in location \"" + path + "\"");
		}
	}
}
