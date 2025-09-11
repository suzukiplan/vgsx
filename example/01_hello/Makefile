VGSX_ROOT = ../..

all:
	cd ${VGSX_ROOT}/src/sdl2 && make
	cd ${VGSX_ROOT}/tools && make
	m68k-elf-gcc -I${VGSX_ROOT}/lib -o program program.c -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld
	${VGSX_ROOT}/src/sdl2/vgsx program
