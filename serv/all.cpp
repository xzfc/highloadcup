#include "all.hpp"
#include "yearsdiff.hpp"
#include <cstring>
#include <boost/intrusive/avl_set.hpp>
#include <iostream>

namespace bi = ::boost::intrusive;

struct UserP : User {
	UserP(const User &user) : User(user) { }
	bi::avl_set_member_hook<> hook_id;
};

struct LocationP : Location {
	LocationP(const Location &location) : Location(location) {}
	bi::avl_set_member_hook<> hook_id;
};

struct VisitP : Visit {
	VisitP(const Visit &visit) : Visit(visit) {}
	bi::avl_set_member_hook<> hook_id, hook_location, hook_user_visited;

	LocationP *location_ptr = 0;
	LocationP *get_location();

	UserP *user_ptr = 0;
	UserP *get_user();
};

struct KeyId {
	typedef uint32_t type;
	template <class T>
	uint32_t operator()(const T &a) const {
		return a.id;
	}
};

struct CompareVisitUserVisited {
	typedef Visit type;
	const Visit &operator()(const VisitP &v) const {
		return v;
	}
	bool operator()(const Visit &a, const Visit &b) const {
		if (a.user < b.user) return true;
		if (a.user > b.user) return false;

		if (a.visited_at < b.visited_at) return true;
		if (a.visited_at > b.visited_at) return false;

		if (a.id < b.id) return true;
		if (a.id > b.id) return false;

		return false;
	}
};

struct CompareVisitLocation {
	typedef Visit type;
	const Visit &operator()(const VisitP &v) const {
		return v;
	}
	bool operator()(const Visit &a, const Visit &b) const {
		if (a.location < b.location) return true;
		if (a.location > b.location) return false;

		if (a.visited_at < b.visited_at) return true;
		if (a.visited_at > b.visited_at) return false;

		if (a.id < b.id) return true;
		if (a.id > b.id) return false;

		return false;
	}
};

typedef bi::avl_set<
	UserP,
	bi::member_hook<UserP, bi::avl_set_member_hook<>, &UserP::hook_id>,
	bi::key_of_value<KeyId>
> UserTreeId;

typedef bi::avl_set<
	LocationP,
	bi::member_hook<LocationP, bi::avl_set_member_hook<>, &LocationP::hook_id>,
	bi::key_of_value<KeyId>
> LocationTreeId;

typedef bi::avl_set<
	VisitP,
	bi::member_hook<VisitP, bi::avl_set_member_hook<>, &VisitP::hook_id>,
	bi::key_of_value<KeyId>
> VisitTreeId;

typedef bi::avl_set<
	VisitP,
	bi::member_hook<VisitP, bi::avl_set_member_hook<>, &VisitP::hook_location>,
	bi::key_of_value<CompareVisitLocation>,
	bi::compare<CompareVisitLocation>
> VisitTreeLocation;

typedef bi::avl_set<
	VisitP,
	bi::member_hook<VisitP, bi::avl_set_member_hook<>, &VisitP::hook_user_visited>,
	bi::key_of_value<CompareVisitUserVisited>,
	bi::compare<CompareVisitUserVisited>
> VisitTreeUserVisited;

struct AllP {
	UserTreeId            tree_user_id;
	LocationTreeId        tree_location_id;
	VisitTreeId           tree_visit_id;

	VisitTreeLocation     tree_visit_location;
	VisitTreeUserVisited  tree_visit_user_visited;

	YearDiffer year_differ;
};

static AllP allp;


LocationP *VisitP::get_location() {
	if (location_ptr != nullptr)
		return location_ptr;
	auto f = allp.tree_location_id.find(location);
	location_ptr = f != allp.tree_location_id.end() ? &*f : nullptr;
	if (location_ptr == nullptr)
		std::cout << "Wooo\n";
	return location_ptr;
}

UserP *VisitP::get_user() {
	if (user_ptr != nullptr)
		return user_ptr;
	auto f = allp.tree_user_id.find(user);
	user_ptr = f != allp.tree_user_id.end() ? &*f : nullptr;
	if (user_ptr == nullptr)
		std::cout << "Wooo\n";
	return user_ptr;
}


bool All::add_user(const User &user) {
	auto it = allp.tree_user_id.find(user.id);
	if (it != allp.tree_user_id.end())
		return false;
	auto e = new UserP(user);
	allp.tree_user_id.insert(it, *e);
	return true;
}

bool All::add_location(const Location &location) {
	auto it = allp.tree_location_id.find(location.id);
	if (it != allp.tree_location_id.end())
		return false;
	auto e = new LocationP(location);
	allp.tree_location_id.insert(it, *e);
	return true;
}

bool All::add_visit(const Visit &visit) {
	auto it = allp.tree_visit_id.find(visit.id);
	if (it != allp.tree_visit_id.end())
		return false;
	auto e = new VisitP(visit);
	allp.tree_visit_id.insert(it, *e);
	allp.tree_visit_location.insert(*e);
	allp.tree_visit_user_visited.insert(*e);
	return true;
}

bool All::update_user(const User &user, uint8_t mask) {
	auto it = allp.tree_user_id.find(user.id);
	if (it == allp.tree_user_id.end())
		return false;
	if (mask & (1<<1)) std::strcpy(it->email,           user.email);
	if (mask & (1<<2)) std::strcpy(it->first_name,      user.first_name);
	if (mask & (1<<3)) std::strcpy(it->last_name,       user.last_name);
	if (mask & (1<<4))             it->gender_is_male = user.gender_is_male;
	if (mask & (1<<5))             it->birth_date     = user.birth_date;
	return true;
}

bool All::update_location(const Location &location, uint8_t mask) {
	auto it = allp.tree_location_id.find(location.id);
	if (it == allp.tree_location_id.end())
		return false;
	if (mask & (1<<1))             it->place    = location.place;
	if (mask & (1<<2)) std::strcpy(it->country,   location.country);
	if (mask & (1<<3)) std::strcpy(it->city,      location.city);
	if (mask & (1<<4))             it->distance = location.distance;
	return true;
}

bool All::update_visit(const Visit &visit, uint8_t mask) {
	auto it = allp.tree_visit_id.find(visit.id);
	if (it == allp.tree_visit_id.end())
		return false;

	if (mask & (1<<1 | 1<<3))
		allp.tree_visit_location.erase(*it);

	if (mask & (1<<2 | 1<<3))
		allp.tree_visit_user_visited.erase(*it);

	if (mask & (1<<1)) { it->location   = visit.location; it->location_ptr = 0; }
	if (mask & (1<<2)) { it->user       = visit.user; it->user_ptr = 0; }
	if (mask & (1<<3)) it->visited_at = visit.visited_at;
	if (mask & (1<<4)) it->mark       = visit.mark;

	if (mask & (1<<1 | 1<<3))
		allp.tree_visit_location.insert(*it);

	if (mask & (1<<2 | 1<<3))
		allp.tree_visit_user_visited.insert(*it);

	return true;
}

User *All::get_user(uint32_t id) {
	auto f = allp.tree_user_id.find(id);
	return f != allp.tree_user_id.end() ? &*f : nullptr;
}

Location *All::get_location(uint32_t id) {
	auto f = allp.tree_location_id.find(id);
	return f != allp.tree_location_id.end() ? &*f : nullptr;
}

Visit *All::get_visit(uint32_t id) {
	auto f = allp.tree_visit_id.find(id);
	return f != allp.tree_visit_id.end() ? &*f : nullptr;
}

bool All::get_visits(
	std::vector<VisitData> &out,
	uint32_t id,
	boost::optional<time_t> from_date,
	boost::optional<time_t> to_date,
	boost::optional<std::string> country,
	boost::optional<uint32_t> to_distance
)
{
	out.clear();
	// if (!allp.tree_user_id.find(id))
	//	return false;

	if (!from_date) from_date = { std::numeric_limits<time_t>::min() };
	if (!to_date)   to_date   = { std::numeric_limits<time_t>::max() };

	if (*from_date >= *to_date)
		return true;

	Visit first;
	first.user = id;
	first.visited_at = *from_date;
	first.id = 0;

	Visit last;
	last.user = id;
	last.visited_at = *to_date;
	last.id = 0xFFFFFFFFu;

	auto it = allp.tree_visit_user_visited.lower_bound(first);
	if (it == allp.tree_visit_user_visited.end())
		return true;

	auto end = allp.tree_visit_user_visited.upper_bound(last);

	for (; it != end; it++) {
		auto &visit = *it;
		Location *location = visit.get_location();
		if (location == nullptr)
			continue;

		if (country     && location->country != *country ||
		    to_distance && location->distance >= *to_distance)
			continue;

		out.push_back({
			visit.mark,
			visit.visited_at,
			location->place
		});
	}
	return true;
}

double All::get_avg(
	uint32_t id,
	boost::optional<time_t>  from_date,
	boost::optional<time_t>  to_date,
	boost::optional<uint8_t> from_age,
	boost::optional<uint8_t> to_age,
	boost::optional<bool>    gender_is_male
) {
	if (!from_date) from_date = { std::numeric_limits<time_t>::min() };
	if (!to_date)   to_date   = { std::numeric_limits<time_t>::max() };

	time_t birth_from, birth_to;
	birth_from = to_age
		? allp.year_differ.jaja(*to_age)
		: std::numeric_limits<time_t>::min();

	birth_to = from_age
		? allp.year_differ.jaja(*from_age)
		: std::numeric_limits<time_t>::max();

	if (*from_date >= *to_date)
		return 0;

	Visit first;
	first.location = id;
	first.visited_at = *from_date;
	first.id = 0;

	Visit last;
	last.location = id;
	last.visited_at = *to_date;
	last.id = 0xFFFFFFFFu;

	auto it = allp.tree_visit_location.lower_bound(first);
	if (it == allp.tree_visit_location.end())
		return 0;

	auto end = allp.tree_visit_location.upper_bound(last);

	unsigned count = 0, sum = 0;
	for (; it != end; it++) {
		auto &visit = *it;
		UserP *user = visit.get_user();
		if (user == nullptr)
			continue;

		if (user->birth_date <= birth_from ||
		    user->birth_date >= birth_to   ||
		    gender_is_male && user->gender_is_male != *gender_is_male)
			continue;

		count++;
		sum += visit.mark;
	}

	if (count == 0)
		return 0;
	return (double)sum / count;
}

void All::optimize() {
	for (auto &visit : allp.tree_visit_id) {
		visit.get_location();
		visit.get_user();
	}
}

void All::set_options(time_t now, bool full) {
	allp.year_differ.set_now(now);
}