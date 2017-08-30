#include <cstdint>
#include <time.h>

enum class HttpRequestLineType {
	error,
	not_found,
	bad_param,

	get_user,
	get_loct,
	get_vist,
	get_loct_avg,
	get_user_vists,

	post_user,
	post_loct,
	post_vist,

	post_user_new,
	post_loct_new,
	post_vist_new,
};

struct HttpRequestLine {
	HttpRequestLineType type;

	uint32_t    id;
	time_t      from_date;
	time_t      to_date;
	const char *country;
	uint32_t    to_distance;
	int16_t     from_age;
	int16_t     to_age;
	char        gender;     // 0, 'm', 'f'
	bool        keep_alive;
};

struct HttpRequest {
	HttpRequestLine *req;
	char   *post_data;
	size_t  post_data_length;

	char   *reply;
	size_t  reply_length;
	size_t  reply_length_max;
	int     status;
};

typedef void HttpHandlerFun(HttpRequest&);

void http_parse_requst_line(char *line, HttpRequestLine &res);

void run_http_server(int port, HttpHandlerFun f);

void http_hande_request(HttpRequest &req);
