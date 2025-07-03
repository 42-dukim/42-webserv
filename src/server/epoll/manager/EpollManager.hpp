#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <sys/epoll.h>

#include <iostream>
#include <vector>

#include "EpollCounter.hpp"
#include "EpollException.hpp"
#include "Server.hpp"
#include "SystemConfig.hpp"

class EpollManager {
	private:
		int				 _epollFd;
		static const int EPOLL_SIZE = SystemConfig::Size::EPOLL_SIZE;
		std::vector<struct epoll_event> _epollEvents;
		struct epoll_event				_event;
		int								_eventCount;
		EpollCounter					_epollCounter;

	public:
		void			   initEpoll(int serverSocket);
		void			   addEpollFd(int socketFd);
		void			   deleteEpollFd(int socketFd);
		void			   waitEvent();
		int				   getEpollFd();
		int				   getEventCount();
		struct epoll_event getEpollEventsAt(int index);
};

#endif