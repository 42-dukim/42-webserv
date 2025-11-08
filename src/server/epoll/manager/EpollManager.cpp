// EpollManager.cpp
#include "EpollManager.hpp"

#include "../exception/EpollException.hpp"

using namespace server;

EpollManager::EpollManager() : _epollFd(-1), _eventCount(0) {}

EpollManager::~EpollManager() {
	if (_epollFd != -1) close(_epollFd);
}

int EpollManager::fd() const {
	return _epollFd;
}

int EpollManager::eventCount() const {
	return _eventCount;
}

const epoll_event& EpollManager::eventAt(int index) const {
	if (index < 0 || static_cast<int>(_events.size()) <= index) {
		throw EpollException("events out of bounds");
	}
	return _events[index];
}

void EpollManager::init() {
	_epollFd = epoll_create(kMaxEvents);
	if (_epollFd == -1) throw EpollException("create");
	_events.resize(kMaxEvents);
}

void EpollManager::add(int fd) {
	if (!_counter.addFd(fd)) {
		remove(_counter.popFd());
		_counter.addFd(fd);
	}
	_event.events = EPOLLIN | EPOLLRDHUP;
	_event.data.fd = fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &_event) == -1) {
		throw EpollException("ctl add");
	}
}

void EpollManager::add(int fd, unsigned int events) {
	if (!_counter.addFd(fd)) {
		remove(_counter.popFd());
		_counter.addFd(fd);
	}
	_event.events = events;
	_event.data.fd = fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &_event) == -1) {
		throw EpollException("ctl add");
	}
}

void EpollManager::modify(int fd, unsigned int events) {
	_event.events = events;
	_event.data.fd = fd;
	if (epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &_event) == -1) {
		throw EpollException("ctl mod");
	}
}

void EpollManager::remove(int fd) {
	if (_counter.deleteFd(fd)) {
		if (epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, NULL) == -1) {
			throw EpollException("ctl del");
		}
		close(fd);
	}
}

void EpollManager::wait() {
	_eventCount = epoll_wait(_epollFd, _events.data(), kMaxEvents, 1000);
	if (_eventCount == -1) {
		if (errno == EINTR) {
			_eventCount = 0;
			return;
		}
		throw EpollException("wait");
	}
}

// namespace server
