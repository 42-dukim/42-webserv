// Server.cpp
#include "Server.hpp"

#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>

#include <algorithm>
#include <iostream>

#include "../http/serializer/Serializer.hpp"
#include "epoll/exception/EpollException.hpp"
#include "exception/Exception.hpp"
#include "wrapper/SocketWrapper.hpp"

using namespace server;
using namespace handler;

Server::Server(const std::map<int, config::Config>& configs) :
	_configs(configs), _clientSocket(-1), _socketOption(1), _addressSize(sizeof(_serverAddress)) {}

void Server::initServer(int port) {
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_port = htons(port);
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	std::fill(_serverAddress.sin_zero, _serverAddress.sin_zero + 8, 0);

	int serverSocket = socket::create(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	socket::setOption(serverSocket, SOL_SOCKET, SO_REUSEADDR, &_socketOption,
					  sizeof(_socketOption));
	socket::bind(serverSocket, reinterpret_cast<sockaddr*>(&_serverAddress),
				 sizeof(_serverAddress));
	socket::listen(serverSocket, 10);
	_serverSockets.insert(serverSocket);
	_epollManager.add(serverSocket);
}

void Server::loop() {
	try {
		while (true) {
			_epollManager.wait();
			handleEvents();
		}
	} catch (const server::Exception& e) {
		std::cerr << e.what() << std::endl;
	} catch (const server::EpollException& e) {
		std::cerr << e.what() << std::endl;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	} catch (...) {
		std::cerr << "[Error] Unknown Error" << std::endl;
	}
}

void Server::handleEvents() {
	for (int i = 0; i < _epollManager.eventCount(); i++) {
		const epoll_event& event = _epollManager.eventAt(i);
		int eventFd = event.data.fd;

		if (_serverSockets.find(eventFd) != _serverSockets.end()) {
			_clientSocket = socket::accept(eventFd, reinterpret_cast<sockaddr*>(&_clientAddress),
										   reinterpret_cast<socklen_t*>(&_addressSize));
			_epollManager.add(_clientSocket);
			continue;
		}

		int localPort = 0;
		sockaddr_in addr;
		socklen_t len = sizeof(addr);
		if (getsockname(eventFd, reinterpret_cast<sockaddr*>(&addr), &len) == 0)
			localPort = ntohs(addr.sin_port);

		EventHandler::Result result =
			_eventHandler.handleEvent(eventFd, event.events, findConfig(localPort), _epollManager);

		if (result.response.fd != -1) {
			sendResponse(result.response.fd, result.response.data);
			if (result.response.closeAfterSend) {
				_eventHandler.cleanup(result.response.fd);
				_epollManager.remove(result.response.fd);
			}
		}

		if (result.closeFd != -1 &&
			(result.closeFd != result.response.fd || !result.response.closeAfterSend)) {
			_eventHandler.cleanup(result.closeFd);
			_epollManager.remove(result.closeFd);
		}
	}
}

const config::Config* Server::findConfig(int localPort) const {
	std::map<int, config::Config>::const_iterator it = _configs.find(localPort);
	if (it != _configs.end()) return &it->second;
	return NULL;
}

void Server::sendResponse(int socketFd, const http::Packet& httpResponse) {
	std::string rawResponse = http::Serializer::serialize(httpResponse);
	::write(socketFd, rawResponse.c_str(), rawResponse.size());
}

void Server::sendResponse(int socketFd, const std::string& rawResponse) {
	::write(socketFd, rawResponse.c_str(), rawResponse.size());
}

void Server::run() {
	struct sigaction sa;

	sa.sa_handler = cgi::ProcessManager::sigchldHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) throw Exception("sigaction failed");
	_epollManager.init();

	std::map<int, config::Config>::iterator it;
	for (it = _configs.begin(); it != _configs.end(); ++it) {
		initServer(it->first);
	}
	loop();
}
// namespace server
