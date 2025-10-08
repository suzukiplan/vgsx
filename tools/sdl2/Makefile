CORE_PATH = ../../src
CPP = g++
CPP += -O2
CPP += -std=gnu++17
CPP += `sdl2-config --cflags`
CPP += -I${CORE_PATH}
CPP += -I${CORE_PATH}/musashi
CPP += -c
OBJECTS = main.o
OBJECTS += vgsx.o
OBJECTS += bios.o
OBJECTS += vgs0math.o
OBJECTS += k8x12_jisx0201.o
OBJECTS += k8x12_jisx0208.o
OBJECTS += utf8_to_sjis.o
OBJECTS += musashi.o
HEADER_FILES = ${CORE_PATH}/*.h
HEADER_FILES += ${CORE_PATH}/*.hpp

all: vgsx

clean:
	rm -f ${OBJECTS}

vgsx: ${OBJECTS}
	g++ -o $@ ${OBJECTS} `sdl2-config --libs`

main.o: main.cpp ${HEADER_FILES}
	${CPP} $<

vgsx.o: ${CORE_PATH}/vgsx.cpp ${HEADER_FILES}
	${CPP} $<

musashi.o: ${CORE_PATH}/musashi.cpp ${HEADER_FILES}
	${CPP} -w $<

bios.o: ${CORE_PATH}/bios.c
	gcc -c $<

utf8_to_sjis.o: ${CORE_PATH}/utf8_to_sjis.cpp ${CORE_PATH}/utf8_to_sjis.h
	${CPP} -w $<

vgs0math.o: ${CORE_PATH}/vgs0math.cpp
	${CPP} $<

k8x12_jisx0201.o: ${CORE_PATH}/k8x12_jisx0201.c
	gcc -c $<

k8x12_jisx0208.o: ${CORE_PATH}/k8x12_jisx0208.c
	gcc -c $<
