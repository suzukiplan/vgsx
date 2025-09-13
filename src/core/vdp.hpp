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

class VDP;
static inline void graphicDrawPixel(VDP* vdp);
static inline void graphicDrawLine(VDP* vdp);
static inline void graphicDrawBox(VDP* vdp);

class VDP
{
  public:
    typedef struct {
        uint32_t skip;                // R0: Skip Render
        uint32_t spos;                // R1: Sprite Position (0: Between BG0 and BG1 ~ 3)
        uint32_t scrollX[VDP_BG_NUM]; // R2-5: Scroll BGs X
        uint32_t scrollY[VDP_BG_NUM]; // R6-9: Scroll BGs Y
        uint32_t bmp[VDP_BG_NUM];     // R10-13: BGs Bitmap Mode
        uint32_t cls[VDP_BG_NUM + 1]; // R14-18: Clear Screen
        uint32_t g_bg;                // R19: Graphic Draw - BG number
        uint32_t g_x1;                // R20: Graphic Draw - X1
        uint32_t g_y1;                // R21: Graphic Draw - Y1
        uint32_t g_x2;                // R22: Graphic Draw - X2
        uint32_t g_y2;                // R23: Graphic Draw - Y2
        uint32_t g_col;               // R24: Graphic Draw - Color (RGB888)
        uint32_t g_opt;               // R25: Graphic Draw - Option
        uint32_t g_exe;               // R26: Graphic Draw - Execute
        uint32_t reserved[229];       // Reserved
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
        uint32_t nametbl[VDP_BG_NUM][65536];      // Name Table
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
            uint16_t addr = (address & 0x3FFFC) >> 2;
            return this->context.nametbl[n][addr];
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint8_t index = (address & 0b111111111100000) >> 6;
                    uint8_t arg = (address & 0b11100) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->context.oam[index];
                    return rawOam[arg];
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0x3C0) >> 6;
                    uint8_t cn = (address & 0x03C) >> 2;
                    return this->context.palette[pn][cn];
                }
                case 0xD20000: {
                    uint8_t index = (address & 0x3FC) >> 2;
                    uint32_t* rawReg = (uint32_t*)&this->context.reg;
                    switch (index) {
                        case 26: return this->readPixel();
                    }
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
            uint16_t addr = (address & 0x3FFFC) >> 2;
            this->context.nametbl[n][addr] = value;
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
                    uint8_t pn = (address & 0x3C0) >> 6;
                    uint8_t cn = (address & 0x03C) >> 2;
                    this->context.palette[pn][cn] = value;
                    return;
                }
                case 0xD20000: {
                    uint8_t index = (address & 0x3FC) >> 2;
                    uint32_t* rawReg = (uint32_t*)&this->context.reg;
                    rawReg[index] = value;
                    switch (index) {
                        case 2: this->bitmapScrollX(0, (int)value); break;
                        case 3: this->bitmapScrollX(1, (int)value); break;
                        case 4: this->bitmapScrollX(2, (int)value); break;
                        case 5: this->bitmapScrollX(3, (int)value); break;
                        case 6: this->bitmapScrollY(0, (int)value); break;
                        case 7: this->bitmapScrollY(1, (int)value); break;
                        case 8: this->bitmapScrollY(2, (int)value); break;
                        case 9: this->bitmapScrollY(3, (int)value); break;
                        case 14: this->cls(value); break;
                        case 15: this->cls(0, value); break;
                        case 16: this->cls(1, value); break;
                        case 17: this->cls(2, value); break;
                        case 18: this->cls(3, value); break;
                        case 26: this->graphicDraw(value); break;
                    }
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
    inline void cls(uint32_t value)
    {
        for (int i = 0; i < 4; i++) {
            this->cls(i, value);
        }
    }

    inline void cls(int n, uint32_t value)
    {
        n &= 3;
        uint8_t* cp = (uint8_t*)&value;
        if (cp[0] == cp[1] && cp[1] == cp[2] && cp[2] == cp[3]) {
            memset(this->context.nametbl[n], *cp, 0x40000);
        } else {
            for (int i = 0; i < 0x10000; i++) {
                this->context.nametbl[n][i] = value;
            }
        }
    }

    inline void graphicDraw(uint32_t op)
    {
        static void (*func[])(VDP*) = {
            graphicDrawPixel,
            graphicDrawLine,
            graphicDrawBox,
        };
        if (op < 3) {
            func[op](this);
        }
    }

    inline uint32_t readPixel()
    {
        int bg = this->context.reg.g_bg & 3;
        int x1 = (int)this->context.reg.g_x1;
        int y1 = (int)this->context.reg.g_y1;
        if (x1 < 0 || VDP_WIDTH <= x1 || y1 < 0 || VDP_HEIGHT <= y1) {
            return 0;
        } else {
            return this->context.nametbl[bg][y1 * VDP_WIDTH + x1];
        }
    }

    inline void bitmapScrollX(int bg, int vector)
    {
        if (!this->context.reg.bmp[bg]) {
            return; // Not Bitmap Mode
        }
        if (0 == vector) {
            return;
        }
        if (VDP_WIDTH <= vector || vector <= -VDP_WIDTH) {
            this->cls(bg, 0);
            return;
        }
        uint32_t* vram = this->context.nametbl[bg];
        if (vector < 0) {
            // left scroll
            vector = -vector;
            for (int i = 0; i < VDP_HEIGHT; i++) {
                memmove(vram, &vram[vector], (VDP_WIDTH - vector) * 4);
                memset(&vram[VDP_WIDTH - vector], 0, vector * 4);
                vram += VDP_WIDTH;
            }
        } else {
            // right scroll
            for (int i = 0; i < VDP_HEIGHT; i++) {
                memmove(&vram[vector], vram, (VDP_WIDTH - vector) * 4);
                memset(vram, 0, vector * 4);
                vram += VDP_WIDTH;
            }
        }
    }

    inline void bitmapScrollY(int bg, int vector)
    {
        if (!this->context.reg.bmp[bg]) {
            return; // Not Bitmap Mode
        }
        if (0 == vector) {
            return;
        }
        if (VDP_HEIGHT <= vector || vector <= -VDP_HEIGHT) {
            this->cls(bg, 0);
            return;
        }
        uint32_t* vram = this->context.nametbl[bg];
        if (vector < 0) {
            // upward scroll
            vector = -vector;
            memmove(vram, &vram[vector * VDP_WIDTH], (VDP_HEIGHT - vector) * VDP_WIDTH * 4);
            memset(&vram[(VDP_HEIGHT - vector) * VDP_WIDTH], 0, vector * VDP_WIDTH * 4);
        } else {
            // down scroll
            memmove(&vram[vector * VDP_WIDTH], vram, (VDP_HEIGHT - vector) * VDP_WIDTH * 4);
            memset(vram, 0, vector * VDP_WIDTH * 4);
        }
    }

    inline void renderBG(int n)
    {
        if (this->context.reg.bmp[n]) {
            // Bitmap Mode
            for (int dptr = 0; dptr < VDP_HEIGHT * VDP_WIDTH; dptr++) {
                uint32_t col = this->context.nametbl[n][dptr];
                if (col) {
                    this->context.display[dptr] = col;
                }
            }
        } else {
            // Character Pattern Mode
            int dptr = 0;
            for (int dy = 0; dy < VDP_HEIGHT; dy++) {
                auto wy = (dy + this->context.reg.scrollY[n]) & 0x7FF;
                for (int dx = 0; dx < VDP_WIDTH; dx++) {
                    auto wx = (dx + this->context.reg.scrollX[n]) & 0x7FF;
                    auto attr = this->context.nametbl[n][(((wy >> 3) & 0xFF) << 8) | (wx >> 3) & 0xFF];
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
    }

    inline void renderSprites()
    {
        // TODO
    }
};

static inline int _abs(int value) { return value < 0 ? -value : value; }
static inline int _sgn(int value) { return value < 0 ? -1 : 1; }

static inline void drawPixel(uint32_t* vram, int32_t x1, int32_t y1, uint32_t col)
{
    if (x1 < 0 || VDP_WIDTH <= x1 || y1 < 0 || VDP_HEIGHT <= y1) {
        return;
    }
    vram[y1 * VDP_WIDTH + x1] = col;
}

static inline void drawLine(uint32_t* vram, int32_t fx, int32_t fy, int32_t tx, int32_t ty, uint32_t col)
{
    int idx, idy;
    int ia, ib, ie;
    int w;
    idx = tx - fx;
    idy = ty - fy;
    if (!idx || !idy) {
        if (tx < fx) {
            w = fx;
            fx = tx;
            tx = w;
        }
        if (ty < fy) {
            w = fy;
            fy = ty;
            ty = w;
        }
        if (0 == idy) {
            for (; fx <= tx; fx++) {
                drawPixel(vram, fx, fy, col);
            }
        } else {
            for (; fy <= ty; fy++) {
                drawPixel(vram, fx, fy, col);
            }
        }
        return;
    }
    w = 1;
    ia = _abs(idx);
    ib = _abs(idy);
    if (ia >= ib) {
        ie = -_abs(idy);
        while (w) {
            drawPixel(vram, fx, fy, col);
            if (fx == tx) break;
            fx += _sgn(idx);
            ie += 2 * ib;
            if (ie >= 0) {
                fy += _sgn(idy);
                ie -= 2 * ia;
            }
        }
    } else {
        ie = -_abs(idx);
        while (w) {
            drawPixel(vram, fx, fy, col);
            if (fy == ty) break;
            fy += _sgn(idy);
            ie += 2 * ia;
            if (ie >= 0) {
                fx += _sgn(idx);
                ie -= 2 * ib;
            }
        }
    }
}

static inline void graphicDrawPixel(VDP* vdp)
{
    drawPixel(vdp->context.nametbl[vdp->context.reg.g_bg & 3],
              (int32_t)vdp->context.reg.g_x1,
              (int32_t)vdp->context.reg.g_y1,
              vdp->context.reg.g_col);
}

static inline void graphicDrawLine(VDP* vdp)
{
    drawLine(vdp->context.nametbl[vdp->context.reg.g_bg & 3],
             (int32_t)vdp->context.reg.g_x1,
             (int32_t)vdp->context.reg.g_y1,
             (int32_t)vdp->context.reg.g_x2,
             (int32_t)vdp->context.reg.g_y2,
             vdp->context.reg.g_col);
}

static inline void graphicDrawBox(VDP* vdp)
{
    uint32_t* vram = vdp->context.nametbl[vdp->context.reg.g_bg & 3];
    int32_t fx = (int32_t)vdp->context.reg.g_x1;
    int32_t fy = (int32_t)vdp->context.reg.g_y1;
    int32_t tx = (int32_t)vdp->context.reg.g_x2;
    int32_t ty = (int32_t)vdp->context.reg.g_y2;
    uint32_t col = vdp->context.reg.g_col;
    drawLine(vram, fx, fy, tx, fy, col);
    drawLine(vram, fx, fy, fx, ty, col);
    drawLine(vram, tx, ty, tx, fy, col);
    drawLine(vram, tx, ty, fx, ty, col);
}
