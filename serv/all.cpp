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

struct LoctP : Loct {
	LoctP(const Loct &loct) : Loct(loct) {}
	bi::avl_set_member_hook<> hook_id;
};

struct VistP : Vist {
	VistP(const Vist &vist) : Vist(vist) {}
	bi::avl_set_member_hook<> hook_id, hook_loct, hook_user_visted;

	LoctP *loct_ptr = 0;
	LoctP *get_loct();

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

struct CompareVistUserVisted {
	typedef Vist type;
	const Vist &operator()(const VistP &v) const {
		return v;
	}
	bool operator()(const Vist &a, const Vist &b) const {
		if (a.user < b.user) return true;
		if (a.user > b.user) return false;

		if (a.visited < b.visited) return true;
		if (a.visited > b.visited) return false;

		if (a.id < b.id) return true;
		if (a.id > b.id) return false;

		return false;
	}
};

struct CompareVistLoct {
	typedef Vist type;
	const Vist &operator()(const VistP &v) const {
		return v;
	}
	bool operator()(const Vist &a, const Vist &b) const {
		if (a.loct < b.loct) return true;
		if (a.loct > b.loct) return false;

		if (a.visited < b.visited) return true;
		if (a.visited > b.visited) return false;

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
	LoctP,
	bi::member_hook<LoctP, bi::avl_set_member_hook<>, &LoctP::hook_id>,
	bi::key_of_value<KeyId>
> LoctTreeId;

typedef bi::avl_set<
	VistP,
	bi::member_hook<VistP, bi::avl_set_member_hook<>, &VistP::hook_id>,
	bi::key_of_value<KeyId>
> VistTreeId;

typedef bi::avl_set<
	VistP,
	bi::member_hook<VistP, bi::avl_set_member_hook<>, &VistP::hook_loct>,
	bi::key_of_value<CompareVistLoct>,
	bi::compare<CompareVistLoct>
> VistTreeLoct;

typedef bi::avl_set<
	VistP,
	bi::member_hook<VistP, bi::avl_set_member_hook<>, &VistP::hook_user_visted>,
	bi::key_of_value<CompareVistUserVisted>,
	bi::compare<CompareVistUserVisted>
> VistTreeUserVisted;

struct AllP {
	UserTreeId            tree_user_id;
	LoctTreeId        tree_loct_id;
	VistTreeId           tree_vist_id;

	VistTreeLoct     tree_vist_loct;
	VistTreeUserVisted  tree_vist_user_visted;

	YearDiffer year_differ;
};

static AllP allp;


LoctP *VistP::get_loct() {
	if (loct_ptr != nullptr)
		return loct_ptr;
	auto f = allp.tree_loct_id.find(loct);
	loct_ptr = f != allp.tree_loct_id.end() ? &*f : nullptr;
	if (loct_ptr == nullptr)
		std::cout << "Wooo" << std::endl;
	return loct_ptr;
}

UserP *VistP::get_user() {
	if (user_ptr != nullptr)
		return user_ptr;
	auto f = allp.tree_user_id.find(user);
	user_ptr = f != allp.tree_user_id.end() ? &*f : nullptr;
	if (user_ptr == nullptr)
		std::cout << "Hooo" << std::endl;
	return user_ptr;
}


bool All::add(const User &user) {
	auto it = allp.tree_user_id.find(user.id);
	if (it != allp.tree_user_id.end())
		return false;
	auto e = new UserP(user);
	allp.tree_user_id.insert(it, *e);
	return true;
}

bool All::add(const Loct &loct) {
	auto it = allp.tree_loct_id.find(loct.id);
	if (it != allp.tree_loct_id.end())
		return false;
	auto e = new LoctP(loct);
	allp.tree_loct_id.insert(it, *e);
	return true;
}

bool All::add(const Vist &vist) {
	auto it = allp.tree_vist_id.find(vist.id);
	if (it != allp.tree_vist_id.end())
		return false;
	auto e = new VistP(vist);
	allp.tree_vist_id.insert(it, *e);
	allp.tree_vist_loct.insert(*e);
	allp.tree_vist_user_visted.insert(*e);
	return true;
}

bool All::update(const User &user, UserMask mask) {
	auto it = allp.tree_user_id.find(user.id);
	if (it == allp.tree_user_id.end())
		return false;
	if (mask.email)          std::strcpy(it->email,           user.email);
	if (mask.first_name)     std::strcpy(it->first_name,      user.first_name);
	if (mask.last_name)      std::strcpy(it->last_name,       user.last_name);
	if (mask.gender_is_male) it->gender_is_male = user.gender_is_male;
	if (mask.birth_date)     it->birth_date     = user.birth_date;
	return true;
}

bool All::update(const Loct &loct, LoctMask mask) {
	auto it = allp.tree_loct_id.find(loct.id);
	if (it == allp.tree_loct_id.end())
		return false;
	if (mask.place)    it->place    = loct.place;
	if (mask.country)  std::strcpy(it->country,   loct.country);
	if (mask.city)     std::strcpy(it->city,      loct.city);
	if (mask.distance) it->distance = loct.distance;
	return true;
}

bool All::update(const Vist &vist, VistMask mask) {
	auto it = allp.tree_vist_id.find(vist.id);
	if (it == allp.tree_vist_id.end())
		return false;

	if (mask.loct || mask.visited)
		allp.tree_vist_loct.erase(*it);

	if (mask.user || mask.visited)
		allp.tree_vist_user_visted.erase(*it);

	if (mask.loct)      { it->loct      = vist.loct; it->loct_ptr = 0; }
	if (mask.user)      { it->user      = vist.user; it->user_ptr = 0; }
	if (mask.visited)   it->visited = vist.visited;
	if (mask.mark)        it->mark      = vist.mark;

	if (mask.loct || mask.visited)
		allp.tree_vist_loct.insert(*it);

	if (mask.user || mask.visited)
		allp.tree_vist_user_visted.insert(*it);

	return true;
}

User *All::get_user(uint32_t id) {
	auto f = allp.tree_user_id.find(id);
	return f != allp.tree_user_id.end() ? &*f : nullptr;
}

Loct *All::get_loct(uint32_t id) {
	auto f = allp.tree_loct_id.find(id);
	return f != allp.tree_loct_id.end() ? &*f : nullptr;
}

Vist *All::get_vist(uint32_t id) {
	auto f = allp.tree_vist_id.find(id);
	return f != allp.tree_vist_id.end() ? &*f : nullptr;
}

bool All::get_vists(
	std::vector<VistData> &out,
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

	Vist first;
	first.user = id;
	first.visited = *from_date;
	first.id = 0;

	Vist last;
	last.user = id;
	last.visited = *to_date;
	last.id = 0xFFFFFFFFu;

	auto it = allp.tree_vist_user_visted.lower_bound(first);
	if (it == allp.tree_vist_user_visted.end())
		return true;

	auto end = allp.tree_vist_user_visted.upper_bound(last);

	for (; it != end; it++) {
		auto &vist = *it;
		Loct *loct = vist.get_loct();
		if (loct == nullptr)
			continue;

		if (country     && loct->country != *country ||
		    to_distance && loct->distance >= *to_distance)
			continue;

		out.push_back({
			vist.mark,
			vist.visited,
			loct->place
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

	Vist first;
	first.loct = id;
	first.visited = *from_date;
	first.id = 0;

	Vist last;
	last.loct = id;
	last.visited = *to_date;
	last.id = 0xFFFFFFFFu;

	auto it = allp.tree_vist_loct.lower_bound(first);
	if (it == allp.tree_vist_loct.end())
		return 0;

	auto end = allp.tree_vist_loct.upper_bound(last);

	unsigned count = 0, sum = 0;
	for (; it != end; it++) {
		auto &vist = *it;
		UserP *user = vist.get_user();
		if (user == nullptr)
			continue;

		if (user->birth_date <= birth_from ||
		    user->birth_date >= birth_to   ||
		    gender_is_male && user->gender_is_male != *gender_is_male)
			continue;

		count++;
		sum += vist.mark;
	}

	if (count == 0)
		return 0;
	return (double)sum / count;
}

void All::optimize() {
	for (auto &vist : allp.tree_vist_id) {
		vist.get_loct();
		vist.get_user();
	}
}

void All::set_options(time_t now, bool full) {
	allp.year_differ.set_now(now);
}

namespace All {
	template <> User *get<User>(uint32_t id) { return get_user(id); }
	template <> Vist *get<Vist>(uint32_t id) { return get_vist(id); }
	template <> Loct *get<Loct>(uint32_t id) { return get_loct(id); }
}
