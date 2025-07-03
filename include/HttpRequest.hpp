#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include <string>

#include "HttpPacket.hpp"

class HttpRequest : public HttpPacket {
	private:
		std::string _httpMethod;
		std::string _target;

	public:
		HttpRequest(std::string header, std::string body,
					std::string httpMethod, std::string target);
};

#endif