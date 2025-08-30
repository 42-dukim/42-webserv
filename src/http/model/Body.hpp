// Body.hpp
#ifndef BODY_HPP
#define BODY_HPP

#include <vector>

#include "../types/HttpEnums.hpp"

class Body {
	private:
		std::vector<unsigned char> _data;
		HTTP::ContentType::Value _type;
		size_t _length;

	public:
		Body();
		~Body();
		Body(const Body&);
		Body& operator=(const Body&);

		const std::vector<unsigned char>& getData() const;
		HTTP::ContentType::Value getType() const;
		size_t getLength() const;

		void setType(HTTP::ContentType::Value);
		void setLength(size_t);

		void append(const char*, size_t);
};

#endif
