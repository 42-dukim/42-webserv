#include "EpollCounter.hpp"

EpollCounter::EpollCounter() : _epollCount(0) {}

bool EpollCounter::addFd(int fd) {
	if (_epollCount >= SystemConfig::Size::EPOLL_SIZE) return false;
	_epollFdVector.push_back(fd);
	_epollCount++;
	return true;
}

bool EpollCounter::deleteFd(int fd) {
	if (std::find(_epollFdVector.begin(), _epollFdVector.end(), fd) != _epollFdVector.end()) {
		_epollFdVector.erase(std::find(_epollFdVector.begin(), _epollFdVector.end(), fd));
		_epollCount--;
		return true;
	}
	return false;
}

int EpollCounter::popFd() {
	int fd;

	fd = _epollFdVector.front();
	_epollFdVector.erase(_epollFdVector.begin());
	_epollCount--;
	return fd;
}
