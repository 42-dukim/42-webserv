// Server.cpp
#include "Server.hpp"

#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>

#include <algorithm>
#include <iostream>

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

		std::map<int, ResponseSender>::iterator senderIt = _responseSenders.find(eventFd);
		if (senderIt != _responseSenders.end() && (event.events & EPOLLOUT)) {
			SendResult sendResult = senderIt->second.send();
			if (sendResult == SendResult::SUCCESS || sendResult == SendResult::ERROR) {
				_eventHandler.cleanup(eventFd, _epollManager);
				_epollManager.remove(eventFd);
				_responseSenders.erase(senderIt);
			}
			continue;  // ResponseSender가 있으면 다른 이벤트 처리 안 함
		}

		int localPort = 0;
		sockaddr_in addr;
		socklen_t len = sizeof(addr);
		if (getsockname(eventFd, reinterpret_cast<sockaddr*>(&addr), &len) == 0)
			localPort = ntohs(addr.sin_port);

		EventHandler::Result result =
			_eventHandler.handleEvent(eventFd, event.events, findConfig(localPort), _epollManager);

		if (result.response.fd != -1) {
			_responseSenders.emplace(result.response.fd,
									 ResponseSender(result.response.fd, result.response.data));
			_epollManager.modify(result.response.fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP);
		}

		if (result.closeFd != -1 &&
			(result.closeFd != result.response.fd || !result.response.closeAfterSend)) {
			_eventHandler.cleanup(result.closeFd, _epollManager);
			_epollManager.remove(result.closeFd);
			_responseSenders.erase(result.closeFd);
		}
	}
}

const config::Config* Server::findConfig(int localPort) const {
	std::map<int, config::Config>::const_iterator it = _configs.find(localPort);
	if (it != _configs.end()) return &it->second;
	return NULL;
}

void Server::run() {
	struct sigaction sa;

	sa.sa_handler = cgi::ProcessManager::sigchldHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) throw Exception("sigaction failed");
	_epollManager.init();

	for (std::map<int, config::Config>::iterator it = _configs.begin(); it != _configs.end();
		 ++it) {
		initServer(it->first);
	}
	loop();
}
