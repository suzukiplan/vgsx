VGSX_ROOT = ../..

all:
	cd ${VGSX_ROOT}/tools && make
	make execute

clean:
	rm -f test00.elf

execute: test00.elf
	${VGSX_ROOT}/tools/sdl2/vgsx -x 59 test00.elf

test00.elf: test00.c ../../lib/libc.a
	m68k-elf-gcc -m68030 -O2 -I${VGSX_ROOT}/lib -o $@ $< -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld -Wl,-ecrt0 -llog
