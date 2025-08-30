// EpollManager.hpp
#ifndef EPOLLMANAGER_HPP
#define EPOLLMANAGER_HPP

#include <sys/epoll.h>
#include <unistd.h>

#include <iostream>
#include <vector>

#include "../../../../include/SystemConfig.hpp"
#include "../counter/EpollCounter.hpp"
#include "../exception/EpollException.hpp"

class EpollManager {
	private:
		static const int EPOLL_SIZE = SystemConfig::Size::EPOLL_SIZE;
		int _epollFd;
		int _eventCount;
		struct epoll_event _event;
		std::vector<struct epoll_event> _epollEvents;
		EpollCounter _epollCounter;

	public:
		int getEpollFd() const;
		int getEventCount() const;
		const struct epoll_event& getEpollEventsAt(int) const;
		void initEpoll();
		void addEpollFd(int);
		void deleteEpollFd(int);
		void waitEvent();
};

#endif
