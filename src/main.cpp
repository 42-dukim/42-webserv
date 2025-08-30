#include "config/ConfigParser.hpp"
#include "config/model/Config.hpp"
#include "server/Server.hpp"

int main(int argc, char* argv[]) {
	try {
		ConfigParser parser;
		if (parser.validateArgument(argc)) parser.loadFromFile(argv[1]);
		std::vector<Config> configs = parser.getConfigs();
		Server server(configs);
		server.runServer();
	} catch (const std::exception& e) {
		std::cerr << e.what() << '\n';
	}

	return 0;
}
