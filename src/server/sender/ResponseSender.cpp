// ResponseSender.cpp
#include "ResponseSender.hpp"

#include <algorithm>

#include "../Defaults.hpp"

using namespace server;

SendResult ResponseSender::send() {
	size_t remaining = _responseBuffer.size() - _responseSent;
	size_t toSend = std::min(remaining, static_cast<size_t>(server::defaults::BUFFER_SIZE));

	ssize_t bytesSent =
		::send(_socketFd, _responseBuffer.c_str() + _responseSent, toSend, MSG_NOSIGNAL);
	if (bytesSent > 0) {
		_responseSent += bytesSent;
	} else if (bytesSent < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) {
		return SendResult::RETRY;
	} else {
		return SendResult::ERROR;  // Error occurred -> 상위에서 cleanup 처리
	}

	if (_responseSent >= _responseBuffer.size()) return SendResult::SUCCESS;
	return SendResult::RETRY;
}