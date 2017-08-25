#include "all.hpp"
#include <iostream>

void parse(All &all, const char *dir);
void start_server(All &all, uint16_t port);

int main(int argc, char **argv) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " PATH PORT\n";
		return 1;
	}
	All all;
	parse(all, argv[1]);
	start_server(all, atoi(argv[2]));
}
