// ErrorBuilder.cpp
#include "ErrorBuilder.hpp"

#include "../utils/response.hpp"

using namespace handler::builder;

std::string ErrorBuilder::joinAllowMethods(const std::vector<std::string>& methods) {
	std::string allow;

	for (size_t i = 0; i < methods.size(); ++i) {
		if (i) allow += ", ";
		allow += methods[i];
	}
	return allow;
}

http::Packet ErrorBuilder::build(const router::RouteDecision& decision, const http::Packet&,
								 const std::vector<config::Config>&) const {
	http::Packet response = utils::makePlainResponse(
		decision.status, http::StatusCode::to_reasonPhrase(decision.status),
		http::ContentType::to_string(http::ContentType::CONTENT_TEXT_PLAIN));
	std::string allow = joinAllowMethods(decision.allow_methods);
	if (!allow.empty()) response.addHeader("Allow", allow);
	return response;
}
