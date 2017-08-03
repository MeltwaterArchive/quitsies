BINNAME= quitsies

CC    = g++
#CFLAGS= -std=c++11 -Wall -O0 -g
CFLAGS= -std=c++11 -Wall -O2

INCLUDES= -I./src
LFLAGS  =
TFLAGS= -DCATCH_CONFIG_MAIN

LIBS= \
	-lserved \
	-lrocksdb \
	-lboost_system \
	-lboost_filesystem

BUILDBASE=target
BUILDBIN=$(BUILDBASE)/bin
BUILDOBJ=$(BUILDBASE)/obj
BUILDTEST=$(BUILDBASE)/test
BINPATH=usr/bin/
PATHINSTBIN=$(DESTDIR)/$(BINPATH)

MAIN= $(BUILDBIN)/$(BINNAME)

ALL_SRCS =$(wildcard src/*.cpp src/*/*.cpp src/*/*/*.cpp)
SRCS     =$(filter-out %.test.cpp src/test/catch.cpp, $(ALL_SRCS))
OBJS     =$(SRCS:.cpp=.o)
TEST_SRCS=$(filter %.test.cpp, $(ALL_SRCS))
TESTS    =$(TEST_SRCS:.test.cpp=_test)

.PHONY: test clean install

all: $(MAIN)

$(MAIN): $(OBJS)
	@mkdir -p $(BUILDBIN)
	$(CC) $(CFLAGS) $(INCLUDES) -o $(MAIN) $(OBJS) $(LFLAGS) $(LIBS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

test: $(TESTS)
	@$(foreach test, $(TESTS), ./$(test);)

%.test.cpp:

%_test: %.test.cpp $(OBJS)
	@mkdir -p $(BUILDBIN)
	$(CC) $(TFLAGS) $(CFLAGS) $(INCLUDES) -o $@ $(@:_test=.test.cpp) $(filter-out %/service.o, $(OBJS)) $(LFLAGS) $(LIBS)

clean:
	$(RM) $(OBJS) $(TESTS) *~ $(MAIN)

install: all
	mkdir -p $(PATHINSTBIN)
	cp -r $(BUILDBIN)/* $(PATHINSTBIN)
	find $(PATHINSTBIN) -type d -exec chmod 755 {} \;
	find $(PATHINSTBIN) -type f -exec chmod 755 {} \;
