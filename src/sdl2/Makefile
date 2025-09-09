CPP = g++ -O2 -c
CORE_PATH = ../core
OBJECTS = main.o
OBJECTS += vgsx.o
OBJECTS += m68kcpu.o
OBJECTS += m68kops.o
OBJECTS += softfloat.o
OBJECTS += ymfm_opn.o
OBJECTS += ymfm_adpcm.o
OBJECTS += ymfm_ssg.o

all: vgsx

clean:
	rm -f ${OBJECTS}

vgsx: ${OBJECTS}
	g++ -o $@ ${OBJECTS}

main.o: main.cpp
	${CPP} $<

vgsx.o: ${CORE_PATH}/vgsx.cpp ${CORE_PATH}/vgsx.h ${CORE_PATH}/vdp.hpp
	${CPP} $<

m68kcpu.o: ${CORE_PATH}/musashi/m68kcpu.c
	${CPP} -w $<

m68kops.o: ${CORE_PATH}/musashi/m68kops.c
	${CPP} -w $<

softfloat.o: ${CORE_PATH}/musashi/softfloat/softfloat.c
	${CPP} -w $<

ymfm_opn.o: ${CORE_PATH}/ymfm/src/ymfm_opn.cpp
	${CPP} -w $<

ymfm_adpcm.o: ${CORE_PATH}/ymfm/src/ymfm_adpcm.cpp
	${CPP} -w $<

ymfm_ssg.o: ${CORE_PATH}/ymfm/src/ymfm_ssg.cpp
	${CPP} -w $<
