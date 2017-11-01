#
# This file is intended to provide basic rules for other upper-layer makefiles,
# therefore its contents should be public, necessary and concise.
# Besides, this file should be easy to use. In other words, it is able to be
# included at any place (mostly at the beginning or at the end) in a caller makefile
# without causing any side-effects. To achieve this goal, some points should be kept in mind:
#   1) ":=" is not recommanded in variable assignments, use other assignment operators instead.
#   2) No targets should exist in this file.
# Or you can modify this file to meet your needs.
#

CC = gcc
CXX = g++
AR = ar
AR_FLAGS = -r
RANLIB = ranlib

ifeq ("$(CXX)","g++")
    CFLAGS += -Wall
    CXXFLAGS += -Wall
endif
ifdef GPROF
    CFLAGS += -pg
    CXXFLAGS += -pg
endif

# Any target is not recommended.
#clean_svn_rubbish:
#	find . -iname "*.c*" | xargs grep "\.svn" | xargs rm -rf

#
# TODO: Any idea to define the rules more concisely?
#

#C_SUFFIXES = .c .C
#CPP_SUFFIXES = .cpp .cxx .cc

#$(C_SUFFIXES): $(addsuffix .o, $(basename %)): %
#	$(CC) $(CFLAGS) -c -o $@ $^

#$(CPP_SUFFIXES): $(addsuffix .o, $(basename %)): %
#	$(CXX) $(CFLAGS) -c -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.C
	$(CC) $(CFLAGS) -c -o $@ $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $^

%.o: %.cc
	$(CXX) $(CXXFLAGS) -c -o $@ $^

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c -o $@ $^
