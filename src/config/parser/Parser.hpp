// Parser.hpp
#ifndef CONFIG_PARSER_HPP
#define CONFIG_PARSER_HPP

#include <string>
#include <vector>

#include "../model/Config.hpp"

namespace config {
	class Parser {
		private:
			std::vector<Config> _configs;

			std::vector<std::string> tokenize(const std::string&);
			bool expectToken(const std::vector<std::string>&, unsigned long,
							 const std::string&) const;
			void parseAutoIndex(const std::vector<std::string>&, Config&, unsigned long&);
			void parseErrorPage(const std::vector<std::string>&, Config&, unsigned long&);
			void parseListen(const std::vector<std::string>&, Config&, unsigned long&);
			void parseClientMaxBodySize(const std::vector<std::string>&, Config&, unsigned long&);
			void parseServerName(const std::vector<std::string>&, Config&, unsigned long&);
			void parseIndex(const std::vector<std::string>&, Config&, unsigned long&);
			void parseRoot(const std::vector<std::string>&, Config&, unsigned long&);
			void parseLocationRoot(const std::vector<std::string>&, Config&, const std::string&,
								   unsigned long&);
			void parseLocationIndex(const std::vector<std::string>&, Config&, const std::string&,
									unsigned long&);
			void parseLocationAllowMethods(const std::vector<std::string>&, Config&,
										   const std::string&, unsigned long&);
			void parseLocation(const std::vector<std::string>&, Config&, unsigned long&);
			Config parseServer(const std::vector<std::string>&, unsigned long&);
			void parse(const std::vector<std::string>&);

		public:
			Parser() {}
			~Parser() {}

			bool validateArgument(int) const;
			void loadFromFile(const char*);
			const std::vector<Config>& getConfigs() const;
	};
}  // namespace config

#endif
