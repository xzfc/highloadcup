ifeq "$(G)" "1"
export LIBRARY_PATH=$(HOME)/.nix-profile/lib
export CPLUS_INCLUDE_PATH=$(HOME)/.nix-profile/include
endif

CXXFLAGS := -std=c++17 -O3
LDFLAGS := -lboost_system -lboost_filesystem

main: main.o json.o
