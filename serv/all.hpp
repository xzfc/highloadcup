#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <memory>
#include <boost/optional.hpp>

#define _u(x) (x*4+1)

struct User {
	uint32_t id;
	char     email[_u(100)];
	char     first_name[_u(50)];
	char     last_name[_u(50)];
	bool     gender_is_male;
	time_t   birth_date;
};

struct Location {
	uint32_t    id;
	std::string place;
	char        country[_u(50)];
	char        city[_u(50)];
	uint32_t    distance;
};

struct Visit {
	uint32_t id;
	uint32_t location;
	uint32_t user;
	uint32_t visited_at;
	uint8_t  mark; // 0..5
};

struct VisitData {
	uint8_t     mark;
	time_t      visited_at;
	std::string place;
};

namespace All {
	bool add_user(const User &user);
	bool add_location(const Location &location);
	bool add_visit(const Visit &visit);

	bool update_user(const User &, uint8_t mask);
	bool update_location(const Location &, uint8_t mask);
	bool update_visit(const Visit &, uint8_t mask);

	User *get_user(uint32_t id);
	Location *get_location(uint32_t id);
	Visit *get_visit(uint32_t id);

	bool get_visits(
		std::vector<VisitData> &out,
		uint32_t id,
		boost::optional<time_t> from_date,
		boost::optional<time_t> to_date,
		boost::optional<std::string> country,
		boost::optional<uint32_t> to_distance
	);

	double get_avg(
		uint32_t id,
		boost::optional<time_t>  from_date,
		boost::optional<time_t>  to_date,
		boost::optional<uint8_t> from_age,
		boost::optional<uint8_t> to_age,
		boost::optional<bool>    gender_is_male
	);

	void optimize();
    void set_options(time_t now, bool full);
};
