// response.hpp
#ifndef HANDLER_UTILS_RESPONSE_HPP
#define HANDLER_UTILS_RESPONSE_HPP

#include <cstring>
#include <string>

#include "../../config/model/Config.hpp"
#include "../../handler/exception/Exception.hpp"
#include "../../http/Enums.hpp"
#include "../../http/model/Packet.hpp"
#include "../../utils/file_utils.hpp"
#include "../../utils/str_utils.hpp"

namespace handler {
	namespace utils {
		inline http::Packet makePlainResponse(http::StatusCode::Value status,
											  const std::string& body,
											  const std::string& contentType) {
			http::StatusLine statusLine = {"HTTP/1.1", status,
										   http::StatusCode::to_reasonPhrase(status)};
			http::Packet response(statusLine, http::Header(), http::Body());

			response.addHeader("Content-Type", contentType);
			if (!body.empty()) response.appendBody(body.c_str(), body.size());
			return response;
		}

		inline std::string makeErrorResponse(
			const config::Config& config,
			http::StatusCode::Value status = http::StatusCode::InternalServerError) {
			std::string statusLine = "HTTP/1.1 " + int_tostr(status) + " " +
									 http::StatusCode::to_reasonPhrase(status) + "\r\n";
			std::map<int, std::string> errorPages = config.getErrorPages();
			std::map<int, std::string>::const_iterator it = errorPages.find(status);

			if (it != errorPages.end()) {
				FileInfo body = readFile(it->second.c_str());
				if (body.error == FileInfo::NONE) {
					std::string httpHeader =
						statusLine + "Content-Type: " +
						http::ContentType::to_string(http::ContentType::CONTENT_TEXT_HTML) +
						"\r\n" + "Content-Length: " + int_tostr(body.content.size()) + "\r\n" +
						"Server: webserv\r\n";
					return httpHeader + "\r\n" + body.content;
				}
			}
			std::string httpHeader =
				statusLine + "Content-Type: " +
				http::ContentType::to_string(http::ContentType::CONTENT_TEXT_PLAIN) + "\r\n" +
				"Content-Length: " + int_tostr(strlen(http::StatusCode::to_reasonPhrase(status))) +
				"\r\n" + "Server: webserv\r\n";
			return httpHeader + "\r\n" + http::StatusCode::to_reasonPhrase(status);
		}

		inline std::string makeCgiResponse(const std::string& cgiOutput) {
			size_t headerEnd = cgiOutput.find("\r\n\r\n");
			if (headerEnd == std::string::npos) {
				throw handler::Exception();
			}
			std::string httpHeader = cgiOutput.substr(0, headerEnd);
			std::string body = cgiOutput.substr(headerEnd + 4);
			std::string statusLine;
			size_t statusPos = httpHeader.find("Status: ");
			if (statusPos != std::string::npos) {
				size_t lineEnd = httpHeader.find("\r\n", statusPos);
				std::string statusStr = httpHeader.substr(statusPos + 8, lineEnd - (statusPos + 8));
				http::StatusCode::Value statusCode =
					static_cast<http::StatusCode::Value>(str_toint(statusStr));
				statusLine = "HTTP/1.1 " + int_tostr(statusCode) + " " +
							 http::StatusCode::to_reasonPhrase(statusCode) + "\r\n";
				httpHeader.erase(statusPos, lineEnd - statusPos + 2);
			} else {
				throw handler::Exception();
			}
			httpHeader +=
				("\r\n"
				 "Content-Length: " +
				 int_tostr(body.size()) +
				 "\r\n"
				 "Server: webserv\r\n");
			return statusLine + httpHeader + "\r\n" + body;
		}
	}  // namespace utils
}  // namespace handler

#endif
