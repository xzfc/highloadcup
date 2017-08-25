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

	user_visits.insert({ visit.user, visit.visited_at, id });
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

	if (!from_date) from_date = { std::numeric_limits<time_t>::min() };
	if (!to_date)   to_date   = { std::numeric_limits<time_t>::max() };

	if (*from_date >= *to_date)
		return true;

	auto it  = user_visits.lower_bound({ id, *from_date, 0 });
	if (it == user_visits.end())
		return true;
	auto end = user_visits.upper_bound({ id, *to_date,   0xFFFFFFFFu });

	for (; it != end; it++) {
		const auto &visit = visits[it->visit];
		Location *location = get_location(visit.location);
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
