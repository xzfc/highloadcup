#include "all.hpp"

bool All::add_user(uint32_t id, const User &user)
{
	users[id] = user;
	return true; // TODO
}

bool All::add_location(uint32_t id, const Location &location)
{
	locations[id] = location;
	return true; // TODO
}

bool All::add_visit(uint32_t id, const Visit &visit)
{
	visits[id] = visit;
	user_visits[visit.user].insert(id);
	return true; // TODO
}

User *All::get_user(uint32_t id)
{
	auto f = users.find(id);
	return f != users.end() ? &f->second : nullptr;
}

Location *All::get_location(uint32_t id)
{
	auto f = locations.find(id);
	return f != locations.end() ? &f->second : nullptr;
}

Visit *All::get_visit(uint32_t id)
{
	auto f = visits.find(id);
	return f != visits.end() ? &f->second : nullptr;
}
#include <iostream>
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
	if (users.count(id) == 0)
		return false;

	auto my_visits = user_visits.find(id);
	if (my_visits == user_visits.end())
		return true;

	for (const auto &visit_ : my_visits->second) {
		auto visit = visits[visit_];
		Location *location =
			get_location(visit.location);
		if (location == nullptr)
			continue;

		// TODO: check optional args

		out.push_back({
			visit.mark,
			visit.visited_at,
			location->place
			});
	}
	return true;
}
