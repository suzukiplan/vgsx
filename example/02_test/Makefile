VGSX_ROOT = ../..

all: test00.elf
	make run

clean:
	rm -f test00.elf

run:
	cd ${VGSX_ROOT}/tools && make
	${VGSX_ROOT}/tools/sdl2/vgsx -d -x 59 test00.elf
	@${VGSX_ROOT}/tools/sdl2/vgsx -d -x 59 test00.elf > test00_result.txt

test00.elf: test00.c ../../lib/libc.a
	m68k-elf-gcc -m68030 -O2 -I${VGSX_ROOT}/lib -o $@ $< -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld -Wl,-ecrt0 -llog
