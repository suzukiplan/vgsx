CORE_PATH = ../../src
CPP = g++
CPP += -O2
CPP += -std=gnu++17
CPP += `sdl2-config --cflags`
CPP += -I${CORE_PATH}
CPP += -I${CORE_PATH}/musashi
CPP += -I${CORE_PATH}/ymfm/src
CPP += -c
OBJECTS = vgmplay.o
OBJECTS += vgmplay_elf.o
OBJECTS += vgsx.o
OBJECTS += bios.o
OBJECTS += vgs0math.o
OBJECTS += k8x12_jisx0201.o
OBJECTS += k8x12_jisx0208.o
OBJECTS += utf8_to_sjis.o
OBJECTS += m68kcpu.o
OBJECTS += m68kops.o
OBJECTS += m68kdasm.o
OBJECTS += softfloat.o
OBJECTS += ymfm_opn.o
OBJECTS += ymfm_opm.o
OBJECTS += ymfm_misc.o
OBJECTS += ymfm_adpcm.o
OBJECTS += ymfm_ssg.o

all: vgmplay

clean:
	rm -f ${OBJECTS}

vgmplay: ${OBJECTS}
	g++ -o $@ ${OBJECTS} `sdl2-config --libs`

vgmplay.o: vgmplay.cpp ${CORE_PATH}/vgsx.h ${CORE_PATH}/vdp.hpp ${CORE_PATH}/vgmrender.hpp
	${CPP} $<

vgmplay_elf.o: vgmplay_elf.c
	gcc -c $<

vgsx.o: ${CORE_PATH}/vgsx.cpp ${CORE_PATH}/vgsx.h ${CORE_PATH}/vdp.hpp ${CORE_PATH}/vgmrender.hpp ${CORE_PATH}/utf8_to_sjis.h
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

ymfm_opm.o: ${CORE_PATH}/ymfm/src/ymfm_opm.cpp
	${CPP} -w $<

ymfm_misc.o: ${CORE_PATH}/ymfm/src/ymfm_misc.cpp
	${CPP} -w $<

ymfm_adpcm.o: ${CORE_PATH}/ymfm/src/ymfm_adpcm.cpp
	${CPP} -w $<

ymfm_ssg.o: ${CORE_PATH}/ymfm/src/ymfm_ssg.cpp
	${CPP} -w $<
