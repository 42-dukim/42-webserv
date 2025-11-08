// ResponseSender.hpp
#ifndef SERVER_SENDER_RESPONSESENDER_HPP
#define SERVER_SENDER_RESPONSESENDER_HPP

#include <sys/socket.h>
#include <unistd.h>

#include <map>
#include <string>

#include "../../http/model/Packet.hpp"
#include "../../http/serializer/Serializer.hpp"

namespace server {
	enum class SendResult {
		SUCCESS,
		RETRY,
		ERROR
	};

	class ResponseSender {
		private:
			std::string _responseBuffer;
			int _socketFd;
			size_t _responseSent;
			ResponseSender();

		public:
			ResponseSender(int socketFd, const http::Packet& httpPacket) :
				_responseBuffer(http::Serializer::serialize(httpPacket)),
				_socketFd(socketFd),
				_responseSent(0) {}
			ResponseSender(int socketFd, const std::string& rawResponse) :
				_responseBuffer(rawResponse), _socketFd(socketFd), _responseSent(0) {}
			SendResult send();
	};
}  // namespace server
#endif