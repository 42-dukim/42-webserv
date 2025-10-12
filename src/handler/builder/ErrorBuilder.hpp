// ErrorBuilder.hpp
#ifndef HANDLER_BUILDER_ERROR_HPP
#define HANDLER_BUILDER_ERROR_HPP

#include "Builder.hpp"

namespace handler {
	namespace builder {
		class ErrorBuilder : public IBuilder {
			private:
				static std::string joinAllowMethods(const std::vector<std::string>&);

			public:
				virtual http::Packet build(const router::RouteDecision&, const http::Packet&,
										   const std::vector<config::Config>&) const;
		};
	}  // namespace builder
}  // namespace handler

#endif
