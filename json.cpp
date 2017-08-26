#include "all.hpp"

#define RAPIDJSON_NO_SIZETYPEDEFINE
namespace rapidjson { typedef ::std::size_t SizeType; }
#include "rapidjson/reader.h"
#include "rapidjson/filereadstream.h"

#include <boost/asio/io_service.hpp>
#include <boost/bind.hpp>
#include <boost/filesystem.hpp>
#include <boost/thread/thread.hpp>
#include <iostream>
#include <mutex>

static std::mutex mut;

#define $key(WHAT, STATE, BIT) \
	if (!strcmp(str, WHAT)) { state = STATE; have |= 1 << BIT; return true; }
#define $str(WHAT) \
	if (length >= sizeof WHAT) return false; /* TODO: check utf length */ \
	memcpy(WHAT, str, length); \
	WHAT[length] = 0; \
	break;
#define $err \
	std::cerr << __FILE__ << ":" << __LINE__ << ": json error\n"
#define $err_(W) \
	std::cerr << __FILE__ << ":" << __LINE__ << ": " << W << "\n"


#define $HANDLER_NAME    UserHandler
#define $LOCAL_VAR       User user
#define $TOPLEVEL_FIELD  "users"
#define $ADD_NEW         mut.lock(); All::add_user(user); mut.unlock()
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
#define $ADD_NEW         mut.lock(); All::add_location(location); mut.unlock()
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
	case 11: location.place = str; \
	case 12: $str(location.country); \
	case 13: $str(location.city)
#include "json_tmpl.hpp"


#define $HANDLER_NAME    VisitsHandler
#define $LOCAL_VAR       Visit visit
#define $TOPLEVEL_FIELD  "visits"
#define $ADD_NEW         mut.lock(); All::add_visit(visit); mut.unlock()
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

struct ThreadPool {
	typedef std::unique_ptr<boost::asio::io_service::work> asio_worker;
	ThreadPool(size_t threads) :service(), working(new asio_worker::element_type(service)) {
		for ( std::size_t i = 0; i < threads; ++i ) {
			auto worker = boost::bind(&boost::asio::io_service::run, &(this->service));
			g.add_thread(new boost::thread(worker));
		}
	}

	template<class F>
		void enqueue(F f){
			service.post(f);
		}

	~ThreadPool() {
		working.reset(); //allow run() to exit
		g.join_all();
		service.stop();
	}

	private:
	boost::asio::io_service service; //< the io_service we are wrapping
	asio_worker working;
	boost::thread_group g; //< need to keep track of threads so we can join them
};

void parse(const char *dir) {
	ThreadPool pool(4);

	for (auto &it: boost::filesystem::directory_iterator(dir)) {
		const auto path = it.path();
		pool.enqueue([path] {
			auto fname = path.filename().string();
			std::cout << fname << "\n";
			FILE *fp;
			char buffer[2048];
			rapidjson::Reader reader;

			fp = fopen(path.string().c_str(), "r");
			rapidjson::FileReadStream stream(fp, buffer, sizeof buffer);
			if (starts_with(fname, "users_")) {
				UserHandler handler;
				reader.Parse(stream, handler);
			}
			if (starts_with(fname, "locations_")) {
				LocationsHandler handler;
				reader.Parse(stream, handler);
			}
			if (starts_with(fname, "visits_")) {
				VisitsHandler handler;
				reader.Parse(stream, handler);
			}
			fclose(fp);
		});
	}
}
