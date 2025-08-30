// ConfFile.hpp
#ifndef CONFFILE_HPP
#define CONFFILE_HPP

namespace ConfFile {
	inline const char* DEFAULT_PATH() {
		return "default.conf";
	}
	inline const char* DEFAULT_SERVER_NAME() {
		return "_";
	}
	inline const char* DEFAULT_INDEX() {
		return "index.html";
	}
	static const long long DEFAULT_CLIENT_MAX_BODY_SIZE = 1024 * 1024;
	static const long long LIMIT_CLIENT_MAX_BODY_SIZE = 2LL * 1024 * 1024 * 1024;  // 2GB
}

#endif
