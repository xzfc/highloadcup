#include <cstdint>
#include <string>
#include <map>

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

struct All {
	std::map<uint32_t, User>     users;
	std::map<uint32_t, Location> locations;
	std::map<uint32_t, Visit>    visits;

	bool add_user(uint32_t id, const User &user);
	bool add_location(uint32_t id, const Location &location);
	bool add_visit(uint32_t id, const Visit &visit);
};

