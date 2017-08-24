CXXFLAGS := -std=c++1y -O3 -MMD -MP
LDFLAGS := -lboost_system -lboost_filesystem
SRC := main.cpp json.cpp
OBJ := $(SRC:.cpp=.o)
DEP := $(SRC:.cpp=.d)

main: $(OBJ)

.PHONY: clean
clean:
	rm -f $(DEP) $(OBJ)

-include $(DEP)
