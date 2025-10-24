// ProcessManager.hpp
#ifndef HANDLER_CGI_PROCESS_MANAGER_HPP
#define HANDLER_CGI_PROCESS_MANAGER_HPP

#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include <map>
#include <string>

#include "../../http/model/Packet.hpp"
#include "../../server/epoll/manager/EpollManager.hpp"
#include "../exception/Exception.hpp"
#include "../utils/response.hpp"

namespace handler {
	namespace cgi {
		class ProcessManager {
			private:
				struct Process {
						pid_t pid;
						int cgiFd;
						int clientFd;
						std::string output;
						bool completed;
						Process() : pid(-1), cgiFd(-1), clientFd(-1), completed(false) {}
				};
				std::map<int, Process> _activeProcesses;
				std::map<int, int> _clientToCgi;

			public:
				ProcessManager() {}
				~ProcessManager() {}

				static void sigchldHandler(int);

				int getClientFd(int) const;
				void registerProcess(pid_t, int, int);
				void handleCgiEvent(int, server::EpollManager&);
				bool isCgiProcess(int) const;
				bool isProcessing(int) const;
				bool isCompleted(int) const;
				void removeCgiProcess(int);
				std::string getResponse(int);
		};
	}  // namespace cgi
}  // namespace handler

#endif