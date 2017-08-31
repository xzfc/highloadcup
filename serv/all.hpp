#pragma once

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

void add(const User &);
void add(const Loct &);
void add(const Vist &);

template <class Data>
void run_parser(std::vector<char> &);
