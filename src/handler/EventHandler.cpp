// EventHandler.cpp
#include "EventHandler.hpp"

#include <unistd.h>

#include <cerrno>

#include "../handler/utils/response.hpp"
#include "../http/Enums.hpp"
#include "../http/model/Packet.hpp"
#include "../http/serializer/Serializer.hpp"
#include "../server/Defaults.hpp"
#include "cgi/Executor.hpp"

using namespace handler;

EventHandler::EventHandler() {}

EventHandler::~EventHandler() {
	for (std::map<int, http::Parser*>::iterator it = _parsers.begin(); it != _parsers.end(); ++it) {
		delete it->second;
	}
	_parsers.clear();
}

EventHandler::Result EventHandler::handleEvent(int fd, uint32_t events,
											   const config::Config* config,
											   server::EpollManager& epollManager) {
	if (_cgiProcessManager.isCgiProcess(fd)) {
		return handleCgiEvent(fd, events, config, epollManager);
	}
	return handleClientEvent(fd, events, config, epollManager);
}

EventHandler::Result EventHandler::handleCgiEvent(int fd, uint32_t, const config::Config* config,
												  server::EpollManager& epollManager) {
	Result result;
	int clientFd = _cgiProcessManager.getClientFd(fd);
	if (clientFd == -1) return result;

	_cgiProcessManager.handleCgiEvent(fd, epollManager);
	if (!_cgiProcessManager.isCompleted(clientFd)) return result;

	std::map<int, const config::Config*>::const_iterator it = _cgiClientConfigs.find(clientFd);
	if (it != _cgiClientConfigs.end()) config = it->second;

	std::string rawResponse;
	try {
		std::string cgiOutput = _cgiProcessManager.getResponse(fd);
		rawResponse = config ? utils::makeCgiResponse(cgiOutput) : utils::makeErrorResponse(*config);
	} catch (const handler::Exception&) {
		rawResponse = utils::makeErrorResponse(*config);
	}
	_cgiProcessManager.removeCgiProcess(clientFd);
	_cgiClientConfigs.erase(clientFd);
	result.response = Response(clientFd, rawResponse, true);
	return result;
}

EventHandler::Result EventHandler::handleClientEvent(int fd, uint32_t events,
													 const config::Config* config,
													 server::EpollManager& epollManager) {
	Result result;
	bool disconnected = (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) != 0;
	std::string buffer = readSocket(fd);
	http::Parser* parser = ensureParser(fd);

	if (!buffer.empty()) parser->append(buffer);
	if (disconnected) parser->markEndOfInput();
	if (buffer.empty() && !disconnected) return result;

	while (true) {
		http::Parser::Result parseResult = parser->parse();
		switch (parseResult.status) {
			case http::Parser::Result::Incomplete:
				break;
			case http::Parser::Result::Error: {
				const std::string& body =
					parseResult.errorMessage.empty()
						? http::StatusCode::to_reasonPhrase(parseResult.errorCode)
						: parseResult.errorMessage;
				http::Packet errorPacket =
					utils::makePlainResponse(parseResult.errorCode, body,
											 http::ContentType::to_string(
												 http::ContentType::CONTENT_TEXT_PLAIN));
				result.response = Response(fd, http::Serializer::serialize(errorPacket), true);
				break;
			}
			case http::Parser::Result::Completed: {
				if (!config) {
					http::Packet errorPacket = utils::makePlainResponse(
						http::StatusCode::InternalServerError,
						http::StatusCode::to_reasonPhrase(http::StatusCode::InternalServerError),
						http::ContentType::to_string(http::ContentType::CONTENT_TEXT_PLAIN));
					result.response = Response(fd, http::Serializer::serialize(errorPacket), true);
					break;
				}

				http::Packet httpRequest = parseResult.packet;
				std::string remainder = parseResult.leftover;
				bool ended = parseResult.endOfInput;

				router::RouteDecision decision = _router.route(httpRequest, *config);
				if (decision.action == router::RouteDecision::Cgi) {
					_cgiClientConfigs[fd] = decision.server;
					cgi::Executor executor;
					executor.execute(decision, httpRequest, epollManager, _cgiProcessManager, fd);

					if (ended) result.closeFd = fd;
					if (!remainder.empty()) {
						parser->append(remainder);
						if (ended) parser->markEndOfInput();
						if (!ended) continue;
					}
					break;
				}

				http::Packet httpResponse =
					_requestHandler.handle(fd, httpRequest, decision, *config);
				result.response = Response(fd, http::Serializer::serialize(httpResponse), ended);

				if (!remainder.empty()) {
					parser->append(remainder);
					if (ended) parser->markEndOfInput();
					if (!ended) continue;
				}
				if (ended) result.closeFd = fd;
				break;
			}
		}
		break;
	}

	if (disconnected) result.closeFd = fd;
	return result;
}

void EventHandler::cleanup(int fd) {
	std::map<int, http::Parser*>::iterator it = _parsers.find(fd);
	if (it != _parsers.end()) {
		delete it->second;
		_parsers.erase(it);
	}
	_cgiClientConfigs.erase(fd);
	_cgiProcessManager.removeCgiProcess(fd);
}

http::Parser* EventHandler::ensureParser(int fd) {
	std::map<int, http::Parser*>::iterator it = _parsers.find(fd);
	if (it == _parsers.end()) {
		http::Parser* parser = new http::Parser();
		_parsers.insert(std::make_pair(fd, parser));
		return parser;
	}
	return it->second;
}

std::string EventHandler::readSocket(int socketFd) const {
	char buffer[server::defaults::BUFFER_SIZE] = {0};
	std::string request;

	while (true) {
		ssize_t readSize = ::read(socketFd, buffer, server::defaults::BUFFER_SIZE);
		if (readSize > 0) {
			request.append(buffer, readSize);
			if (readSize < server::defaults::BUFFER_SIZE) break;
		} else if (readSize == 0) {
			break;
		} else {
			if (errno == EAGAIN || errno == EWOULDBLOCK) break;
			return std::string();
		}
	}
	return request;
}
