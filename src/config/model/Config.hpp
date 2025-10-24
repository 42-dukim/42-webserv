// Config.hpp
#ifndef CONFIG_MODEL_CONFIG_HPP
#define CONFIG_MODEL_CONFIG_HPP

#include <map>
#include <string>
#include <vector>

namespace config {
	struct LocationConfig {
			std::string _root;
			std::string _index;
			std::vector<std::string> _allow_methods;
	};

	class Config {
		private:
			bool _auto_index;
			int _listen;
			long long _client_max_body_size;
			std::string _server_name;
			std::string _index;
			std::string _root;
			std::map<std::string, LocationConfig> _location;
			std::map<int, std::string> _error_pages;

		public:
			Config();
			Config(const Config&);
			Config& operator=(const Config&);
			~Config() {}

			bool getAutoIndex() const;
			int getListen() const;
			long long getClientMaxBodySize() const;
			const std::string& getServerName() const;
			const std::string& getIndex() const;
			const std::string& getRoot() const;
			const std::map<int, std::string>& getErrorPages() const;
			const std::map<std::string, LocationConfig>& getLocation() const;
			const std::string& getLocationRoot(const std::string&) const;
			const std::string& getLocationIndex(const std::string&) const;
			const std::vector<std::string>& getLocationAllowMethods(const std::string&) const;

			void setAutoIndex(bool);
			void setListen(int);
			void setClientMaxBodySize(long long);
			void setServerName(const std::string&);
			void setIndex(const std::string&);
			void setRoot(const std::string&);
			void setErrorPage(int, const std::string&);
			void setLocationRoot(const std::string&, const std::string&);
			void setLocationIndex(const std::string&, const std::string&);
			void setLocationAllowMethods(const std::string&, const std::vector<std::string>&);

			void initLocation(const std::string&);
	};
}  // namespace config

#endif
