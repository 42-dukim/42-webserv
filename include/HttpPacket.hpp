#ifndef HTTPPACKET_HPP
#define HTTPPACKET_HPP

#include <string>

class HttpPacket {
	protected:
		const std::string _version;
		std::string		  _header;
		std::string		  _body;

	public:
		HttpPacket(std::string header, std::string body);
};

#endif