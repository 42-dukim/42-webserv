// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <set>
#include <string>
#include <vector>

#include "../../include/SystemConfig.hpp"
#include "../config/model/Config.hpp"
#include "../http/model/HttpPacket.hpp"
#include "../http/parser/HttpParser.hpp"
#include "../http/serializer/HttpSerializer.hpp"
#include "epoll/manager/EpollManager.hpp"
#include "exception/ServerException.hpp"
#include "wrapper/SocketWrapper.hpp"

class Server {
	private:
		std::vector<Config> _configs;
		std::set<int> _serverSockets;
		int _clientSocket;
		int _socketOption;
		int _addressSize;
		struct sockaddr_in _serverAddress;
		struct sockaddr_in _clientAddress;
		EpollManager _epollManager;

		void initAddress(int);
		void initServer();
		void loopServer();
		void writeHttpPacket(int, HttpPacket);
		void handleEvents();
		void writeSocket(int, std::string);
		std::string readSocket(int);
		HttpPacket convertHttpPacket(std::string&);

	public:
		Server(std::vector<Config>);

		void runServer();
};

#endif
