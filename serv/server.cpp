#include "all.hpp"
#include <algorithm>
#include "bench.hpp"
#include "json-serialize.hpp"
#include "json.hpp"

#include <boost/lexical_cast.hpp>
#include <shared_mutex>
#include <algorithm>

#include "simple-web-server/server_http.hpp"

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef std::shared_ptr<HttpServer::Response> ResponsePtr;
typedef std::shared_ptr<HttpServer::Request> RequestPtr;

static uint32_t atou32(const std::string &str) {
	return atoll(str.c_str());
}

constexpr auto $200 = SimpleWeb::StatusCode::success_ok;
constexpr auto $400 = SimpleWeb::StatusCode::client_error_bad_request;
constexpr auto $404 = SimpleWeb::StatusCode::client_error_not_found;

std::shared_mutex mut;

template <class Data>
static void server_get(ResponsePtr response, RequestPtr request)
{
	uint32_t id = atou32(request->path_match[1]);
	mut.lock_shared();
	if (Data *data = All::get<Data>(id)) {
		auto ret = json_serialize(*data);
		mut.unlock_shared();
		response->write($200, ret);
	} else {
		mut.unlock_shared();
		response->write($404, "Not found");
	}
}

template <class Data>
static void server_new(ResponsePtr response, RequestPtr request)
{
	auto content = request->content.string();
	Data data;
	typename Data::Mask mask;
	if (!json_parse_single(content, data, mask) || !mask.is_full()) {
		response->write($400, "Bad param");
		return;
	}
	mut.lock();
	bool ok = All::add(data);
	mut.unlock();
	if (ok)
		response->write($200, "{}");
	else
		response->write($400, "Bad param");
}

template <class Data>
static void server_update(ResponsePtr response, RequestPtr request)
{
	uint32_t id = atou32(request->path_match[1]);

	auto content = request->content.string();
	Data data;
	typename Data::Mask mask;
	if (!json_parse_single(content, data, mask) || mask.id) {
		response->write($400, "Bad param");
		return;
	}
	data.id = id;
	mut.lock();
	bool ok = All::update(data, mask);
	mut.unlock();
	if (ok)
		response->write($200, "{}");
	else
		response->write($404, "Not found");
}

void start_server(uint16_t port) {
	HttpServer server;
	server.config.port = port;
	server.config.thread_pool_size = 4;

	server.resource["^/users/([0-9]+)$"    ]["GET"] = server_get<User>;
	server.resource["^/locations/([0-9]+)$"]["GET"] = server_get<Loct>;
	server.resource["^/visits/([0-9]+)$"   ]["GET"] = server_get<Vist>;

	server.resource["^/users/new$"    ]["POST"] = server_new<User>;
	server.resource["^/locations/new$"]["POST"] = server_new<Loct>;
	server.resource["^/visits/new$"   ]["POST"] = server_new<Vist>;

	server.resource["^/users/([0-9]+)$"    ]["POST"] = server_update<User>;
	server.resource["^/locations/([0-9]+)$"]["POST"] = server_update<Loct>;
	server.resource["^/visits/([0-9]+)$"   ]["POST"] = server_update<Vist>;

	server.resource["^/users/([0-9]+)/visits$"]["GET"] =
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

			mut.lock_shared();
			bool have_user = All::get<User>(id) != nullptr;
			mut.unlock_shared();
			if (!have_user) {
				response->write($404, "Not found");
				return;
			}

			boost::optional<time_t> from_date;
			boost::optional<time_t> to_date;
			boost::optional<std::string> country;
			boost::optional<uint32_t> to_distance;

			auto query = request->parse_query_string();

			try {
				auto it = query.find("fromDate");
				if (it != query.end())
					from_date = boost::lexical_cast<time_t>(it->second);

				it = query.find("toDate");
				if (it != query.end())
					to_date = boost::lexical_cast<time_t>(it->second);

				it = query.find("country");
				if (it != query.end())
					country = it->second;

				it = query.find("toDistance");
				if (it != query.end())
					to_distance = boost::lexical_cast<time_t>(it->second);
			} catch (boost::bad_lexical_cast) {
				response->write($400, "Bad param");
				return;
			}

			mut.lock_shared();
			static thread_local std::vector<VistData> out;
			All::get_vists(out, id, from_date, to_date, country, to_distance);
			mut.unlock_shared();
			response->write($200, json_serialize(out));
		};

	server.resource["^/locations/([0-9]+)/avg$"]["GET"] = 
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

			mut.lock_shared();
			bool have_loct = All::get_loct(id) != nullptr;
			mut.unlock_shared();
			if (!have_loct) {
				response->write($404, "Not found");
				return;
			}

			boost::optional<time_t>  from_date;
			boost::optional<time_t>  to_date;
			boost::optional<uint8_t> from_age;
			boost::optional<uint8_t> to_age;
			boost::optional<bool>    gender_is_male;

			auto query = request->parse_query_string();
			try {
				auto it = query.find("fromDate");
				if (it != query.end())
					from_date = boost::lexical_cast<time_t>(it->second);

				it = query.find("toDate");
				if (it != query.end())
					to_date = boost::lexical_cast<time_t>(it->second);

				it = query.find("fromAge");
				if (it != query.end())
					from_age = boost::lexical_cast<time_t>(it->second);

				it = query.find("toAge");
				if (it != query.end())
					to_age = boost::lexical_cast<time_t>(it->second);

				it = query.find("gender");
				if (it != query.end()) {
					if (it->second == "m")
						gender_is_male = { true };
					else if (it->second == "f")
						gender_is_male = { false };
					else
						throw boost::bad_lexical_cast();
				}
			} catch (boost::bad_lexical_cast) {
				response->write($400, "Bad param");
				return;
			}

			mut.lock_shared();
			auto avg = All::get_avg(
					id, from_date, to_date, from_age, to_age, gender_is_male);
			mut.unlock_shared();
			response->write($200, json_serialize(avg));
		};

	std::vector<int> ids;
	for (int i = 0; i < 500000 / 10; i++)
		ids.push_back(i);
	std::random_shuffle(ids.begin(), ids.end());

	server.resource["^/bench/avg$"]["GET"] =
		[&](ResponsePtr response, RequestPtr) {
			auto t0 = std::chrono::steady_clock::now();
			double res = 0;
			for (auto id : ids)
				res += All::get_avg(id, {}, {}, {}, {}, {});
			auto t1 = std::chrono::steady_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
			std::stringstream ooo;
			ooo << "count = " << ids.size() << "\n"
			       "res   = " << res << "\n"
			       "ms    = " << ms << "\n"
			       "ms/req= " << (ms*1.0/ids.size()) << "\n";
			response->write($200, ooo);
		};

	server.resource["^/bench/visits$"]["GET"] =
		[&](ResponsePtr response, RequestPtr) {
			auto t0 = std::chrono::steady_clock::now();
			double res = 0;
			for (auto id : ids) {
				static thread_local std::vector<VistData> out;
				All::get_vists(out, id, {}, {}, {}, {});
			}
			auto t1 = std::chrono::steady_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::microseconds>(t1-t0).count();
			std::stringstream ooo;
			ooo << "count = " << ids.size() << "\n"
			       "res   = " << res << "\n"
			       "ms    = " << ms << "\n"
			       "ms/req= " << (ms*1.0/ids.size()) << "\n";
			response->write($200, ooo);
		};

	server.default_resource["GET"] =
		[](ResponsePtr response, RequestPtr) {
			response->write($404, "Not found");
		};

	server.start();
}
