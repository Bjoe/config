# The Art of C++
# Copyright (c) 2018 Dr. Colin Hirsch and Daniel Frey
# Please see LICENSE for license or visit https://github.com/taocpp/config

CXXSTD = -std=c++17
CPPFLAGS = -pedantic -I../json/include -Iinclude
CXXFLAGS = -Wall -Wextra -Werror -O3

SOURCES := $(shell find src -name '*.cpp')
DEPENDS := $(SOURCES:%.cpp=build/%.d)
BINARIES := $(SOURCES:%.cpp=build/%)

TESTFILES := $(shell find tests -name '*.config')
TESTCASES := $(TESTFILES:%.config=%)

.PHONY: all test

all: test $(BINARIES)

test: build/src/test/config/tests
	build/src/test/config/tests $(TESTCASES)

build/%.d: %.cpp Makefile
	@mkdir -p $(@D)
	$(CXX) $(CXXSTD) $(CPPFLAGS) -MM -MQ $@ $< -o $@

build/%: %.cpp build/%.d
	$(CXX) $(CXXSTD) $(CPPFLAGS) $(CXXFLAGS) $< -o $@

-include $(DEPENDS)
