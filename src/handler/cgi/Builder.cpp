// CgiBuilder.cpp
#include "Builder.hpp"

#include "../../utils/str_utils.hpp"

using namespace handler::cgi;

void Builder::setArgv(const router::RouteDecision& decision) {
	_argv.push_back(const_cast<char*>("/usr/bin/python3"));
	_argv.push_back(const_cast<char*>(decision.fs_path.c_str()));
	_argv.push_back(NULL);
}

void Builder::setEnvp(const router::RouteDecision& decision, const http::Packet& request) {
	std::string requestMethod = http::Method::to_string(request.getStartLine().method);
	std::string contentType = request.getHeader().get("Content-Type");
	std::string contentLength = request.getHeader().get("Content-Length");
	std::string scriptName = request.getStartLine().target;
	std::string serverPort = int_tostr(decision.server->getListen());
	std::string serverName = decision.server->getServerName();

	_envStrings.push_back("REQUEST_METHOD=" + requestMethod);
	_envStrings.push_back("QUERY_STRING=");
	_envStrings.push_back("CONTENT_TYPE=" + contentType);
	_envStrings.push_back("CONTENT_LENGTH=" + contentLength);
	_envStrings.push_back("SCRIPT_NAME=" + scriptName);
	_envStrings.push_back("PATH_INFO=" + decision.fs_path);
	_envStrings.push_back("PATH_TRANSLATED=" + decision.fs_root + decision.fs_path);
	_envStrings.push_back("SERVER_NAME=" + serverName);
	_envStrings.push_back("SERVER_PORT=" + serverPort);
	_envStrings.push_back("SERVER_PROTOCOL=" + request.getStartLine().version);
	_envStrings.push_back("REMOTE_ADDR=127.0.0.1");
	_envStrings.push_back("REMOTE_PORT=0");
	_envStrings.push_back("GATEWAY_INTERFACE=CGI/1.1");

	for (size_t i = 0; i < _envStrings.size(); ++i) {
		_envp.push_back(const_cast<char*>(_envStrings[i].c_str()));
	}
	_envp.push_back(NULL);
}

void Builder::build(const router::RouteDecision& decision, const http::Packet& request,
					server::EpollManager& epollManager, cgi::Manager& cgiManager, int clientFd) {
	int stdoutPipe[2], stdinPipe[2];
	if (pipe2(stdoutPipe, O_NONBLOCK) == -1 || pipe(stdinPipe) == -1) {
		throw Exception();
	}

	pid_t pid = fork();
	if (pid == -1) {
		close(stdoutPipe[0]);
		close(stdoutPipe[1]);
		close(stdinPipe[0]);
		close(stdinPipe[1]);
		throw Exception();
	}

	if (pid == 0) {
		close(stdoutPipe[0]);
		close(stdinPipe[1]);
		dup2(stdoutPipe[1], STDOUT_FILENO);
		dup2(stdoutPipe[1], STDERR_FILENO);
		close(stdoutPipe[1]);
		dup2(stdinPipe[0], STDIN_FILENO);
		close(stdinPipe[0]);

		setArgv(decision);
		setEnvp(decision, request);

		execve(_argv[0], _argv.data(), _envp.data());
		exit(1);
	}
	close(stdoutPipe[1]);
	close(stdinPipe[0]);

	// 필요시 POST 데이터를 CGI로 전송
	if (request.getStartLine().method == http::Method::POST) {
		const std::vector<unsigned char>& body_data = request.getBody().getData();
		if (!body_data.empty()) {
			write(stdinPipe[1], body_data.data(), body_data.size());
		}
	}
	close(stdinPipe[1]);

	// 모니터링을 위해 CGI stdout을 epoll에 추가
	epollManager.add(stdoutPipe[0], EPOLLIN | EPOLLET);
	cgiManager.registerProcess(pid, stdoutPipe[0], clientFd);
}
