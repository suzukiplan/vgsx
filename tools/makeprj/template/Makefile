VGSX_ROOT = ./vgsx
BMP2CHR = ${VGSX_ROOT}/tools/bmp2chr/bmp2chr
BMP2PAL = ${VGSX_ROOT}/tools/bmp2pal/bmp2pal
MAKEROM = ${VGSX_ROOT}/tools/makerom/makerom
SDL2EMU = ${VGSX_ROOT}/tools/sdl2/vgsx
M68KCC = m68k-elf-gcc -m68030 -O2 -I${VGSX_ROOT}/lib -I./src

OBJ_FILES = ./src/main.o
# OBJ_FILES += ./src/sub.o
CHR_FILES = ./bmp/font.chr
BGM_FILES = ./vgm/sample.vgm
WAV_FILES = ./wav/sample.wav

all:
	cd vgsx/tools && make
	make game.rom 
	${SDL2EMU} game.rom

clean:
	rm -f game.rom program.elf palette.bin ${CHR_FILES} ${OBJ_FILES}

game.rom: program.elf palette.bin ${CHR_FILES} ${BGM_FILES} ${WAV_FILES}
	${MAKEROM} -o $@ -e program.elf -c palette.bin -g ${CHR_FILES} -b ${BGM_FILES} -s ${WAV_FILES}

program.elf: ${OBJ_FILES}
	${M68KCC} -o $@ ${OBJ_FILES} -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld -Wl,-ecrt0

palette.bin: ./bmp/font.bmp
	${BMP2PAL} $< $@
	
.SUFFIXES: .bmp .chr

.bmp.chr:
	${BMP2CHR} $< $@

.c.o:
	${M68KCC} -c $< -o $@
