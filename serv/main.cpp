#include "all.hpp"
#include "json.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>

void start_server(uint16_t port);

void load_options(const std::string &path) {
	std::ifstream options(path);
	time_t now;
	bool full;
	options >> now >> full;
	All::set_options(now, full);
}

void put_log(const char *text) {
	std::string s;
	std::ifstream("/proc/uptime") >> s;
	std::cout << "[" << std::setw(11) << s << "] " << text << std::endl;
}

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " PATH PORT\n";
		return 1;
	}

	parse(argv[1]);
	put_log("parsed");
	load_options(argv[1] + std::string("/options.txt"));
	All::optimize();
	put_log("optimized");
	start_server(atoi(argv[2]));
}
