#include "all.hpp"

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef ::std::size_t SizeType; }
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"

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

template <class Data>
void run_parser(std::vector<char> &file) {
	rapidjson::Reader reader;
	rapidjson::MemoryStream stream(file.data(), file.size());
	JsonHandler<Data> handler(false);
	reader.Parse(stream, handler);
}

template void run_parser<User>(std::vector<char> &);
template void run_parser<Loct>(std::vector<char> &);
template void run_parser<Vist>(std::vector<char> &);
