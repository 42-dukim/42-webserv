#include "Config.hpp"

Config::Config() {
	_listen = -1;
	_server_name = "_";
	_index = "index.html";
}

Config::Config(const Config& other) { *this = other; }

Config& Config::operator=(const Config& other) {
	_listen = other._listen;
	_server_name = other._server_name;
	_index = other._index;
	_root = other._root;
	_location = other._location;
	return *this;
}

int Config::getListen() const {
    return _listen;
}

const std::string& Config::getServerName() const {
    return _server_name;
}

const std::string& Config::getIndex() const {
    return _index;
}

const std::string& Config::getRoot() const {
    return _root;
}

const std::map<std::string, ConfigLocation>& Config::getLocation() const {
    return _location;
}

void Config::setListen(int listen) {
    _listen = listen;
}

void Config::setServerName(const std::string& serverName) {
    _server_name = serverName;
}

void Config::setIndex(const std::string& index) {
    _index = index;
}

void Config::setRoot(const std::string& root) {
    _root = root;
}

void Config::setLocation(const std::string& path) {
	if (_location.find(path) != _location.end())
		throw ConfigException("[emerg] Invalid configuration: duplicate location");
	ConfigLocation location;
	location._root = _root;
	location._index = _index;
    _location[path] = location;
}

void Config::setLocationRoot(const std::string& path, const std::string& root) {
    _location[path]._root = root;
}

void Config::setLocationIndex(const std::string& path, const std::string& index) {
    _location[path]._index = index;
}

void Config::setLocationAllowMethod(const std::string& path, const std::vector<std::string>& methods) {
    _location[path]._allow_method = methods;
}

const std::string& Config::getLocationRoot(const std::string& path) const {
    std::map<std::string, ConfigLocation>::const_iterator it = _location.find(path);
    if (it == _location.end()) {
        throw ConfigException("Invalid path: " + path);
    }
    return it->second._root;
}

const std::string& Config::getLocationIndex(const std::string& path) const {
    auto it = _location.find(path);
    if (it == _location.end()) {
        throw ConfigException("Invalid path: " + path);
    }
    return it->second._index;
}

const std::vector<std::string>& Config::getLocationAllowMethod(const std::string& path) const {
    auto it = _location.find(path);
    if (it == _location.end()) {
        throw ConfigException("Invalid path: " + path);
    }
    return it->second._allow_method;
}

//🚨 반드시 명시해야 하는 필수 지시어들
//지시어
//listen: 서버가 어떤 포트에서 요청을 받을지 지정 (예: listen 80;)
//location: 요청 경로에 따라 처리 방식 지정 (없으면 요청 처리 불가)
//root 또는 proxy_pass: 요청을 어디로 전달할지 지정 (정적 파일 또는 프록시)

//이 항목들이 없으면 Nginx가 요청을 어떻게 처리해야 할지 몰라서 에러가 발생하거나 요청을 무시할 수 있어요.