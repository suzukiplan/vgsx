CXX ?= g++
CC ?= gcc

BUILD_DIR := build

CXXFLAGS ?= -std=c++17 -O2 -I../../src -Wall -Wextra
CFLAGS ?= -O2 -I../../src -Wall -Wextra

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

DEPS := $(OBJS:.o=.d)

all: test_io

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(BUILD_DIR)/test_io.o: test_io.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/vgsx.o: ../../src/vgsx.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/musashi.o: ../../src/musashi.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/vgs0math.o: ../../src/vgs0math.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/utf8_to_sjis.o: ../../src/utf8_to_sjis.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/bios.o: ../../src/bios.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/k8x12_jisx0201.o: ../../src/k8x12_jisx0201.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

$(BUILD_DIR)/k8x12_jisx0208.o: ../../src/k8x12_jisx0208.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) $(DEPFLAGS) -c $< -o $@

test_io: $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $@

-include $(DEPS)

clean:
	rm -rf $(BUILD_DIR) test_io

.PHONY: all clean
