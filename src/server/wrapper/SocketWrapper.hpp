#ifndef SOCKETWRAPPER_HPP
#define SOCKETWRAPPER_HPP

#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include "../exception/ServerException.hpp"

namespace SocketWrapper {
	inline int accept(int socket, struct sockaddr* address, socklen_t* address_len) {
		int returnValue = ::accept(socket, address, address_len);
		if (returnValue < 0) throw ServerException("accept");
		return returnValue;
	}

	inline void bind(int socket, const struct sockaddr* address, socklen_t address_len) {
		int returnValue = ::bind(socket, address, address_len);
		if (returnValue < 0) throw ServerException("bind");
	}

	inline void listen(int socket, int backlog) {
		int returnValue = ::listen(socket, backlog);
		if (returnValue < 0) throw ServerException("listen");
	}

	inline int socket(int domain, int type, int protocol) {
		int returnValue = ::socket(domain, type, protocol);
		if (returnValue < 0) throw ServerException("socket");
		return returnValue;
	}

	inline void setsockopt(int socket, int level, int option_name, const void* option_value,
						   socklen_t option_len) {
		int returnValue = ::setsockopt(socket, level, option_name, option_value, option_len);
		if (returnValue < 0) throw ServerException("setsockopt");
	}
}

#endif
