// Executor.hpp
#ifndef HANDLER_CGI_EXECUTOR_HPP
#define HANDLER_CGI_EXECUTOR_HPP

#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

#include <map>

#include "../../config/model/Config.hpp"
#include "../../http/model/Packet.hpp"
#include "../../router/model/RouteDecision.hpp"
#include "../../server/epoll/manager/EpollManager.hpp"
#include "../exception/Exception.hpp"
#include "../utils/response.hpp"
#include "ProcessManager.hpp"

namespace handler {
	namespace cgi {
		class Executor {
			private:
				std::vector<std::string> _envStrings;
				std::vector<char*> _envp;
				std::vector<char*> _argv;

				void setArgv(const router::RouteDecision&);
				void setEnvp(const router::RouteDecision&, const http::Packet&);

			public:
				Executor() {}
				~Executor() {}
				void execute(const router::RouteDecision&, const http::Packet&,
							 server::EpollManager&, cgi::ProcessManager&, int);
		};
	}  // namespace cgi
}  // namespace handler

#endif
