// PacketLine.hpp
#ifndef PACKET_LINE
#define PACKET_LINE

#include <string>

#include "HttpEnums.hpp"

namespace HTTP {
	struct StartLine {
			HTTP::Method::Value method;
			std::string target;
			std::string version;
	};

	struct StatusLine {
			std::string version;
			HTTP::StatusCode::Value statusCode;
			std::string reasonPhrase;
	};
}

#endif
