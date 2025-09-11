/**
 * VGS-X Video Display Processor
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
#include <stdint.h>

class VDP
{
  public:
    typedef struct {
        uint32_t skip;       // Skip Render
        uint32_t spos;       // Sprite Position (0: Between BG0 and BG1 ~ 3)
        uint32_t scrollX[4]; // Scroll BGs X
        uint32_t scrollY[4]; // Scroll BGs Y
    } Register;
    typedef struct {
        uint32_t hidden;      // Hidden (0 or not 0)
        int32_t y;            // Position (Y)
        int32_t x;            // Position (X)
        uint32_t attr;        // Attribute
        uint32_t reserved[4]; // Reserved
    } OAM;
    struct Context {
        uint8_t ptn[65536][32];        // Character Pattern (ROM)
        uint32_t nametbl[4][256][256]; // Name Table
        OAM oam[1024];                 // OAM
        uint32_t palette[16][16];      // Palette
        Register reg;                  // Register
    } context;

    uint32_t read(uint32_t address)
    {
        if (0xC00000 <= address && address < 0xD00000) {
            uint8_t n = (address & 0x300000) >> 20;
            uint8_t y = (address & 0x0FF000) >> 12;
            uint8_t x = (address & 0x000FF0) >> 4;
            return this->context.nametbl[n][y][x];
        }
        return 0xFFFFFFFF;
    }

    void write(uint32_t address, uint32_t value)
    {
    }
};
