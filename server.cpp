#include "all.hpp"
#include "json-serialize.hpp"

#include <boost/lexical_cast.hpp>

#include "simple-web-server/server_http.hpp"

typedef SimpleWeb::Server<SimpleWeb::HTTP> HttpServer;
typedef std::shared_ptr<HttpServer::Response> ResponsePtr;
typedef std::shared_ptr<HttpServer::Request> RequestPtr;

static uint32_t atou32(const std::string &str) {
	return atoll(str.c_str());
}

static void do_resp(
		HttpServer::Response &response,
		int code,
		const std::string &data)
{
	switch (code) {
	case 200: response << "HTTP/1.1 200 OK\r\n";                    break;
	case 400: response << "HTTP/1.1 400 Bad Request\r\n";           break;
	case 404: response << "HTTP/1.1 404 Not Found\r\n";             break;
	case 500: response << "HTTP/1.1 500 Internal Server Error\r\n"; break;
	}
	response << "Content-Length: " << data.length() << "\r\n\r\n" << data;
}

static boost::optional<std::string> query_get(
		const SimpleWeb::CaseInsensitiveMultimap &query,
		const char *key
	)
{
	auto it = query.find(key);
	if (it == query.end())
		return {};
	return it->second;
}

template <typename T, typename Func>
auto operator|(const boost::optional<T>& opt, Func&& func)
	-> boost::optional<decltype(func(*opt))>
{
	using OutType = boost::optional<decltype(func(*opt))>;
	return (opt) ? OutType(func(*opt)) : boost::none;
}



void start_server(All &all) {
	HttpServer server;
	server.config.port = 1234;

	server.resource["^/users/([0-9]+)$"]["GET"] =
		[&all](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
			if (User *user = all.get_user(id))
				do_resp(*response, 200, json_serialize(id, *user));
			else
				do_resp(*response, 404, "Not found");
		};

	server.resource["^/locations/([0-9]+)$"]["GET"] =
		[&all](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
			if (Location *location = all.get_location(id))
				do_resp(*response, 200, json_serialize(id, *location));
			else
				do_resp(*response, 404, "Not found");
		};

	server.resource["^/visits/([0-9]+)$"]["GET"] =
		[&all](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
			if (Visit *visit = all.get_visit(id))
				do_resp(*response, 200, json_serialize(id, *visit));
			else
				do_resp(*response, 404, "Not found");
		};

	server.resource["^/users/([0-9]+)/visits$"]["GET"] =
		[&all](ResponsePtr response, RequestPtr request) {
			uint32_t id = atou32(request->path_match[1]);
			User *user = all.get_user(id);
			if (user == nullptr) {
				do_resp(*response, 404, "Not found");
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
				do_resp(*response, 400, "Bad param");
				return;
			}

			static thread_local std::vector<VisitData> out;
			bool ok = all.get_visits(
					out, id, from_date, to_date, country, to_distance);
			if (ok)
				do_resp(*response, 200, json_serialize(out));
			else
				do_resp(*response, 404, "Not found");
		};

	server.default_resource["GET"] =
		[](ResponsePtr response, RequestPtr request) {
			do_resp(*response, 404, "Not found");
		};

	std::cout << "Started\n";
	server.start();
}
