CXXFLAGS := -std=c++17 -O3 -MMD -MP -Wall -Wextra -Wno-parentheses -Wno-maybe-uninitialized -g
LDFLAGS := -lboost_system -lboost_filesystem -pthread
SRC := main.cpp json.cpp all.cpp http-app.cpp http-server.cpp http-request-line-parser.cpp
OBJ := $(SRC:.cpp=.o)
DEP := $(SRC:.cpp=.d)

main: $(OBJ)
	$(CXX) $^ -o $@ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(DEP) $(OBJ) main

-include $(DEP)
