CXX ?= g++
CXXFLAGS ?= -Wall -O3 -std=c++17

.PHONY: all check clean
all: build/mcp_bb

build/mcp_bb: src/mcp_bb.cc
	mkdir -p build
	$(CXX) $(CXXFLAGS) -o $@ $<

check: build/mcp_bb
	python tests/check_cases.py --binary build/mcp_bb

clean:
	rm -f build/mcp_bb
