CORE_PATH = ../../src
CXX = g++
CXXFLAGS = -O2 -std=gnu++17 -I${CORE_PATH} -c
OBJECTS = vgm2wav.o

all: vgm2wav

clean:
	rm -f vgm2wav ${OBJECTS}

vgm2wav: ${OBJECTS}
	${CXX} -o $@ ${OBJECTS}

vgm2wav.o: vgm2wav.cpp ${CORE_PATH}/vgmdrv.hpp ${CORE_PATH}/ymfm_opn2.hpp
	${CXX} ${CXXFLAGS} $<
