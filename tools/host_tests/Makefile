CXX ?= g++
CC ?= gcc

BUILD_DIR := build

CXXFLAGS ?= -std=c++17 -O2 -I../../src
CFLAGS ?= -O2 -I../../src

DEPFLAGS = -MMD -MP -MF $(@:.o=.d) -MT $@

OBJS := \
	$(BUILD_DIR)/test_io.o \
	$(BUILD_DIR)/vgsx.o \
	$(BUILD_DIR)/musashi.o \
	$(BUILD_DIR)/vgs0math.o \
	$(BUILD_DIR)/utf8_to_sjis.o \
	$(BUILD_DIR)/bios.o \
	$(BUILD_DIR)/k8x12_jisx0201.o \
	$(BUILD_DIR)/k8x12_jisx0208.o

DEPS := $(BUILD_DIR)/test_vgm.d $(OBJS:.o=.d) $(BUILD_DIR)/benchmark_sprites.d

all: test_io $(BUILD_DIR)/test_vgm
	./test_io
	./$(BUILD_DIR)/test_vgm

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_io.o: test_io.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/vgsx.o: ../../src/vgsx.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/musashi.o: ../../src/musashi.cpp | $(BUILD_DIR)
	$(CXX) -w $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/vgs0math.o: ../../src/vgs0math.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/utf8_to_sjis.o: ../../src/utf8_to_sjis.cpp | $(BUILD_DIR)
	$(CXX) -w $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/bios.o: ../../src/bios.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/k8x12_jisx0201.o: ../../src/k8x12_jisx0201.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/k8x12_jisx0208.o: ../../src/k8x12_jisx0208.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

test_io: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

-include $(DEPS)

benchmark: benchmark_sprites
	./benchmark_sprites

$(BUILD_DIR)/benchmark_sprites.o: benchmark_sprites.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

benchmark_sprites: $(BUILD_DIR)/benchmark_sprites.o $(BUILD_DIR)/vgs0math.o
	$(CXX) $(CXXFLAGS) $^ -o $@

clean:
	rm -rf $(BUILD_DIR) test_io benchmark_sprites

.PHONY: all clean benchmark

$(BUILD_DIR)/test_vgm.o: test_vgm.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/test_vgm: $(BUILD_DIR)/test_vgm.o
	$(CXX) $(CXXFLAGS) -pthread $^ -o $@

# Optional cross-toolchain check, separate from host-only regression tests.
build/bgm_api.elf: bgm_api.c ../../lib/vgs_bgm.h ../../lib/vgs_io.h | $(BUILD_DIR)
	m68k-elf-gcc -m68030 -O2 -nostdlib -I../../lib -T../../lib/linker.ld -Wl,-etest_entry $< -o $@

check-m68k: test_io build/bgm_api.elf
	./test_io build/bgm_api.elf

.PHONY: check-m68k
