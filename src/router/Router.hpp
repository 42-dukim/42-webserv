// Router.hpp
#ifndef ROUTER_ROUTER_HPP
#define ROUTER_ROUTER_HPP

#include <string>
#include <vector>

#include "../config/model/Config.hpp"
#include "../http/model/Packet.hpp"
#include "model/RouteDecision.hpp"

namespace router {
	class Router {
		private:
			const config::Config* selectServer(const std::vector<config::Config>&, int) const;
			std::string bestLocationPrefix(const config::Config&, const std::string&) const;

			bool ensureRequestIsValid(const http::Packet&, RouteDecision&) const;
			void resolveLocation(const config::Config&, const http::Packet&, std::string&,
								 std::string&, RouteDecision&) const;
			bool validateMethod(const config::Config&, const http::Packet&, const std::string&,
								RouteDecision&) const;
			bool validateBodySize(const config::Config&, const http::Packet&, RouteDecision&) const;
			bool decideResource(const config::Config&, const std::string&, const std::string&,
								RouteDecision&) const;

		public:
			Router() {}
			RouteDecision route(const http::Packet&, const std::vector<config::Config>&, int) const;
	};
}  // namespace router

#endif
