#include "all.hpp"
#include "bench.hpp"
#include "json-serialize.hpp"
#include "json.hpp"

#include <boost/lexical_cast.hpp>
#include <shared_mutex>

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

void start_server(uint16_t port) {
	HttpServer server;
	server.config.port = port;
    server.config.thread_pool_size = 4;

	server.resource["^/users/([0-9]+)$"]["GET"] =
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
            mut.lock_shared();
			if (User *user = All::get_user(id)) {
                auto ret = json_serialize(*user);
                mut.unlock_shared();
				response->write($200, ret);
            } else {
                mut.unlock_shared();
				response->write($404, "Not found");
            }
		};

	server.resource["^/locations/([0-9]+)$"]["GET"] =
		[](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
            mut.lock_shared();
			if (Location *location = All::get_location(id)) {
                auto ret = json_serialize(*location);
                mut.unlock_shared();
				response->write($200, ret);
            } else {
                mut.unlock_shared();
				response->write($404, "Not found");
            }
		};

	server.resource["^/visits/([0-9]+)$"]["GET"] =
		[](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
            mut.lock_shared();
			if (Visit *visit = All::get_visit(id)) {
                auto ret = json_serialize(*visit);
                mut.unlock_shared();
				response->write($200, ret);
            } else {
                mut.unlock_shared();
				response->write($404, "Not found");
            }
		};

	server.resource["^/users/([0-9]+)/visits$"]["GET"] =
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

            mut.lock_shared();
			bool have_user = All::get_user(id) != nullptr;
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
			static thread_local std::vector<VisitData> out;
            All::get_visits(out, id, from_date, to_date, country, to_distance);
            mut.unlock_shared();
			response->write($200, json_serialize(out));
		};

	server.resource["^/locations/([0-9]+)/avg$"]["GET"] = 
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

            mut.lock_shared();
			bool have_location = All::get_location(id) != nullptr;
            mut.unlock_shared();
			if (!have_location) {
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

	server.resource["^/info$"]["GET"] = 
		[&](ResponsePtr response, RequestPtr request) {
			std::string resp;
            /*
			#define $(X) resp += #X ": " + X.str() + "\n"
			$(b0); $(b1); $(b2); $(b3); $(b4);
			$(ba0); $(ba1); $(ba2);
			#undef $
            */
			response->write($200, resp);
		};

	server.resource["^/users/new$"]["POST"] =
		[&](ResponsePtr response, RequestPtr request) {
			auto content = request->content.string();
			User user;
			uint8_t mask;
			if (!parse_user(content, user, mask) || mask != (1<<6)-1) {
				response->write($400, "Bad param");
				return;
			}
			mut.lock();
			bool ok = All::add_user(user);
			mut.unlock();
			if (ok)
				response->write($200, "{}");
			else
				response->write($400, "Bad param");
		};

	server.resource["^/locations/new$"]["POST"] =
		[&](ResponsePtr response, RequestPtr request) {
			auto content = request->content.string();
			Location location;
			uint8_t mask;
			if (!parse_location(content, location, mask) || mask != (1<<5)-1) {
				response->write($400, "Bad param");
				return;
			}
			mut.lock();
			bool ok = All::add_location(location);
			mut.unlock();
			if (ok)
				response->write($200, "{}");
			else
				response->write($400, "Bad param");
		};

	server.resource["^/visits/new$"]["POST"] =
		[&](ResponsePtr response, RequestPtr request) {
			auto content = request->content.string();
			Visit visit;
			uint8_t mask;
			if (!parse_visit(content, visit, mask) || mask != (1<<5)-1) {
				response->write($400, "Bad param");
				return;
			}
			mut.lock();
			bool ok = All::add_visit(visit);
			mut.unlock();
			if (ok)
				response->write($200, "{}");
			else
				response->write($400, "Bad param");
		};

	server.resource["^/users/([0-9]+)$"]["POST"] =
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

			auto content = request->content.string();
			User user;
			uint8_t mask;
			if (!parse_user(content, user, mask) || mask & 1) {
				response->write($400, "Bad param");
				return;
			}
			user.id = id;
			mut.lock();
			bool ok = All::update_user(user, mask);
			mut.unlock();
			if (ok)
				response->write($200, "{}");
			else
				response->write($404, "Not found");
		};

	server.resource["^/locations/([0-9]+)$"]["POST"] =
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

			auto content = request->content.string();
			Location location;
			uint8_t mask;
			if (!parse_location(content, location, mask) || mask & 1) {
				response->write($400, "Bad param");
				return;
			}
			location.id = id;
			mut.lock();
			bool ok = All::update_location(location, mask);
			mut.unlock();
			if (ok)
				response->write($200, "{}");
			else
				response->write($404, "Not found");
		};

	server.resource["^/visits/([0-9]+)$"]["POST"] =
		[&](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);

			auto content = request->content.string();
			Visit visit;
			uint8_t mask;
			if (!parse_visit(content, visit, mask) || mask & 1) {
				response->write($400, "Bad param");
				return;
			}
			visit.id = id;
			mut.lock();
			bool ok = All::update_visit(visit, mask);
			mut.unlock();
			if (ok)
				response->write($200, "{}");
			else
				response->write($404, "Not found");
		};

	server.default_resource["GET"] =
		[](ResponsePtr response, RequestPtr) {
			response->write($404, "Not found");
		};

	server.start();
}
