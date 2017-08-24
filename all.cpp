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
	for (const auto &visit : visits) {
		if (visit.second.user == id) {
			Location *location =
				get_location(visit.second.location);
			if (location == nullptr)
				continue;

			out.push_back({
				visit.second.mark,
				visit.second.visited_at,
				location->place
			});
		}
	}
	return true;
}
