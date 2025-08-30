// EpollCounter.hpp
#ifndef EPOLLCOUNTER_HPP
#define EPOLLCOUNTER_HPP

#include <algorithm>
#include <vector>

#include "../../../../include/SystemConfig.hpp"

class EpollCounter {
	private:
		std::vector<int> _epollFdVector;
		int _epollCount;

	public:
		EpollCounter();

		bool addFd(int);
		bool deleteFd(int);
		int popFd();
};

#endif