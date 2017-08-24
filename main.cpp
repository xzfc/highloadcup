#include "all.hpp"
#include <iostream>

void parse(All &all);
void start_server(All &all);
int main()
{
	All all;
	parse(all);

	std::cout << all.users.size() << "\n";
	std::cout << all.locations.size() << "\n";
	std::cout << all.visits.size() << "\n";

	start_server(all);
}
