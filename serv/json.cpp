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

#define $key(WHAT, STATE, FIELD) \
	if (!strcmp(str, WHAT)) { \
		if (mask.FIELD) \
			{ $err; return false; } \
		state = STATE; \
		mask.FIELD = true; \
		return true; \
	}
#define $str(WHAT) \
	if (length >= sizeof WHAT) return false; /* TODO: check utf length */ \
	memcpy(WHAT, str, length); \
	WHAT[length] = 0; \
	break;
#define $err return false
template <class Data> struct JsonHandler;

#define $DATA            User
#define $TOPLEVEL_FIELD  "users"
#define $KEY_HANDLER \
	$key("id",         10, id);             \
	$key("email",      11, email);          \
	$key("first_name", 12, first_name);     \
	$key("last_name",  13, last_name);      \
	$key("gender",     14, gender_is_male); \
	$key("birth_date", 15, birth_date)
#define $INT_HANDLER \
	case 10: data.id = val;         break; \
	case 15: data.birth_date = val; break
#define $STRING_HANDLER \
	case 11: $str(data.email); \
	case 12: $str(data.first_name); \
	case 13: $str(data.last_name); \
	case 14: \
		 if (length != 1 || (*str != 'm' && *str != 'f')) \
			{ $err; return false; } \
		 data.gender_is_male = *str == 'm'; \
		 break
#include "json_tmpl.hpp"


#define $DATA            Loct
#define $TOPLEVEL_FIELD  "locations"
#define $KEY_HANDLER \
	$key("id",         10, id);      \
	$key("place",      11, place);   \
	$key("country",    12, country); \
	$key("city",       13, city);    \
	$key("distance",   14, distance)
#define $INT_HANDLER \
	case 10: data.id = val;       break; \
	case 14: data.distance = val; break
#define $STRING_HANDLER \
	case 11: data.place = str; break; \
	case 12: $str(data.country); \
	case 13: $str(data.city)
#include "json_tmpl.hpp"


#define $DATA            Vist
#define $TOPLEVEL_FIELD  "visits"
#define $KEY_HANDLER \
	$key("id",         10, id);      \
	$key("location",   11, loct);    \
	$key("user",       12, user);    \
	$key("visited_at", 13, visited); \
	$key("mark",       14, mark)
#define $INT_HANDLER \
	case 10: data.id = val;      break; \
	case 11: data.loct = val;    break; \
	case 12: data.user = val;    break; \
	case 13: data.visited = val; break; /* TODO: timestamp */ \
	case 14: data.mark = val;    break  /* TODO: 0..5 */
#define $STRING_HANDLER
#include "json_tmpl.hpp"

bool starts_with(const std::string &str, const char *prefix)
{
	return str.compare(0, strlen(prefix), prefix) == 0;
}

template <class Data>
static void run_parser(const boost::filesystem::path &path, const char *prefix) {
	if (!starts_with(path.filename().string(), prefix))
		return;

	char buffer[2048];
	rapidjson::Reader reader;
	FILE *fp = fopen(path.string().c_str(), "r");
	rapidjson::FileReadStream stream(fp, buffer, sizeof buffer);
	JsonHandler<Data> handler(false);
	reader.Parse(stream, handler);
	fclose(fp);
}

void parse(const char *dir) {
	for (auto &it: boost::filesystem::directory_iterator(dir)) {
		auto path = it.path();
		run_parser<User>(path, "users_");
		run_parser<Loct>(path, "locations_");
		run_parser<Vist>(path, "visits_");
	}
}

template <class Data>
static bool json_parse_single_tmpl(const std::string &json, Data &data, typename Data::Mask &mask)
{
	rapidjson::Reader reader;
	rapidjson::MemoryStream stream(json.c_str(), json.size());
	JsonHandler<Data> handler(true);
	bool ok = reader.Parse(stream, handler);
	if (ok) {
		data = handler.data;
		mask = handler.mask;
	}
	return ok;
}

bool json_parse_single(const std::string &json, User &data, UserMask &mask) {
	return json_parse_single_tmpl(json, data, mask);
}

bool json_parse_single(const std::string &json, Loct &data, LoctMask &mask) {
	return json_parse_single_tmpl(json, data, mask);
}

bool json_parse_single(const std::string &json, Vist &data, VistMask &mask) {
	return json_parse_single_tmpl(json, data, mask);
}
