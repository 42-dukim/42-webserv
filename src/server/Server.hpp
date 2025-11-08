// Server.hpp
#ifndef SERVER_HPP
#define SERVER_HPP

#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <set>
#include <string>
#include <vector>

#include "../config/model/Config.hpp"
#include "../handler/EventHandler.hpp"
#include "../http/model/Packet.hpp"
#include "../http/serializer/Serializer.hpp"
#include "epoll/manager/EpollManager.hpp"
#include "sender/ResponseSender.hpp"

namespace server {
	class Server {
		private:
			std::map<int, config::Config> _configs;
			std::map<int, ResponseSender> _responseSenders;
			std::set<int> _serverSockets;
			int _clientSocket;
			int _socketOption;
			int _addressSize;
			sockaddr_in _serverAddress;
			sockaddr_in _clientAddress;
			EpollManager _epollManager;
			handler::EventHandler _eventHandler;

			const config::Config* findConfig(int) const;

			void initServer(int);
			void loop();
			void handleEvents();

		public:
			explicit Server(const std::map<int, config::Config>&);

			void run();
	};
}  // namespace server

#endif
