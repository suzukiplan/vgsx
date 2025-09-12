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
#include <string.h>

#define VDP_BG_NUM 4   /* Number of the BG plan */
#define VDP_WIDTH 320  /* Width of the Screen */
#define VDP_HEIGHT 200 /* Height of the Screen */

class VDP
{
  public:
    typedef struct {
        uint32_t skip;                // Skip Render
        uint32_t spos;                // Sprite Position (0: Between BG0 and BG1 ~ 3)
        uint32_t scrollX[VDP_BG_NUM]; // Scroll BGs X
        uint32_t scrollY[VDP_BG_NUM]; // Scroll BGs Y
        uint32_t reserved[246];       // Reserved
    } Register;

    typedef struct {
        uint32_t hidden;      // Hidden (0 or not 0)
        int32_t y;            // Position (Y)
        int32_t x;            // Position (X)
        uint32_t attr;        // Attribute
        uint32_t reserved[4]; // Reserved
    } OAM;

    struct Context {
        uint32_t display[VDP_WIDTH * VDP_HEIGHT]; // Virtual Display
        uint8_t ptn[65536][32];                   // Character Pattern (ROM)
        uint32_t nametbl[VDP_BG_NUM][256][256];   // Name Table
        OAM oam[1024];                            // OAM
        uint32_t palette[16][16];                 // Palette
        Register reg;                             // Register
    } context;

    void reset()
    {
        memset(this->context.display, 0, sizeof(this->context.display));
        memset(this->context.nametbl, 0, sizeof(this->context.nametbl));
        memset(this->context.oam, 0, sizeof(this->context.oam));
        memset(&this->context.reg, 0, sizeof(Register));
    }

    uint32_t read(uint32_t address)
    {
        if (0xC00000 <= address && address < 0xD00000) {
            uint8_t n = (address & 0xC0000) >> 18;
            uint8_t y = (address & 0x3FC00) >> 10;
            uint8_t x = (address & 0x003FC) >> 2;
            return this->context.nametbl[n][y][x];
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint8_t index = (address & 0b111111111100000) >> 6;
                    uint8_t arg = (address & 0b11100) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->context.oam[index];
                    return rawOam[arg];
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0xF00) >> 8;
                    uint8_t cn = (address & 0x0F0) >> 4;
                    return this->context.palette[pn][cn];
                }
                case 0xD20000: {
                    uint8_t index = (address & 0xFF0) >> 4;
                    uint32_t* rawReg = (uint32_t*)&this->context.reg;
                    return rawReg[index];
                }
            }
        }
        return 0xFFFFFFFF;
    }

    void write(uint32_t address, uint32_t value)
    {
        if (0xC00000 <= address && address < 0xD00000) {
            uint8_t n = (address & 0xC0000) >> 18;
            uint8_t y = (address & 0x3FC00) >> 10;
            uint8_t x = (address & 0x003FC) >> 2;
            this->context.nametbl[n][y][x] = value;
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint8_t index = (address & 0b111111111100000) >> 6;
                    uint8_t arg = (address & 0b11100) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->context.oam[index];
                    rawOam[arg] = value;
                    return;
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0xF00) >> 8;
                    uint8_t cn = (address & 0x0F0) >> 4;
                    this->context.palette[pn][cn] = value;
                    return;
                }
                case 0xD20000: {
                    uint8_t index = (address & 0xFF0) >> 4;
                    uint32_t* rawReg = (uint32_t*)&this->context.reg;
                    rawReg[index] = value;
                    return;
                }
            }
        }
    }

    void render()
    {
        if (this->context.reg.skip) {
            return;
        }
        for (int i = 0; i < VDP_WIDTH * VDP_HEIGHT; i++) {
            this->context.display[i] = this->context.palette[0][0];
        }
        for (int i = 0; i < VDP_BG_NUM; i++) {
            this->renderBG(i);
            if (i == this->context.reg.spos) {
                this->renderSprites();
            }
        }
    }

  private:
    void renderBG(int n)
    {
        int dptr = 0;
        for (int dy = 0; dy < VDP_HEIGHT; dy++) {
            auto wy = (dy + this->context.reg.scrollY[n]) & 0x7FF;
            for (int dx = 0; dx < VDP_WIDTH; dx++) {
                auto wx = (dx + this->context.reg.scrollX[n]) & 0x7FF;
                auto attr = this->context.nametbl[n][(wy >> 3) & 0xFF][(wx >> 3) & 0xFF];
                bool flipH = (attr & 0x80000000) ? true : false;
                bool flipV = (attr & 0x40000000) ? true : false;
                uint8_t pal = (attr & 0xF0000) >> 16;
                uint8_t* ptn = this->context.ptn[attr & 0xFFFF];
                if (flipV) {
                    ptn += (7 - (wy & 0b111)) << 2;
                } else {
                    ptn += (wy & 0b111) << 2;
                }
                if (flipH) {
                    ptn += (7 - (wx & 0b111)) >> 1;
                } else {
                    ptn += (wx & 0b111) >> 1;
                }
                uint8_t col = *ptn;
                if (flipH) {
                    if (wx & 1) {
                        col &= 0xF0;
                        col >>= 4;
                    } else {
                        col &= 0x0F;
                    }
                } else {
                    if (wx & 1) {
                        col &= 0x0F;
                    } else {
                        col &= 0xF0;
                        col >>= 4;
                    }
                }
                if (col) {
                    this->context.display[dptr] = this->context.palette[pal][col];
                }
                dptr++;
            }
        }
    }

    void renderSprites()
    {
        // TODO
    }
};
