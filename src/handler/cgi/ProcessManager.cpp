// ProcessManager.cpp
#include "ProcessManager.hpp"

#include <cerrno>

using namespace handler::cgi;

void ProcessManager::sigchldHandler(int sig) {
	(void) sig;
	int status;
	while (waitpid(-1, &status, WNOHANG) > 0) {
	}
}

int ProcessManager::getClientFd(int cgiFd) const {
	std::map<int, Process>::const_iterator it = _activeProcesses.find(cgiFd);
	if (it == _activeProcesses.end()) {
		return -1;
	}
	return it->second.clientFd;
}

void ProcessManager::registerProcess(pid_t pid, int cgiFd, int clientFd) {
	Process process(pid, cgiFd, clientFd);

	_activeProcesses[cgiFd] = process;
	_clientToCgi[clientFd] = cgiFd;
}

// rename 고민
void ProcessManager::handleCgiEvent(int cgiFd, server::EpollManager& epollManager) {
	std::map<int, Process>::iterator it = _activeProcesses.find(cgiFd);
	if (it == _activeProcesses.end()) {
		return;
	}
	Process& process = it->second;
	char buffer[4096];
	bool eof = false;

	while (true) {
		int n = read(cgiFd, buffer, sizeof(buffer));
		if (n > 0) {
			process.output.append(buffer, n);
			continue;
		}
		if (n == 0) eof = true;
		if (n < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
			// no more data for now
		} else if (n < 0 && errno != EINTR)
			eof = true;
		break;
	}
	process.completed = eof;
	if (eof) epollManager.remove(cgiFd);
}

bool ProcessManager::isCgiProcess(int fd) const {
	return _activeProcesses.find(fd) != _activeProcesses.end();
}

bool ProcessManager::isProcessing(int clientFd) const {
	return _clientToCgi.find(clientFd) != _clientToCgi.end();
}

bool ProcessManager::isCompleted(int fd) const {
	std::map<int, int>::const_iterator it = _clientToCgi.find(fd);
	if (it == _clientToCgi.end()) {
		return false;
	}

	int cgiFd = it->second;
	std::map<int, Process>::const_iterator cgiIt = _activeProcesses.find(cgiFd);
	if (cgiIt == _activeProcesses.end()) {
		return false;
	}
	return cgiIt->second.completed;
}

void ProcessManager::removeCgiProcess(int clientFd) {
	std::map<int, int>::iterator it = _clientToCgi.find(clientFd);
	if (it != _clientToCgi.end()) {
		int cgiFd = it->second;
		_clientToCgi.erase(it);

		std::map<int, Process>::iterator procIt = _activeProcesses.find(cgiFd);
		if (procIt != _activeProcesses.end()) _activeProcesses.erase(procIt);
	}
}

std::string ProcessManager::getResponse(int cgiFd) {
	std::map<int, Process>::iterator procIt = _activeProcesses.find(cgiFd);
	if (procIt == _activeProcesses.end()) throw handler::Exception();
	std::string output = procIt->second.output;
	if (output.empty()) throw handler::Exception();

	return output;
}
