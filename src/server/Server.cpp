#include "Server.hpp"

Server::Server(Config config)
	: _PORT(config.getListen()),
	  _socketOption(1),
	  _addressSize(sizeof(_serverAddress)) {
	initAddress();
}

void Server::initAddress() {
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_port = htons(_PORT);
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	std::fill(_serverAddress.sin_zero, _serverAddress.sin_zero + 8, 0);
}

int Server::validateFunction(int returnValue, std::string type) {
	if (returnValue < 0) throw ServerException(type);
	return returnValue;
}

void Server::initServer() {
	_serverSocket = validateFunction(
		socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0), "socket");
	validateFunction(setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR,
								&_socketOption, sizeof(_socketOption)),
					 "socket option");
	validateFunction(bind(_serverSocket, (struct sockaddr*) &_serverAddress,
						  sizeof(_serverAddress)),
					 "bind");
	validateFunction(listen(_serverSocket, 10), "listen");
	_epollManager.initEpoll(_serverSocket);
}

void Server::loopServer() {
	try {
		while (1) {
			_epollManager.waitEvent();
			handleEvents();
		}
	} catch (const ServerException& e) {
		std::cout << e.what() << std::endl;
	} catch (const EpollException& e) {
		std::cout << e.what() << std::endl;
	} catch (...) {
		std::cout << "[Error] Unknown Error" << std::endl;
	}
}

void Server::handleEvents() {
	for (int i = 0; i < _epollManager.getEventCount(); i++) {
		if (_epollManager.getEpollEventsAt(i).data.fd == _serverSocket) {
			_clientSocket = validateFunction(
				validateFunction(
					accept(_serverSocket, (struct sockaddr*) &_clientAddress,
						   (socklen_t*) &_addressSize),
					"accept"),
				"accept");
			std::cout << "new client :" << _clientSocket << std::endl;
			_epollManager.addEpollFd(_clientSocket);
		} else {
			HttpRequest httpRequest =
				readHttpPacket(_epollManager.getEpollEventsAt(i).data.fd);
			if (_epollManager.getEpollEventsAt(i).events &
				(EPOLLRDHUP | EPOLLHUP | EPOLLERR))
				_epollManager.deleteEpollFd(
					_epollManager.getEpollEventsAt(i).data.fd);
			else {
				HttpResponse httpResponse("test", "test", 200);
				writeHttpPacket(_epollManager.getEpollEventsAt(i).data.fd,
								httpResponse);
			}
		}
	}
}

std::string Server::readSocket(int socketFd) {
	char		buffer[SystemConfig::Size::BUFFER_SIZE] = {0};
	int			readSize;
	std::string request = "";

	while (
		(readSize = read(socketFd, buffer, SystemConfig::Size::BUFFER_SIZE)) ==
		SystemConfig::Size::BUFFER_SIZE) {
		request.append(buffer);
	}
	request.append(buffer);
	return request;
}

void Server::writeHttpPacket(int socketFd, HttpResponse httpResponse) {
	writeSocket(socketFd, httpResponse.getRawData());
}

HttpRequest Server::readHttpPacket(int socketFd) {
	HttpRequest httpRequest =
		HttpParser::parseRequestHttp(readSocket(socketFd));
	return httpRequest;
}

void Server::writeSocket(int socketFd, std::string rawData) {
	write(socketFd, rawData.c_str(), rawData.size());
}

void Server::runServer() {
	initServer();
	loopServer();
}