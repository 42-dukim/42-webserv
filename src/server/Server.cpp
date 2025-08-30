#include "Server.hpp"

Server::Server(std::vector<Config> configs) :
	_configs(configs), _socketOption(1), _addressSize(sizeof(_serverAddress)) {
	_epollManager.initEpoll();
}

void Server::initAddress(int index) {
	_serverAddress.sin_family = AF_INET;
	_serverAddress.sin_port = htons(_configs[index].getListen());
	_serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
	std::fill(_serverAddress.sin_zero, _serverAddress.sin_zero + 8, 0);
}

void Server::initServer() {
	int serverSocket = SocketWrapper::socket(PF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
	SocketWrapper::setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &_socketOption,
							  sizeof(_socketOption));
	SocketWrapper::bind(serverSocket, (struct sockaddr*) &_serverAddress, sizeof(_serverAddress));
	SocketWrapper::listen(serverSocket, 10);
	_serverSockets.insert(serverSocket);
	_epollManager.addEpollFd(serverSocket);
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
	} catch (const std::exception& e) {
		std::cout << e.what() << std::endl;
	} catch (...) {
		std::cout << "[Error] Unknown Error" << std::endl;
	}
}

void Server::handleEvents() {
	for (int i = 0; i < _epollManager.getEventCount(); i++) {
		if (_serverSockets.find(_epollManager.getEpollEventsAt(i).data.fd) !=
			_serverSockets.end()) {
			_clientSocket = SocketWrapper::accept(*_serverSockets.find(
													  _epollManager.getEpollEventsAt(i).data.fd),
												  (struct sockaddr*) &_clientAddress,
												  (socklen_t*) &_addressSize);
			std::cout << "new client :" << _clientSocket << std::endl;
			_epollManager.addEpollFd(_clientSocket);
		} else {
			std::string buffer = readSocket(_epollManager.getEpollEventsAt(i).data.fd);
			if (!buffer.size() ||
				_epollManager.getEpollEventsAt(i).events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
				_epollManager.deleteEpollFd(_epollManager.getEpollEventsAt(i).data.fd);
			} else {
				HttpPacket httpRequest = convertHttpPacket(buffer);

				HTTP::StatusLine statusLine = {"HTTP/1.1", HTTP::StatusCode::OK,
											   HTTP::StatusCode::to_reasonPhrase(
												   HTTP::StatusCode::OK)};
				HttpPacket httpResponse(statusLine, Header(), Body());
				httpResponse.addHeader("Content-Type", "text/html");
				httpResponse.appendBody("<h2> This is Webserv. </h2>", 27);
				httpResponse.appendBody("Hello, World!", 13);

				writeHttpPacket(_epollManager.getEpollEventsAt(i).data.fd, httpResponse);
			}
		}
	}
}

std::string Server::readSocket(int socketFd) {
	char buffer[SystemConfig::Size::BUFFER_SIZE] = {0};
	int readSize;
	std::string request = "";

	while ((readSize = read(socketFd, buffer, SystemConfig::Size::BUFFER_SIZE)) ==
		   SystemConfig::Size::BUFFER_SIZE) {
		request.append(buffer);
	}
	request.append(buffer);
	return request;
}

void Server::writeHttpPacket(int socketFd, HttpPacket httpResponse) {
	std::string rawResponse = HttpSerializer::serialize(httpResponse);
	std::cout << rawResponse << "\n";
	writeSocket(socketFd, rawResponse);
}

HttpPacket Server::convertHttpPacket(std::string& buffer) {
	HttpParser httpParser(buffer);
	httpParser.parse();
	HttpPacket httpRequest = httpParser.getResult();
	return httpRequest;
}

void Server::writeSocket(int socketFd, std::string rawData) {
	write(socketFd, rawData.c_str(), rawData.size());
}

void Server::runServer() {
	for (unsigned long i = 0; i < _configs.size(); i++) {
		initAddress(i);
		initServer();
	}
	loopServer();
}
