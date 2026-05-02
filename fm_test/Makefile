CXX = g++
CC = gcc
CXXFLAGS = -O2 -std=gnu++17 -I../src -I.
CFLAGS = -O2 -std=c99 -I.

OBJECTS = test.o Nuked-OPN2/ym3438.o YMF276-LLE/fmopn2.o

all: test

test: $(OBJECTS)
	$(CXX) -o $@ $(OBJECTS)

test.o: test.cpp ../src/vgmdrv.hpp ../src/ymfm_opn2.hpp Nuked-OPN2/ym3438.h YMF276-LLE/fmopn2.h
	$(CXX) $(CXXFLAGS) -c $<

Nuked-OPN2/ym3438.o: Nuked-OPN2/ym3438.c Nuked-OPN2/ym3438.h
	$(CC) $(CFLAGS) -c $< -o $@

YMF276-LLE/fmopn2.o: YMF276-LLE/fmopn2.c YMF276-LLE/fmopn2.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f test $(OBJECTS)
