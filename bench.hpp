#include <chrono>
#include <sstream>

class bench {
	unsigned count;
	std::chrono::steady_clock::duration t;
public:
	bench() : count(0), t(0) {

	}
	void ok(const std::chrono::time_point<std::chrono::steady_clock> d) {
		count++;
		t += (now() - d);
	}
	std::string str() {
		std::stringstream ss;
		auto us = std::chrono::duration_cast<std::chrono::microseconds>(t).count();
		ss << count << " / " << us << " us\n";
		return ss.str();
	}
	static std::chrono::time_point<std::chrono::steady_clock> now() {
		return std::chrono::steady_clock::now();
	}
};
