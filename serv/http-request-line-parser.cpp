#include "http.hpp"

#include <boost/lexical_cast.hpp>
#include <limits>

static char *advance_u32(char *s, uint32_t &res) {
	if (*s < '0' || *s > '9')
		return NULL;

	res = 0;
	for (int n = 0; *s >= '0' && *s <= '9'; s++, n++) {
		res = res * 10 + (*s - '0');
		if (n == 8) return NULL; // too big number
	}
	return s;
}

/* Taken from Onion HTTP server library
 * Copyright (C) 2010-2016 David Moreno Montero and others
 * Licensed under Apache License Version 2.0.
 */
static void onion_unquote_inplace(char *str){
	char *r=str;
	char *w=str;
	char tmp[3]={0,0,0};
	while (*r){
		if (*r == '%'){
			r++;
			tmp[0]=*r++;
			tmp[1]=*r;
			*w=strtol(tmp, (char **)NULL, 16);
		}
		else if (*r=='+'){
			*w=' ';
		}
		else{
			*w=*r;
		}
		r++;
		w++;
	}
	*w='\0';
}

template <class F>
static bool advance_parameters(char **line, const F &fun) {
	char *s = *line;
	if (*s == ' ') { *s = ' '; return true; }

	char *param = ++s, *value = nullptr;
	int state = 0;
	for (; ; s++) {
		if (*s == 0) return false;
		if (state == 0 && *s == '=') {
			*s = 0;
			value = s + 1;
			state = 1;
			continue;
		}
		if (*s == '&') {
			*s = 0;
			if (value) onion_unquote_inplace(value);
			if (!fun(param, value)) return false;
			state = 0;
			param = s + 1;
			value = nullptr;
			continue;
		}
		if (*s == ' ') {
			*s = 0;
			if (value) onion_unquote_inplace(value);
			if (!fun(param, value)) return false;
			*line = s + 1;
			return true;
		}
		if (state == 1 && *s == ' ') {
			*s = 0;
			if (value) onion_unquote_inplace(value);
			if (!fun(param, value)) return false;
			*line = s + 1;
			return true;
		}
	}

	return true;
}

template <class T> static void set_min(T &val) { val = std::numeric_limits<T>::min(); }
template <class T> static void set_max(T &val) { val = std::numeric_limits<T>::max(); }

template <class T>
static bool parse_param(const char *s, T &out) {
	return boost::conversion::try_lexical_convert(s, out);
}

template <>
bool parse_param<const char *>(const char *s, const char *&out) {
	out = s;
	return true;
}

template <>
bool parse_param<char>(const char *s, char &out) {
	if (s[0] == 'm' && s[1] == 0) { out = 'm'; return true; }
	if (s[0] == 'f' && s[1] == 0) { out = 'f'; return true; }
	return false;
}

void http_parse_requst_line(char *line, HttpRequestLine &res) {
	char *advance_tmp;
	bool rc;

#define $advance(STR) (memcmp(line, STR, sizeof STR - 1) == 0 && (line += sizeof STR - 1, true))
#define $advance_u32(NUM) ((advance_tmp = advance_u32(line, NUM)) && (line = advance_tmp, true))

	#define $t HttpRequestLineType
	if ($advance("GET /")) {
		if (false);
		else if ($advance("users/"    )) res.type = $t::get_user;
		else if ($advance("locations/")) res.type = $t::get_loct;
		else if ($advance("visits/"   )) res.type = $t::get_vist;
		else { res.type = $t::not_found; goto end; }

		if (!$advance_u32(res.id)) {
			res.type = $t::not_found;
			goto end;
		}

		if (false);
		else if (res.type == $t::get_loct && $advance("/avg"))
			res.type = $t::get_loct_avg;
		else if (res.type == $t::get_user && $advance("/visits"))
			res.type = $t::get_user_vists;
	} else if ($advance("POST /")) {
		if (false);
		else if ($advance("users/"    )) res.type = $t::post_user;
		else if ($advance("locations/")) res.type = $t::post_loct;
		else if ($advance("visits/"   )) res.type = $t::post_vist;
		else { res.type = $t::not_found; goto end; }

		if ($advance("new")) {
			switch (res.type) {
			case $t::post_user: res.type = $t::post_user_new; break;
			case $t::post_loct: res.type = $t::post_loct_new; break;
			case $t::post_vist: res.type = $t::post_vist_new; break;
			default:; // never happens
			}
		} else if ($advance_u32(res.id)) {
			// ok
		} else {
			res.type = $t::not_found;
			goto end;
		}
	} else {
		res.type = $t::error;
		// FIXME
		goto end;
	}

	set_min(res.from_date);
	set_max(res.to_date);
	res.country = nullptr;
	set_max(res.to_distance);
	set_min(res.from_age);
	set_max(res.to_age);
	res.gender = 0;

	#define $param(P) \
		if (strcmp(param, p_##P) == 0) { \
			if (value != nullptr && parse_param(value, res.P)) \
				return true; \
			res.type = $t::bad_param; \
			return false; \
		}
	static char
		p_from_date[]   = "fromDate",
		p_to_date[]     = "toDate",
		p_from_age[]    = "fromAge",
		p_to_age[]      = "toAge",
		p_to_distance[] = "toDistance",
		p_country[]     = "country",
		p_gender[]      = "gender";

	rc = advance_parameters(&line,
		[&](const char *param, const char *value) {
			switch (res.type) {
			case $t::get_user_vists:
				$param(from_date);
				$param(to_date);
				$param(country);
				$param(to_distance);
				break;
			case $t::get_loct_avg:
				$param(from_date);
				$param(to_date);
				$param(from_age);
				$param(to_age);
				$param(gender);
				break;
			default: break;
			}
			return true;
		});
	if (!rc) { res.type = $t::bad_param; goto end;; }

end:;
#ifdef KEEPALIVE
	while (*line != 0)
		line++;
	res.keep_alive = line[-1] != '0'; // check for HTTP 1/.0
#endif
}
