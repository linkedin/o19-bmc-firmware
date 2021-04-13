CPP_SRCS := $(wildcard *.cpp)
CXXFLAGS += -std=c++17 -Wall -Werror -g
LDFLAGS += -lstdc++fs -lpowershelf -lgpio

all: fan-control

fan-control: $(CPP_SRCS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

.PHONY: clean

clean:
	rm -rf *.o fan-control
