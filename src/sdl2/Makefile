CORE_PATH = ../core
CPP = g++
CPP += -O2
CPP += `sdl2-config --cflags`
CPP += -I${CORE_PATH}
CPP += -I${CORE_PATH}/musashi
CPP += -I${CORE_PATH}/ymfm/src
CPP += -c
OBJECTS = main.o
OBJECTS += vgsx.o
OBJECTS += vgs0math.o
OBJECTS += m68kcpu.o
OBJECTS += m68kops.o
OBJECTS += m68kdasm.o
OBJECTS += softfloat.o
OBJECTS += ymfm_opn.o
OBJECTS += ymfm_adpcm.o
OBJECTS += ymfm_ssg.o

all: vgsx

clean:
	rm -f ${OBJECTS}

vgsx: ${OBJECTS}
	g++ -o $@ ${OBJECTS} `sdl2-config --libs`

main.o: main.cpp
	${CPP} $<

vgsx.o: ${CORE_PATH}/vgsx.cpp ${CORE_PATH}/vgsx.h ${CORE_PATH}/vdp.hpp
	${CPP} $<

vgs0math.o: ${CORE_PATH}/vgs0math.cpp
	${CPP} $<

m68kcpu.o: ${CORE_PATH}/musashi/m68kcpu.c
	${CPP} -w $<

m68kops.o: ${CORE_PATH}/musashi/m68kops.c
	${CPP} -w $<

m68kdasm.o: ${CORE_PATH}/musashi/m68kdasm.c
	${CPP} -w $<

softfloat.o: ${CORE_PATH}/musashi/softfloat/softfloat.c
	${CPP} -w $<

ymfm_opn.o: ${CORE_PATH}/ymfm/src/ymfm_opn.cpp
	${CPP} -w $<

ymfm_adpcm.o: ${CORE_PATH}/ymfm/src/ymfm_adpcm.cpp
	${CPP} -w $<

ymfm_ssg.o: ${CORE_PATH}/ymfm/src/ymfm_ssg.cpp
	${CPP} -w $<
