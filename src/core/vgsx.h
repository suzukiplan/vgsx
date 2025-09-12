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
    struct Context {
        uint8_t ram[0x100000]; // WRAM (1MB)
        const uint8_t* elf;
        size_t elfSize;
        const uint8_t* program;
        size_t programSize;
        int randomIndex;
    } context;

    VDP vdp;

    VGSX();
    ~VGSX();
    bool loadProgram(const void* data, size_t size);
    bool loadPattern(uint16_t index, const void* data, size_t size);
    bool loadPalette(const void* data, size_t size);
    const char* getLastError() { return this->lastError; }
    void reset();
    void tick();
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
