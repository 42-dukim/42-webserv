#include "server/Server.hpp"

int main() {
	Config config(8080);
	Server server(config);
	server.runServer();
}