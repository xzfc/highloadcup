#include "all.hpp"

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef ::std::size_t SizeType; }
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"

#include <iostream>
#include <shared_mutex>
#include <boost/filesystem.hpp>

#include "json.hpp"

extern std::shared_mutex mut; // server.cpp

#define $key(WHAT, STATE, BIT) \
	if (!strcmp(str, WHAT)) { \
		if (have & (1 << BIT)) \
			{ $err; return false; } \
		state = STATE; \
		have |= 1 << BIT; \
		return true; \
	}
#define $str(WHAT) \
	if (length >= sizeof WHAT) return false; /* TODO: check utf length */ \
	memcpy(WHAT, str, length); \
	WHAT[length] = 0; \
	break;
#define $err do { \
	std::cerr << __FILE__ << ":" << __LINE__ << ": json error\n"; \
	return false; \
	} while (0)
#define $err_(W) \
	std::cerr << __FILE__ << ":" << __LINE__ << ": " << W << "\n"


#define $HANDLER_NAME    UserHandler
#define $LOCAL_VAR       User user
#define $TOPLEVEL_FIELD  "users"
#define $ADD_NEW         All::add_user(user)
#define $UPDATE          All::update_user(user, have)
#define $KEY_NUMBER      6
#define $KEY_HANDLER \
	$key("id",         10, 0); \
	$key("email",      11, 1); \
	$key("first_name", 12, 2); \
	$key("last_name",  13, 3); \
	$key("gender",     14, 4); \
	$key("birth_date", 15, 5)
#define $INT_HANDLER \
	case 10: user.id = val;         break; \
	case 15: user.birth_date = val; break
#define $STRING_HANDLER \
	case 11: $str(user.email); \
	case 12: $str(user.first_name); \
	case 13: $str(user.last_name); \
	case 14: \
		 if (length != 1 || (*str != 'm' && *str != 'f')) \
			{ $err; return false; } \
		 user.gender_is_male = *str == 'm'; \
		 break
#include "json_tmpl.hpp"


#define $HANDLER_NAME    LocationsHandler
#define $LOCAL_VAR       Location location
#define $TOPLEVEL_FIELD  "locations"
#define $ADD_NEW         All::add_location(location)
#define $UPDATE          All::update_location(location, have)
#define $KEY_NUMBER      5
#define $KEY_HANDLER \
	$key("id",         10, 0); \
	$key("place",      11, 1); \
	$key("country",    12, 2); \
	$key("city",       13, 3); \
	$key("distance",   14, 4)
#define $INT_HANDLER \
	case 10: location.id = val;       break; \
	case 14: location.distance = val; break
#define $STRING_HANDLER \
	case 11: location.place = str; break; \
	case 12: $str(location.country); \
	case 13: $str(location.city)
#include "json_tmpl.hpp"


#define $HANDLER_NAME    VisitsHandler
#define $LOCAL_VAR       Visit visit
#define $TOPLEVEL_FIELD  "visits"
#define $ADD_NEW         All::add_visit(visit)
#define $UPDATE          All::update_visit(visit, have)
#define $KEY_NUMBER      5
#define $KEY_HANDLER \
	$key("id",         10, 0); \
	$key("location",   11, 1); \
	$key("user",       12, 2); \
	$key("visited_at", 13, 3); \
	$key("mark",       14, 4)
#define $INT_HANDLER \
	case 10: visit.id = val;          break; \
	case 11: visit.location = val;    break; \
	case 12: visit.user = val;        break; \
	case 13: visit.visited_at = val;  break; /* TODO: timestamp */ \
	case 14: visit.mark = val;        break  /* TODO: 0..5 */
#define $STRING_HANDLER
#include "json_tmpl.hpp"

bool starts_with(const std::string &str, const char *prefix)
{
	return str.compare(0, strlen(prefix), prefix) == 0;
}

template<class T>
bool starts_with(const T& input, const T& match) {
    return input.size() >= match.size() &&
	    std::equal(match.begin(), match.end(), input.begin());
}

void parse(const char *dir) {
	char buffer[2048];
	rapidjson::Reader reader;
	FILE *fp;

	for (auto &it: boost::filesystem::directory_iterator(dir)) {
		auto fname = it.path().filename().string();
		fp = fopen(it.path().string().c_str(), "r");
		rapidjson::FileReadStream stream(fp, buffer, sizeof buffer);
		if (starts_with(fname, "users_")) {
			UserHandler handler(false);
			reader.Parse(stream, handler);
		}
		if (starts_with(fname, "locations_")) {
			LocationsHandler handler(false);
			reader.Parse(stream, handler);
		}
		if (starts_with(fname, "visits_")) {
			VisitsHandler handler(false);
			reader.Parse(stream, handler);
		}
		fclose(fp);
	}
}

bool parse_user(const std::string &json, User &user, uint8_t &mask) {
	rapidjson::Reader reader;
	rapidjson::MemoryStream stream(json.c_str(), json.size());
	UserHandler handler(true);
	bool ok = reader.Parse(stream, handler);
	if (ok) {
		user = handler.user;
		mask = handler.have;
	}
	return ok;
}

bool parse_location(const std::string &json, Location &location, uint8_t &mask) {
	rapidjson::Reader reader;
	rapidjson::MemoryStream stream(json.c_str(), json.size());
	LocationsHandler handler(true);
	bool ok = reader.Parse(stream, handler);
	if (ok) {
		location = handler.location;
		mask = handler.have;
	}
	return ok;
}

bool parse_visit(const std::string &json, Visit &visit, uint8_t &mask) {
	rapidjson::Reader reader;
	rapidjson::MemoryStream stream(json.c_str(), json.size());
	VisitsHandler handler(true);
	bool ok = reader.Parse(stream, handler);
	if (ok) {
		visit = handler.visit;
		mask = handler.have;
	}
	return ok;
}