VGSX_ROOT = ../..

all:
	cd ${VGSX_ROOT}/tools && make
	make execute

clean:
	rm -f program.rom program.elf *.chr palette.bin

execute: program.rom
	${VGSX_ROOT}/tools/sdl2/vgsx -m -rsv program.rom

program.rom: program.elf font.chr mapchip.chr palette.bin
	${VGSX_ROOT}/tools/makerom/makerom \
		-c palette.bin \
		-g font.chr \
		-g mapchip.chr \
		-e program.elf \
		-o program.rom

program.elf: program.c map.c route.h ../../lib/libc.a
	m68k-elf-gcc -m68030 -O2 $(CPPFLAGS) -I${VGSX_ROOT}/lib -o $@ program.c map.c -L${VGSX_ROOT}/lib -T${VGSX_ROOT}/lib/linker.ld -Wl,-ecrt0

font.chr: font.bmp
	${VGSX_ROOT}/tools/bmp2chr/bmp2chr $< $@

mapchip.chr: mapchip.png Makefile
	${VGSX_ROOT}/tools/bmp2chr/bmp2chr -s 1 $< $@

palette.bin: mapchip.png Makefile ${VGSX_ROOT}/tools/bmp2pal/bmp2pal
	${VGSX_ROOT}/tools/bmp2pal/bmp2pal $< $@

route.h: map.c mapchip-patterns.json generate-route.py
	python3 generate-route.py

${VGSX_ROOT}/tools/bmp2pal/bmp2pal: ${VGSX_ROOT}/tools/bmp2pal/bmp2pal.c
	$(MAKE) -C ${VGSX_ROOT}/tools/bmp2pal

test: route.h
	@set -e; test_binary=$$(mktemp /tmp/vgs-mode7-test.XXXXXX); trap 'rm -f "$$test_binary"' EXIT; \
	for settings in "" "-DSTAR_COUNT=0" "-DMAP_VERTICAL_CHIP_NUM=12 -DMAP_ANGLE=45 -DMAP_SCALE=150" "-DMAP_VERTICAL_CHIP_NUM=50 -DMAP_ANGLE=0 -DMAP_ANGLE_DEPTH=0 -DMAP_SCALE=100" "-DMAP_VERTICAL_CHIP_NUM=25 -DMAP_ANGLE=75 -DMAP_ANGLE_DEPTH=75 -DMAP_SCALE=8"; do \
		cc -std=c99 -O2 -Wall -Wextra -Werror -fsanitize=address,undefined $$settings -Itests tests/test_example.c map.c -lm -o "$$test_binary"; \
		"$$test_binary"; \
	done

.PHONY: all clean execute test
