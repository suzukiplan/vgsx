VGSX_ROOT = ./vgsx
BIN2VAR = ${VGSX_ROOT}/tools/bin2var/bin2var
BMP2CHR = ${VGSX_ROOT}/tools/bmp2chr/bmp2chr
BMP2PAL = ${VGSX_ROOT}/tools/bmp2pal/bmp2pal
CSV2VAR = ${VGSX_ROOT}/tools/csv2var/csv2var
MAKEROM = ${VGSX_ROOT}/tools/makerom/makerom
SDL2EMU = ${VGSX_ROOT}/tools/sdl2/vgsx
M68KCC = m68k-elf-gcc -m68030 -O2 -fno-builtin
M68KCC += -I${VGSX_ROOT}/lib -I./src
M68KCC += -T${VGSX_ROOT}/lib/linker.ld

# === Source and Objects =====================================================
SRC_DIR := ./src
OBJ_DIR := ./obj
SRC_FILES := $(wildcard $(SRC_DIR)/*.c)
OBJ_FILES := $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC_FILES))
DEP_FILES := $(OBJ_FILES:.o=.d)

# === Assets =================================================================
CHR_FILES = ./bmp/font.chr
# CHR_FILES += ./bmp/new_pattern.chr

BGM_FILES = ./vgm/sample.vgm
# BGM_FILES += ./vgm/new_song.vgm

WAV_FILES = ./wav/sample.wav
# WAV_FILES += ./wav/new_sfx.wav

# ============================================================================
# Main Target
# ============================================================================
all:
	git submodule update --init
	cd vgsx/tools && make
	make game.rom 
	${SDL2EMU} game.rom

clean:
	rm -f game.rom program.elf palette.bin ${CHR_FILES}
	rm -rf $(OBJ_DIR)

game.rom: program.elf palette.bin ${CHR_FILES} ${BGM_FILES} ${WAV_FILES}
	${MAKEROM} -o $@ -e program.elf -c palette.bin -g ${CHR_FILES} -b ${BGM_FILES} -s ${WAV_FILES}

program.elf: ${OBJ_FILES}
	${M68KCC} -o $@ ${OBJ_FILES} -L${VGSX_ROOT}/lib -Wl,-ecrt0 -llog

palette.bin: ./bmp/font.bmp
	${BMP2PAL} $< $@

.SUFFIXES: .bmp .chr

.bmp.chr:
	${BMP2CHR} $< $@

# ============================================================================
# Object Output Settings
# ============================================================================

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c | $(OBJ_DIR)
	${M68KCC} -MMD -MP -c $< -o $@

-include $(DEP_FILES)
