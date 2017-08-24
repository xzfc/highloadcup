#include "all.hpp"

bool All::add_user(uint32_t id, const User &user)
{
	users[id] = user;
	return true; // TODO
}

bool All::add_location(uint32_t id, const Location &location)
{
	locations[id] = location;
	return true; // TODO
}

bool All::add_visit(uint32_t id, const Visit &visit)
{
	visits[id] = visit;
	return true; // TODO
}

void parse(All &all);

#include <iostream>

int main()
{
	All all;
	parse(all);

	std::cout << all.users.size() << "\n";
	std::cout << all.locations.size() << "\n";
	std::cout << all.visits.size() << "\n";
}

/*

#include "simple-web-server/server_http.hpp"

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef std::shared_ptr<HttpServer::Response> ResponsePtr;
typedef std::shared_ptr<HttpServer::Request> RequestPtr;

static uint32_t atou32(const std::string &str) {
	return atoll(str.c_str());
}

void do_resp(HttpServer::Response &response, int code, const std::string &data) {
	switch (code) {
	case 200: response << "HTTP/1.1 200 OK\r\n";                    break;
	case 400: response << "HTTP/1.1 400 Bad Request\r\n";           break;
	case 404: response << "HTTP/1.1 404 Not Found\r\n";             break;
	case 500: response << "HTTP/1.1 500 Internal Server Error\r\n"; break;
	}
	response << "Content-Length: " << data.length() << "\r\n\r\n" << data;
}

int main() {
	HttpServer server;
	server.config.port = 1234;

	server.resource["^/users/([0-9]+)$"]["GET"] =
		[](ResponsePtr response, RequestPtr request) {
			std::string number = request->path_match[1];
			uint32_t id = atou32(request->path_match[1]);
			do_resp(*response, 404, "notfound");
		};

	server.default_resource["GET"] = [](ResponsePtr response, RequestPtr request) {
		std::string data = "what";
		*response << "HTTP/1.1 404 OK\r\nContent-Length: " << data.length() << "\r\n\r\n" << data;
	};

	std::cout << "Started\n";
	server.start();
}
*/
