#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include <string>

#include "HttpPacket.hpp"

class HttpResponse : public HttpPacket {
	private:
		int			_statusCode;
		std::string _statusText;

	public:
		HttpResponse(std::string header, std::string body, int statusCode);
		std::string getRawData();
};

#endif