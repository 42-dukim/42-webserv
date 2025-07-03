#ifndef EPOLLCOUNTER_HPP
#define EPOLLCOUNTER_HPP

#include <algorithm>
#include <vector>

#include "EpollManager.hpp"
#include "SystemConfig.hpp"

class EpollCounter {
	private:
		std::vector<int> _epollFdVector;
		int				 _epollCount;
		int				 _epollMaxCount;

	public:
		EpollCounter();

		bool addFd(int fd);
		bool deleteFd(int fd);
		int	 popFd();
};

#endif