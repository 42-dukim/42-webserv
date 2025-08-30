// HttpPacket.hpp
#ifndef HTTPPACKET_HPP
#define HTTPPACKET_HPP

#include <stdexcept>
#include <string>

#include "../types/PacketLine.hpp"
#include "Body.hpp"
#include "Header.hpp"

class HttpPacket {
	private:
		HTTP::StartLine _startLine;
		HTTP::StatusLine _statusLine;
		Header _header;
		Body _body;
		bool _isRequest;

	public:
		HttpPacket(const HTTP::StartLine&, const Header&, const Body&);
		HttpPacket(const HTTP::StatusLine&, const Header&, const Body&);

		const HTTP::StartLine& getStartLine() const;
		const HTTP::StatusLine& getStatusLine() const;
		const Header& getHeader() const;
		const Body& getBody() const;
		bool isRequest() const;

		void addHeader(const std::string&, const std::string&);
		void appendBody(const char*, size_t);
		void applyBodyLength(size_t);
		void applyBodyType(HTTP::ContentType::Value);
};

#endif
