// EpollManager.hpp
#ifndef SERVER_EPOLL_MANAGER_HPP
#define SERVER_EPOLL_MANAGER_HPP

#include <sys/epoll.h>
#include <unistd.h>

#include <vector>

#include "../../Defaults.hpp"
#include "../counter/EpollCounter.hpp"

namespace server {
	class EpollManager {
		private:
			static const int kMaxEvents = defaults::EPOLL_SIZE;
			int _epollFd;
			int _eventCount;
			epoll_event _event;
			std::vector<epoll_event> _events;
			EpollCounter _counter;

		public:
			EpollManager();
			~EpollManager();

			int fd() const;
			int eventCount() const;
			const epoll_event& eventAt(int) const;
			void init();
			void add(int);
			void add(int, unsigned int);
			void modify(int, unsigned int);
			void remove(int);
			void wait();
	};
}  // namespace server

#endif
