#include <iostream>


#include <cstdint>
#include <string>
#include <map>

#define _u(x) (x*4+1)

struct User {
	// uint32_t id;
	char     email[_u(100)];
	char     first_name[_u(50)];
	char     last_name[_u(50)];
	bool     gender_is_male;
	time_t   birth_date;
};

struct Location {
	// uint32_t    id;
	std::string place;
	char        country[_u(50)];
	char        city[_u(50)];
	uint32_t    distance;
};

struct Visit {
	// uint32_t id;
	uint32_t location;
	uint32_t user;
	uint32_t visited_at;
	uint8_t  mark; // 0..5
};

struct All {
	std::map<uint32_t, User>     users;
	std::map<uint32_t, Location> locations;
	std::map<uint32_t, Visit>    visits;

	bool add_user(uint32_t id, const User &user);

	bool add_location(
		uint32_t           id,
		const std::string &place,
		const std::string &country,
		const std::string &city,
		uint32_t           distance
	);
};

bool All::add_user(uint32_t id, const User &user)
{
	users[id] = user;
	return true; // TODO
}

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef ::std::size_t SizeType; }
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"

#include <fstream>

#define PATH_PREFIX "/home/alice/dist/github.com/sat2707/hlcupdocs/data/FULL/data/"

#include <iostream>

#define $key(WHAT, STATE, BIT) \
	if (!strcmp(str, WHAT)) { state = STATE; have |= 1 << BIT; return true; }
#define $str(WHAT) \
	if (length >= sizeof user.WHAT) return false; \
	memcpy(user.WHAT, str, length); break;

struct MyHandler : rapidjson::BaseReaderHandler<> {
	All &all;
	int state = 0;
	uint32_t id;
	User user;
	uint8_t have;

	MyHandler(All &all) : all{all} { }

	bool StartObject() {
		switch (state) {
		case 0: state = 1; return true;
		case 3: state = 4; have = 0; return true;
		default: return false;
		}
	}
	bool EndObject(rapidjson::SizeType memberCount) {
		switch (state) {
		case 2: state = 0; return true;
		case 4:
			if (have != (1<<6) - 1) return false;
			state = 3;
			all.add_user(id, user);
			return true;
		default: return false;
		}
	}
	bool StartArray() {
		if (state != 2)
			return false;
		state = 3;
		return true;
	}
	bool EndArray(size_t) {
		if (state != 3)
			return false;
		state = 2;
		return true;
	}
	bool Key(const char* str, rapidjson::SizeType length, bool copy) {
		switch (state) {
		case 1:
			if (strncmp(str, "users", length)) return false;
			state = 2;
			return true;
		case 4:
			$key("id",         10, 0);
			$key("email",      11, 1);
			$key("first_name", 12, 2);
			$key("last_name",  13, 3);
			$key("gender",     14, 4);
			$key("birth_date", 15, 5);
		default:
			return false;
		}
	}
	bool Int(int val) {
		switch (state) {
		case 10: id = val; break;
		case 15: user.birth_date = val; break;
		default: return false;
		}
		state = 4;
		return true;
	}
	bool Uint(unsigned val) {
		switch (state) {
		case 10: id = val; break;
		case 15: user.birth_date = val; break;
		default: return false;
		}
		state = 4;
		return true;
	}
	bool String(const char* str, rapidjson::SizeType length, bool copy) {
		switch (state) {
		case 11: $str(email);
		case 12: $str(first_name);
		case 13: $str(last_name);
		case 14:
			 if (length != 1 || (*str != 'm' && *str != 'f')) return false;
			 user.gender_is_male = *str == 'm';
			 break;
		default: return false;
		}
		state = 4; return true;
	}
};

int main()
{
	All all;

	char buffer[2048];
	MyHandler handler(all);
	FILE *fp = fopen(PATH_PREFIX "users_1.json", "r");
	rapidjson::Reader reader;
	rapidjson::FileReadStream stream(fp, buffer, sizeof buffer);
	reader.Parse(stream, handler);
	fclose(fp);
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
