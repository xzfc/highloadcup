#pragma once

#include <cstdint>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <boost/optional.hpp>

#define _u(x) (x*4+1)

struct User {
	// uint32_t id;
	char     email[_u(100)];
	char     first_name[_u(50)];
	char     last_name[_u(50)];
	bool     gender_is_male;
	time_t   birth_date;
};

struct Location {
	// uint32_t    id;
	std::string place;
	char        country[_u(50)];
	char        city[_u(50)];
	uint32_t    distance;
};

struct Visit {
	// uint32_t id;
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

struct UserVisit {
	uint32_t user;
	time_t   visited_at;
	uint32_t visit;
};

static bool operator <(const UserVisit &a, const UserVisit &b) {
	if (a.user < b.user) return true;
	if (a.user > b.user) return false;

	if (a.visited_at < b.visited_at) return true;
	if (a.visited_at > b.visited_at) return false;

	if (a.visit < b.visit) return true;
	if (a.visit > b.visit) return false;

	return false;
}

struct All {
	std::map<uint32_t, User>     users;
	std::map<uint32_t, Location> locations;
	std::map<uint32_t, Visit>    visits;

	std::set<UserVisit> user_visits;

	bool add_user(uint32_t id, const User &user);
	bool add_location(uint32_t id, const Location &location);
	bool add_visit(uint32_t id, const Visit &visit);

	bool update_user(...); // TODO
	bool update_location(...); // TODO
	bool update_visit(...); // TODO

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
};
