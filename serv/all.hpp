#pragma once

#include <boost/optional.hpp>
#include <cstdint>
#include <cstring>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

struct UserMark {};
struct LoctMark {};
struct VistMark {};

struct UserMask {
	uint8_t id            :1;
	uint8_t email         :1;
	uint8_t first_name    :1;
	uint8_t last_name     :1;
	uint8_t gender_is_male:1;
	uint8_t birth_date    :1;

	void reset() { memset(this, 0, sizeof *this); }
	bool is_full() const {
		return id && email && first_name && last_name && gender_is_male && birth_date;
	}
};

struct LoctMask {
	uint8_t id      :1;
	uint8_t place   :1;
	uint8_t country :1;
	uint8_t city    :1;
	uint8_t distance:1;

	void reset() { memset(this, 0, sizeof *this); }
	bool is_full() const {
		return id && place && country && city && distance;
	}
};

struct VistMask {
	uint8_t id     :1;
	uint8_t loct   :1;
	uint8_t user   :1;
	uint8_t visited:1;
	uint8_t mark   :1;
	
	void reset() { memset(this, 0, sizeof *this); }
	bool is_full() const {
		return id && loct && user && visited && mark;
	}
};

#define _u(x) (x*4+1)

struct User {
	uint32_t id;
	char     email[_u(100)];
	char     first_name[_u(50)];
	char     last_name[_u(50)];
	bool     gender_is_male;
	time_t   birth_date;

	typedef UserMask Mask;
};

struct Loct {
	uint32_t    id;
	std::string place;
	char        country[_u(50)];
	char        city[_u(50)];
	uint32_t    distance;

	typedef LoctMask Mask;
};

struct Vist {
	uint32_t id;
	uint32_t loct;
	uint32_t user;
	uint32_t visited;
	uint8_t  mark; // 0..5

	typedef VistMask Mask;
};

struct VistData {
	uint8_t     mark;
	time_t      visited;
	std::string place;
};

namespace All {
	bool add(const User &);
	bool add(const Loct &);
	bool add(const Vist &);

	bool update(const User &, UserMask);
	bool update(const Loct &, LoctMask);
	bool update(const Vist &, VistMask);

	User *get_user(uint32_t id);
	Loct *get_loct(uint32_t id);
	Vist *get_vist(uint32_t id);

	template <class Data> Data *get(uint32_t id);

	bool get_vists(
		std::vector<VistData> &out,
		uint32_t id,
		boost::optional<time_t>      from_date,
		boost::optional<time_t>      to_date,
		boost::optional<std::string> country,
		boost::optional<uint32_t>    to_distance
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
