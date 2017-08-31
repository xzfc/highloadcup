#include "all.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <shared_mutex>
#include<iterator>

void put_log(const char *text) {
	std::string s;
	std::ifstream("/proc/uptime") >> s;
	std::cout << "[" << std::setw(11) << s << "] " << text << std::endl;
}

static size_t sum;

static std::vector<char> load_file(const char *path) {
	std::ifstream f(path, std::ios::binary);
	return std::vector<char>(
		std::istreambuf_iterator<char>(f),
		std::istreambuf_iterator<char>());
}

int main(int argc, char **argv) {
	auto
		users = load_file(argv[1]),
		locts = load_file(argv[2]),
		vists = load_file(argv[3]);

	run_parser<User>(users);
	run_parser<Loct>(locts);
	run_parser<Vist>(vists);
	std::cout << sum << "\n";

	auto count = 1024*4;

	timespec ts, ts2;
	clock_gettime(CLOCK_MONOTONIC, &ts);
	for (int i = 0; i < count; i++) {
		run_parser<User>(users);
		run_parser<Loct>(locts);
		run_parser<Vist>(vists);
	}
	clock_gettime(CLOCK_MONOTONIC, &ts2);

	double dt = 1.0 * (ts2.tv_sec - ts.tv_sec) + 1e-9 * (ts2.tv_nsec - ts.tv_nsec);
	std::cout << std::setprecision(15) << dt << std::endl;
	size_t data_size = users.size() + locts.size() + vists.size();
	std::cout << std::setprecision(15) << (1.0 * count * data_size) / dt / (1024.0 * 1024.0) << std::endl;
}

#define $int(X) sum += (unsigned)data.X
#define $str(X) sum += strlen(data.X)

void add(const User &data) {
	$int(id);
	$str(email);
	$str(first_name);
	$str(last_name);
}
void add(const Loct &data) {
	$int(id);
	$str(city);
	$str(country);
	$str(place.c_str());
}
void add(const Vist &data) {
	$int(id);
	$int(mark);
}
