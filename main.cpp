#include "all.hpp"
#include <iostream>
#include <fstream>

void parse(const char *dir);
void start_server(uint16_t port);

void load_options(const std::string &path) {
	std::ifstream options(path);
	time_t now;
	bool full;
	options >> now >> full;
	All::set_options(now, full);
}

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " PATH PORT\n";
		return 1;
	}

	parse(argv[1]);
	load_options(argv[1] + std::string("/options.txt"));
	All::optimize();
	start_server(atoi(argv[2]));
}
