// Server.cpp
#include "Server.hpp"

#include <arpa/inet.h>
#include <signal.h>
#include <sys/epoll.h>

#include <algorithm>
#include <cerrno>
#include <cstring>
#include <iostream>

#include "../handler/utils/response.hpp"
#include "../http/Enums.hpp"
#include "../http/parser/Parser.hpp"
#include "../http/parser/exception/NeedMoreInput.hpp"
#include "../http/parser/exception/ParserException.hpp"
#include "../http/serializer/Serializer.hpp"
#include "Defaults.hpp"
#include "epoll/exception/EpollException.hpp"
#include "exception/Exception.hpp"
#include "wrapper/SocketWrapper.hpp"

using namespace server;

Server::Server(const std::vector<config::Config>& configs) :
	_configs(configs), _clientSocket(-1), _socketOption(1), _addressSize(sizeof(_serverAddress)) {
	struct sigaction sa;

	sa.sa_handler = handler::cgi::ProcessManager::sigchldHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		throw Exception("sigaction failed");
	}
	_epollManager.init();
}

void Server::initAddress(int index) {
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_port = htons(_configs[index].getListen());
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	std::fill(_serverAddress.sin_zero, _serverAddress.sin_zero + 8, 0);
}

void Server::initServer() {
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
	} catch (const Exception& e) {
		std::cerr << e.what() << std::endl;
	} catch (const EpollException& e) {
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
		int fd = event.data.fd;

		if (_requestHandler.isCgiProcess(fd)) {
			int clientFd = _requestHandler.getClientFd(fd);
			_requestHandler.handleCgiEvent(fd, _epollManager);
			if (clientFd != -1 && _requestHandler.isCgiCompleted(clientFd)) {
				std::string cgiResponse = _requestHandler.getCgiResponse(clientFd);
				writeSocket(clientFd, cgiResponse);
				_requestHandler.removeCgiProcess(clientFd);
				_epollManager.remove(clientFd);
			}
			continue;
		}

		if (_serverSockets.find(event.data.fd) != _serverSockets.end()) {
			_clientSocket =
				socket::accept(event.data.fd, reinterpret_cast<sockaddr*>(&_clientAddress),
							   reinterpret_cast<socklen_t*>(&_addressSize));
			_epollManager.add(_clientSocket);
			continue;
		}

		int clientFd = event.data.fd;
		if (_requestHandler.isCgiProcessing(clientFd)) continue;

		std::string buffer = readSocket(clientFd);
		bool disconnected = (event.events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) != 0;
		http::Parser* parser = ensureParser(clientFd);

		if (!buffer.empty()) parser->append(buffer);
		if (disconnected) parser->markEndOfInput();
		if (buffer.empty() && !disconnected) continue;

		bool alreadyCleaned = false;
		while (true) {
			http::Parser::Result parseResult = parser->parse();

			switch (parseResult.status) {
				case http::Parser::Result::Incomplete:
					break;
				case http::Parser::Result::Error: {
					const std::string& body =
						parseResult.errorMessage.empty()
							? http::StatusCode::to_reasonPhrase(parseResult.errorCode)
							: parseResult.errorMessage;
					http::Packet errorPacket = handler::utils::makePlainResponse(
						parseResult.errorCode, body,
						http::ContentType::to_string(http::ContentType::CONTENT_TEXT_PLAIN));

					writePacket(clientFd, errorPacket);
					cleanupClient(clientFd);
					parser = NULL;
					alreadyCleaned = true;
					break;
				}
				case http::Parser::Result::Completed: {
					http::Packet httpRequest = parseResult.packet;
					std::string remainder = parseResult.leftover;
					bool ended = parseResult.endOfInput;

					int localPort = 0;
					sockaddr_in addr;
					socklen_t len = sizeof(addr);
					if (getsockname(clientFd, reinterpret_cast<sockaddr*>(&addr), &len) == 0) {
						localPort = ntohs(addr.sin_port);
					}

					router::RouteDecision decision =
						_requestHandler.route(httpRequest, _configs, localPort);
					if (decision.action == router::RouteDecision::Cgi) {
						_requestHandler.handleCgi(httpRequest, _configs, localPort, clientFd,
												  _epollManager);
						if (ended) cleanupClient(clientFd);
						alreadyCleaned = true;
						break;
					}

					http::Packet httpResponse =
						_requestHandler.handle(httpRequest, _configs, localPort);
					writePacket(clientFd, httpResponse);

					if (!remainder.empty()) {
						parser->append(remainder);
						if (ended) parser->markEndOfInput();
						continue;
					}
					if (ended) {
						cleanupClient(clientFd);
						alreadyCleaned = true;
					}
					break;
				}
			}
			break;
		}

		if (alreadyCleaned) continue;
		if (disconnected) cleanupClient(clientFd);
	}
}

std::string Server::readSocket(int socketFd) {
	char buffer[defaults::BUFFER_SIZE] = {0};
	std::string request;

	while (true) {
		ssize_t readSize = ::read(socketFd, buffer, defaults::BUFFER_SIZE);
		if (readSize > 0) {
			request.append(buffer, readSize);
			if (readSize < defaults::BUFFER_SIZE) break;
		} else if (readSize == 0) {
			break;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) break;
			return std::string();
		}
	}
	return request;
}

void Server::writePacket(int socketFd, const http::Packet& httpResponse) {
	std::string rawResponse = http::Serializer::serialize(httpResponse);
	writeSocket(socketFd, rawResponse);
}

void Server::writeSocket(int socketFd, const std::string& rawData) {
	::write(socketFd, rawData.c_str(), rawData.size());
}

void Server::cleanupClient(int fd) {
	std::map<int, http::Parser*>::iterator it = _parsers.find(fd);

	if (it != _parsers.end()) {
		delete it->second;
		_parsers.erase(it);
	}
	_requestHandler.removeCgiProcess(fd);
	_epollManager.remove(fd);
}

http::Parser* Server::ensureParser(int fd) {
	std::map<int, http::Parser*>::iterator it = _parsers.find(fd);

	if (it == _parsers.end()) {
		http::Parser* parser = new http::Parser();
		_parsers.insert(std::make_pair(fd, parser));
		return parser;
	}
	return it->second;
}

void Server::run() {
	for (size_t i = 0; i < _configs.size(); ++i) {
		initAddress(static_cast<int>(i));
		initServer();
	}
	loop();
}
// namespace server
