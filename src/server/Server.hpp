#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include "Config.hpp"
#include "EpollManager.hpp"
#include "HttpParser.hpp"
#include "SystemConfig.hpp"
#include "exception/ServerException.hpp"

class Server {
	private:
		const int		   _PORT;
		int				   _serverSocket;
		int				   _clientSocket;
		int				   _socketOption;
		int				   _addressSize;
		struct sockaddr_in _serverAddress;
		struct sockaddr_in _clientAddress;
		EpollManager	   _epollManager;

		void		initAddress();
		void		initServer();
		void		loopServer();
		void		writeHttpPacket(int socketFd, HttpResponse httpResponse);
		void		handleEvents();
		void		writeSocket(int socketFd, std::string rawData);
		int			validateFunction(int returnValue, std::string type);
		std::string readSocket(int socketFd);
		HttpRequest readHttpPacket(int socketFd);

	public:
		Server(Config config);

		void runServer();
};

#endif