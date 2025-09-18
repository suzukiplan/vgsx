VGSX_ROOT = ../..

all:
	cd ${VGSX_ROOT}/src/sdl2 && make
	cd ${VGSX_ROOT}/tools && make
	make execute

clean:
	rm -f program.rom program.elf font.chr palette.bin 

execute: program.rom
	${VGSX_ROOT}/src/sdl2/vgsx program.rom

program.rom: program.elf font.chr gamepad.chr palette.bin
	${VGSX_ROOT}/tools/makerom/makerom \
		-c palette.bin \
		-g font.chr \
		-g gamepad.chr \
		-b test.vgm \
		-s test.wav \
		-e program.elf \
		-o program.rom

program.elf: program.c ../../lib/libc.a
	m68k-elf-gcc -m68030 -O2 -I${VGSX_ROOT}/lib -o $@ $< -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld -Wl,-ecrt0

font.chr: font.bmp
	${VGSX_ROOT}/tools/bmp2chr/bmp2chr $< $@

gamepad.chr: gamepad.bmp
	${VGSX_ROOT}/tools/bmp2chr/bmp2chr $< $@

palette.bin: font.bmp
	${VGSX_ROOT}/tools/bmp2pal/bmp2pal $< $@
