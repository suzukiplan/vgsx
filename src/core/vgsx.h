/**
 * VGS-X
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Yoji Suzuki.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "vdp.hpp"

class VGSX
{
  public:
    typedef struct {
        const uint8_t* data;
        size_t size;
    } Binary;

    typedef struct {
        const int16_t* data;
        size_t count;
        bool play;
        size_t index;
    } SfxData;

    struct Context {
        uint8_t ram[0x100000]; // WRAM (1MB)
        Binary vgmData[0x10000];
        SfxData sfxData[0x100];
        const uint8_t* elf;
        size_t elfSize;
        const uint8_t* program;
        size_t programSize;
        int randomIndex;
        uint32_t frameClocks;
    } context;

    struct KeyStatus {
        uint8_t up;
        uint8_t down;
        uint8_t left;
        uint8_t right;
        uint8_t a;
        uint8_t b;
        uint8_t x;
        uint8_t y;
        uint8_t start;
        int8_t axisX;
        int8_t axisY;
    } key;

    VDP vdp;
    void* vgmHelper;

    VGSX();
    ~VGSX();
    bool loadProgram(const void* data, size_t size);
    bool loadPattern(uint16_t index, const void* data, size_t size);
    bool loadPalette(const void* data, size_t size);
    bool loadVgm(uint16_t index, const void* data, size_t size);
    bool loadWav(uint8_t index, const void* data, size_t size);
    const char* getLastError() { return this->lastError; }
    void reset();
    void tick();
    void tickSound(int16_t* buf, int samples);
    inline uint32_t* getDisplay() { return this->vdp.context.display; }
    inline int getDisplayWidth() { return VDP_WIDTH; }
    inline int getDisplayHeight() { return VDP_HEIGHT; }
    uint32_t inPort(uint32_t address);
    void outPort(uint32_t address, uint32_t value);

  private:
    char lastError[256];
    void setLastError(const char* format, ...);
    volatile bool detectReferVSync;
};

extern VGSX vgsx;
