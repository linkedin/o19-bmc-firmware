CPP_SRCS := $(wildcard *.cpp)
CXXFLAGS += -std=c++11 -Wall -Werror -g
LDFLAGS += -lstdc++fs

all: config-util

config-util: $(CPP_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o config-util
