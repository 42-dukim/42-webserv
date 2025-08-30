#include "EpollManager.hpp"

void EpollManager::initEpoll() {
	_epollFd = epoll_create(EPOLL_SIZE);
	_epollEvents.resize(EPOLL_SIZE);
}

void EpollManager::addEpollFd(int socketFd) {
	if (!_epollCounter.addFd(socketFd)) {
		deleteEpollFd(_epollCounter.popFd());
		_epollCounter.addFd(socketFd);
	}
	_event.events = EPOLLIN | EPOLLRDHUP;
	_event.data.fd = socketFd;
	epoll_ctl(_epollFd, EPOLL_CTL_ADD, socketFd, &_event);
}

void EpollManager::deleteEpollFd(int socketFd) {
	if (_epollCounter.deleteFd(socketFd)) {
		epoll_ctl(_epollFd, EPOLL_CTL_DEL, socketFd, NULL);
		close(socketFd);
	}
}

void EpollManager::waitEvent() {
	if ((_eventCount = (epoll_wait(_epollFd, (struct epoll_event*) _epollEvents.data(), EPOLL_SIZE,
								   1000))) == -1)
		throw EpollException("wait event");
}

int EpollManager::getEpollFd() const {
	return _epollFd;
}

int EpollManager::getEventCount() const {
	return _eventCount;
}

const struct epoll_event& EpollManager::getEpollEventsAt(int index) const {
	if (index < 0 || static_cast<int>(_epollEvents.size()) <= index) {
		throw EpollException("events out of bounds");
	}
	return _epollEvents[index];
}
