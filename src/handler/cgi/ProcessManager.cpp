// ProcessManager.cpp
#include "ProcessManager.hpp"

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
	Process process;
	process.pid = pid;
	process.cgiFd = cgiFd;
	process.clientFd = clientFd;
	process.completed = false;
	process.output.clear();

	_activeProcesses[cgiFd] = process;
	_clientToCgi[clientFd] = cgiFd;
}

void ProcessManager::handleCgiEvent(int cgiFd, server::EpollManager& epollManager) {
	std::map<int, Process>::iterator it = _activeProcesses.find(cgiFd);
	if (it == _activeProcesses.end()) {
		return;
	}
	Process& process = it->second;
	char buffer[4096];
	int n;
	while ((n = read(cgiFd, buffer, sizeof(buffer) - 1)) > 0) {
		buffer[n] = '\0';
		process.output.append(buffer);
	}
	process.completed = true;
	epollManager.remove(cgiFd);
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

std::string ProcessManager::getResponse(int clientFd) {
	std::map<int, int>::iterator it = _clientToCgi.find(clientFd);
	if (it == _clientToCgi.end()) throw handler::Exception();

	int cgiFd = it->second;
	std::map<int, Process>::iterator procIt = _activeProcesses.find(cgiFd);
	if (procIt == _activeProcesses.end()) throw handler::Exception();

	std::string output = procIt->second.output;
	if (output.empty()) throw handler::Exception();

	_activeProcesses.erase(procIt);
	_clientToCgi.erase(it);
	return output;
}
