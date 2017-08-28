#include "all.hpp"
#include "yearsdiff.hpp"
#include <cstring>
#include <iostream>
#include <algorithm>
#include <cassert>
#include "algorithm.hpp"

#define INVALID_ID      0xFFFFFFFFU
#define USERS_COUNT         1010000
#define LOCATIONS_COUNT     1002000
#define VISIT_COUNT        10010000

struct LoctVist {
	time_t   visited;
	uint32_t id;
	uint32_t user : 24;
	uint32_t mark :  8;

	LoctVist(const Vist &o)
		: visited(o.visited)
		, id     (o.id)
		, user   (o.user)
		, mark   (o.mark)
	{ }
};

struct UserVist {
	time_t visited;
	uint32_t id;
	uint32_t loct : 24;
	uint32_t mark     : 8;

	UserVist(const Vist &o)
		: visited (o.visited)
		, id      (o.id)
		, loct(o.loct)
		, mark    (o.mark)
	{ }
};

struct CmpVisited {
	template <class T> bool operator()(time_t visited, const T &x) const {
		return visited < x.visited;
	}
	template <class T> bool operator()(const T &x, time_t visited) const {
		return x.visited < visited;
	}
	template <class T> bool operator()(const T &x, const T &y) const {
		return x.visited < y.visited;
	}
};

template <class T, size_t N>
struct Array {
	T data[N];
	T invalid;

	Array() {
		for (auto &x : data)
			x.id = INVALID_ID;
		invalid.id = INVALID_ID;
	}

	T &operator[](uint32_t id) {
		if (id < N)
			return data[id];
		std::cout << "INVALID" << id << "\n";
		abort();
		return invalid;
	}

	bool have(uint32_t id) {
		return id < N && data[id].id != INVALID_ID;
	}

	T *get(uint32_t id) {
		if (id < N && data[id].id != INVALID_ID)
			return &data[id];
		return nullptr;
	}
};

template <class T, size_t N>
struct Array2 {
	T data[N];
	T invalid;

	T &operator[](uint32_t id) {
		if (id < N)
			return data[id];
		std::cout << "INVALID" << id << "\n";
		abort();
		return invalid;
	}

	bool have(uint32_t id) {
		return id < N;
	}

	T *get(uint32_t id) {
		if (id < N)
			return &data[id];
		return nullptr;
	}
};

struct AllP {
	Array<User, USERS_COUNT>     users;
	Array<Loct, LOCATIONS_COUNT> locts;
	Array<Vist, VISIT_COUNT>     vists;

	Array2<std::vector<UserVist>, USERS_COUNT> user_visits;
	Array2<std::vector<LoctVist>, LOCATIONS_COUNT> loct_visits;

	YearDiffer year_differ;
};

static AllP &all = *(new AllP());

template <class T>
typename std::vector<T>::iterator
find_by_visited_and_id(std::vector<T> &vec, time_t visited_at, uint32_t id) {
    auto range = std::equal_range(vec.begin(), vec.end(), visited_at, CmpVisited{});
    auto fun = [id](const T &item) -> bool { return item.id == id; };
    auto it = std::find_if(range.first, range.second, fun);
    assert(it != vec.end());
    assert(it->id == id);
    assert(it->visited == visited_at);
    return it;
}

template <class T>
void
erase_by_visited_and_id(std::vector<T> &vec, time_t visited_at, uint32_t id) {
	vec.erase(find_by_visited_and_id(vec, visited_at, id));
}


bool All::add(const User &user) {
	if (all.users[user.id].id != INVALID_ID)
		return false;
	all.users[user.id] = user;
	return true;
};

bool All::add(const Loct &loct) {
	if (all.locts[loct.id].id != INVALID_ID)
		return false;
	all.locts[loct.id] = loct;
	return true;
};

bool All::add(const Vist &vist) {
	if (all.vists[vist.id].id != INVALID_ID)
		return false;
	insert_sorted(all.user_visits[vist.user], {vist}, CmpVisited{});
	insert_sorted(all.loct_visits[vist.loct], {vist}, CmpVisited{});
	all.vists[vist.id] = vist;
	return true;
};

User *All::get_user(uint32_t id) {
	return all.users.get(id);
}

Loct *All::get_loct(uint32_t id) {
	return all.locts.get(id);
}

Vist *All::get_vist(uint32_t id) {
	return all.vists.get(id);
}

#define UPDATE(X)   if (mask.X) curr->X = upd.X
#define UPDATE_S(X) if (mask.X) strcpy(curr->X, upd.X)

bool All::update(const User &upd, UserMask mask) {
	auto curr = all.users.get(upd.id);
	if (curr == nullptr)
		return false;
	UPDATE_S(email);
	UPDATE_S(first_name);
	UPDATE_S(last_name);
	UPDATE  (gender_is_male);
	UPDATE  (birth_date);
	return true;
}

bool All::update(const Loct &upd, LoctMask mask) {
	auto curr = all.locts.get(upd.id);
	if (curr == nullptr)
		return false;
	UPDATE  (place);
	UPDATE_S(country);
	UPDATE_S(city);
	UPDATE  (distance);
	return true;
}

#undef UPDATE_S

static void debug03_check(uint32_t vist_id, uint32_t loct_id, uint32_t user_id) {
#ifdef DEBUG_03
	bool loct_found = false, user_found;

	printf("loct %lu = [\n", loct_id);
	time_t prev_visited;

	prev_visited = -999999;
	for (auto &vist : all.loct_visits[loct_id]) {
		printf("  %20ld  %5lu %5lu %5lu\n", vist.visited, vist.id, vist.user, vist.mark);
		assert(vist.visited > prev_visited); prev_visited = vist.visited;
		if (vist.id == vist_id) loct_found = true;
		assert(all.vists.have(vist.id));
		assert(all.vists[vist.id].user == vist.user);
	}
	printf("]\n" "user %lu = [\n", user_id);

	prev_visited = -999999;
	for (auto &vist : all.user_visits[loct_id]) {
		printf("  %20ld  %5lu %5lu %5lu\n", vist.visited, vist.id, vist.loct, vist.mark);
		assert(vist.visited > prev_visited); prev_visited = vist.visited;
		if (vist.id == vist_id) user_found = true;
		assert(all.vists.have(vist.id));
		assert(all.vists[vist.id].loct == vist.loct);
	}

	assert(loct_found);
	assert(user_found);
	printf("]\n\n\n");
#endif
}

bool All::update(const Vist &upd, VistMask mask) {
	auto curr = all.vists.get(upd.id);
	if (curr == nullptr)
		return false;

	#define IGNORE(X) if (mask.X && curr->X == upd.X) mask.X = false
	IGNORE(loct);
	IGNORE(user);
	IGNORE(visited);
	IGNORE(mark);
	#undef IGNORE

	debug03_check(curr->id, curr->loct, curr->user);

	if (mask.loct)
		erase_by_visited_and_id(
				all.loct_visits[curr->loct],
				curr->visited, curr->id);
	if (mask.user)
		erase_by_visited_and_id(
				all.user_visits[curr->user],
				curr->visited, curr->id);

	if (mask.visited) {
		if (!mask.loct) {
			auto &vec = all.loct_visits[curr->loct];
			auto it = find_by_visited_and_id(vec, curr->visited, curr->id);
			auto new_pos = std::upper_bound(vec.begin(), vec.end(), upd.visited, CmpVisited{});
			it->visited = upd.visited;
			if (mask.mark) it->mark = upd.mark;
			if (mask.user) it->user = upd.user;
			reorder(it, new_pos);
		}
		if (!mask.user) {
			auto &vec = all.user_visits[curr->user];
			auto it = find_by_visited_and_id(vec, curr->visited, curr->id);
			auto new_pos = std::upper_bound(vec.begin(), vec.end(), upd.visited, CmpVisited{});

			it->visited = upd.visited;
			if (mask.mark) it->mark = upd.mark;
			if (mask.loct) it->loct = upd.loct;

			reorder(it, new_pos);
		}
	} else if (mask.mark || mask.loct || mask.user) {
		if (!mask.loct) {
			auto vist = find_by_visited_and_id(
				all.loct_visits[curr->loct],
				curr->visited, curr->id);
			if (mask.mark) vist->mark = upd.mark;
			if (mask.user) vist->user = upd.user;
		}
		if (!mask.user) {
			auto vist = find_by_visited_and_id(
				all.user_visits[curr->user],
				curr->visited, curr->id);
			if (mask.mark) vist->mark = upd.mark;
			if (mask.loct) vist->loct = upd.loct;
		}
	}

	UPDATE(loct);
	UPDATE(user);
	UPDATE(visited);
	UPDATE(mark);

	if (mask.loct) insert_sorted(all.loct_visits[curr->loct], {*curr}, CmpVisited{});
	if (mask.user) insert_sorted(all.user_visits[curr->user], {*curr}, CmpVisited{});

	debug03_check(curr->id, curr->loct, curr->user);

	return true;
}


static 
std::pair<unsigned, unsigned>
get_avg_(uint32_t loct_id,
	 time_t   from_date,
	 time_t   to_date,
	 time_t   from_birth,
	 time_t   to_birth,
	 boost::optional<bool> gender_is_male)
{
	auto visits = all.loct_visits.get(loct_id);
	if (visits == nullptr)
		return {0, 0};

	unsigned res_sum = 0, res_cnt = 0;
	for (auto visit = std::upper_bound(visits->begin(), visits->end(), from_date, CmpVisited{});
	     visit != visits->end() && visit->visited < to_date;
	     visit++)
	{
		auto user = all.users[visit->user];
		if (user.birth_date <= from_birth ||
		    user.birth_date >= to_birth ||
		    (gender_is_male && gender_is_male != user.gender_is_male))
			continue;

		res_sum += visit->mark;
		res_cnt ++;
	}
	return {res_sum, res_cnt};
}

static
bool
get_visits_(std::vector<VistData> &out,
            uint32_t    user_id,
	    time_t      from_date,
	    time_t      to_date,
	    boost::optional<std::string> country,
	    uint32_t    to_distance)
{
	auto visits = all.user_visits.get(user_id);
	if (visits == nullptr)
		return false;
	for (auto visit = std::upper_bound(visits->begin(), visits->end(), from_date, CmpVisited{});
	     visit != visits->end() && visit->visited < to_date;
	     visit++)
	{
		auto loct = all.locts.get(visit->loct);
		if (loct == nullptr ||
		    country && loct->country != *country ||
		    loct->distance >= to_distance)
			continue;
		out.push_back({visit->mark, visit->visited, loct->place});
	}
	return true;
}

bool All::get_vists(
		std::vector<VistData> &out,
		uint32_t id,
		boost::optional<time_t>      from_date,
		boost::optional<time_t>      to_date,
		boost::optional<std::string> country,
		boost::optional<uint32_t>    to_distance
	      )
{
	out.clear();
	return get_visits_(
		out, id,
		from_date ? *from_date : std::numeric_limits<time_t>::min(),
		to_date   ? *to_date   : std::numeric_limits<time_t>::max(),
		country,
		to_distance ? *to_distance : 0xFFFFFFFFU);
		
}

double All::get_avg(
		uint32_t id,
		boost::optional<time_t>  from_date,
		boost::optional<time_t>  to_date,
		boost::optional<uint8_t> from_age,
		boost::optional<uint8_t> to_age,
		boost::optional<bool>    gender_is_male
	      )
{
	auto res =  get_avg_(
		id,
		from_date ? *from_date : std::numeric_limits<time_t>::min(),
		to_date   ? *to_date   : std::numeric_limits<time_t>::max(),
		to_age   ? all.year_differ.jaja(*to_age) : std::numeric_limits<time_t>::min(),
		from_age ? all.year_differ.jaja(*from_age) : std::numeric_limits<time_t>::max(),
		gender_is_male);
	if (res.second == 0)
		return 0;
	return (res.first / (double)res.second) + 1e-7;
}

void All::optimize() {
}

void All::set_options(time_t now, bool full) {
	all.year_differ.set_now(now);
}

namespace All {
	template <> User *get<User>(uint32_t id) { return get_user(id); }
	template <> Vist *get<Vist>(uint32_t id) { return get_vist(id); }
	template <> Loct *get<Loct>(uint32_t id) { return get_loct(id); }
}
