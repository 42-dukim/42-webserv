#ifndef HTTPPARSER_HPP
#define HTTPPARSER_HPP

#include <string>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"

class HttpParser {
	public:
		static HttpRequest	parseRequestHttp(std::string rawDate);
		static HttpResponse parseResponseHttp(std::string rawDate);
};

#endif