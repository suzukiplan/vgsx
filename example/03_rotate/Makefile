VGSX_ROOT = ../..

CHR_FILES = font.chr
CHR_FILES += player.chr

all:
	cd ${VGSX_ROOT}/tools && make
	make execute

clean:
	rm -f program.rom program.elf font.chr palette.bin 

execute: program.rom
	${VGSX_ROOT}/tools/sdl2/vgsx program.rom

program.rom: program.elf palette.bin ${CHR_FILES}
	${VGSX_ROOT}/tools/makerom/makerom \
		-c palette.bin \
		-g ${CHR_FILES} \
		-e program.elf \
		-o program.rom

program.elf: program.c ../../lib/libc.a
	m68k-elf-gcc -m68030 -O2 -I${VGSX_ROOT}/lib -o $@ $< -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld -Wl,-ecrt0

palette.bin: font.bmp
	${VGSX_ROOT}/tools/bmp2pal/bmp2pal $< $@

.SUFFIXES: .bmp .chr

.bmp.chr:
	${VGSX_ROOT}/tools/bmp2chr/bmp2chr $< $@
