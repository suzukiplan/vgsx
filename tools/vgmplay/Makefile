CORE_PATH = ../../src
CPP = g++
CPP += -O2
CPP += -std=gnu++17
CPP += `sdl2-config --cflags`
CPP += -I${CORE_PATH}
CPP += -I${CORE_PATH}/musashi
CPP += -c
OBJECTS = vgmplay.o
OBJECTS += vgmplay_elf.o
OBJECTS += vgsx.o
OBJECTS += bios.o
OBJECTS += vgs0math.o
OBJECTS += k8x12_jisx0201.o
OBJECTS += k8x12_jisx0208.o
OBJECTS += utf8_to_sjis.o
OBJECTS += musashi.o

all: vgmplay

clean:
	rm -f ${OBJECTS}

vgmplay: ${OBJECTS}
	g++ -o $@ ${OBJECTS} `sdl2-config --libs`

vgmplay.o: vgmplay.cpp ${CORE_PATH}/vgsx.h ${CORE_PATH}/vdp.hpp ${CORE_PATH}/vgmdrv.hpp ${CORE_PATH}/ymfm_opn2.hpp
	${CPP} $<

vgmplay_elf.o: vgmplay_elf.c
	gcc -c $<

vgsx.o: ${CORE_PATH}/vgsx.cpp ${CORE_PATH}/vgsx.h ${CORE_PATH}/vdp.hpp ${CORE_PATH}/vgmdrv.hpp ${CORE_PATH}/ymfm_opn2.hpp ${CORE_PATH}/utf8_to_sjis.h
	${CPP} $<

utf8_to_sjis.o: ${CORE_PATH}/utf8_to_sjis.cpp ${CORE_PATH}/utf8_to_sjis.h
	${CPP} -w $<

bios.o: ${CORE_PATH}/bios.c
	gcc -c $<

vgs0math.o: ${CORE_PATH}/vgs0math.cpp
	${CPP} $<

k8x12_jisx0201.o: ${CORE_PATH}/k8x12_jisx0201.c
	gcc -c $<

k8x12_jisx0208.o: ${CORE_PATH}/k8x12_jisx0208.c
	gcc -c $<

musashi.o: ${CORE_PATH}/musashi.cpp ${CORE_PATH}/musashi.hpp
	${CPP} -w $<
