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
#include <math.h>
#include <ctype.h>

#define VDP_BG_NUM 4   /* Number of the BG plan */
#define VDP_WIDTH 320  /* Width of the Screen */
#define VDP_HEIGHT 200 /* Height of the Screen */

extern "C" {
extern const uint8_t k8x12_jisx0201[3072];   // 12x256
extern const uint8_t k8x12_jisx0208[106032]; // 12x8836
};

class VDP;
static inline void graphicDrawPixel(VDP* vdp);
static inline void graphicDrawLine(VDP* vdp);
static inline void graphicDrawBox(VDP* vdp);
static inline void graphicDrawBoxFill(VDP* vdp);
static inline void graphicDrawCharacter(VDP* vdp);
static inline void graphicDrawJisX0201(VDP* vdp);
static inline void graphicDrawJisX0208(VDP* vdp);

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
        uint32_t skip0;               // R27: Skip Rendering BG0
        uint32_t skip1;               // R28: Skip Rendering BG1
        uint32_t skip2;               // R29: Skip Rendering BG2
        uint32_t skip3;               // R30: Skip Rendering BG3
        uint32_t pf_init;             // R31: Profotinaol Font - Initialize
        uint32_t pf_ptn;              // R32: Profotinaol Font - Pattern
        uint32_t pf_dx;               // R33: Profotinaol Font - diff X
        uint32_t pf_dy;               // R34: Profotinaol Font - diff Y
        uint32_t pf_width;            // R35: Profotinaol Font - width
        uint32_t reserved[220];       // Reserved
    } Register;

    typedef struct {
        uint32_t visible;     // Visible (0 or not 0)
        int32_t y;            // Position (Y)
        int32_t x;            // Position (X)
        uint32_t attr;        // Attribute
        uint32_t size;        // Size (0: 8x8, 1: 16x16, 2: 24x24, 3: 32x32 ... 31: 256x256)
        int32_t rotate;       // Rotate (-360 ~ 360)
        uint32_t scale;       // Scale (0: disabled, or 1 ~ 400 percent)
        uint32_t alpha;       // Alpha (0: disabled, or 0x000001 ~ 0xFFFFFF)
        uint32_t reserved[8]; // Reserved
    } OAM;

    typedef struct {
        int32_t dx;
        int32_t dy;
        int32_t width;
        int32_t reserved;
    } PropotionalInfo;

    struct Context {
        uint32_t display[VDP_WIDTH * VDP_HEIGHT]; // Virtual Display
        uint8_t ptn[65536][32];                   // Character Pattern (ROM)
        uint32_t nametbl[VDP_BG_NUM][65536];      // Name Table
        OAM oam[1024];                            // OAM
        uint32_t palette[16][16];                 // Palette
        PropotionalInfo pinfo[0x80];              // Propotional Info
        Register reg;                             // Register
    } ctx;

    void reset()
    {
        memset(this->ctx.display, 0, sizeof(this->ctx.display));
        memset(this->ctx.nametbl, 0, sizeof(this->ctx.nametbl));
        memset(this->ctx.oam, 0, sizeof(this->ctx.oam));
        memset(&this->ctx.reg, 0, sizeof(Register));
    }

    uint32_t read(uint32_t address)
    {
        if (0xC00000 <= address && address < 0xD00000) {
            uint8_t n = (address & 0xC0000) >> 18;
            uint16_t addr = (address & 0x3FFFC) >> 2;
            return this->ctx.nametbl[n][addr];
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint16_t index = (address & 0xFFC0) >> 6;
                    uint8_t arg = (address & 0x003C) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->ctx.oam[index];
                    return rawOam[arg];
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0x3C0) >> 6;
                    uint8_t cn = (address & 0x03C) >> 2;
                    return this->ctx.palette[pn][cn];
                }
                case 0xD20000: {
                    uint8_t index = (address & 0x3FC) >> 2;
                    uint32_t* rawReg = (uint32_t*)&this->ctx.reg;
                    switch (index) {
                        case 26: return this->readPixel();
                        case 33: return this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dx;
                        case 34: return this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dy;
                        case 35: return this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].width;
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
            this->ctx.nametbl[n][addr] = value;
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint16_t index = (address & 0xFFC0) >> 6;
                    uint8_t arg = (address & 0x003C) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->ctx.oam[index];
                    rawOam[arg] = value;
                    return;
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0x3C0) >> 6;
                    uint8_t cn = (address & 0x03C) >> 2;
                    this->ctx.palette[pn][cn] = value;
                    return;
                }
                case 0xD20000: {
                    uint8_t index = (address & 0x3FC) >> 2;
                    uint32_t* rawReg = (uint32_t*)&this->ctx.reg;
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
                        case 31: this->setupPropotional(value & 0xFFFF); break;
                        case 33: this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dx = value; break;
                        case 34: this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dy = value; break;
                        case 35: this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].width = value; break;
                    }
                    return;
                }
            }
        }
    }

    void render()
    {
        if (this->ctx.reg.skip) {
            return;
        }
        for (int i = 0; i < VDP_WIDTH * VDP_HEIGHT; i++) {
            this->ctx.display[i] = this->ctx.palette[0][0];
        }
        for (int i = 0; i < VDP_BG_NUM; i++) {
            if (0 == ((uint32_t*)&this->ctx.reg.skip0)[i]) {
                this->renderBG(i);
            }
            if (i == this->ctx.reg.spos) {
                this->renderSprites();
            }
        }
    }

  private:
    void setupPropotional(uint16_t ptn_start)
    {
        if (0x10000 - 0x80 < ptn_start) {
            return;
        }
        uint8_t* ptn = this->ctx.ptn[ptn_start];
        for (int i = 0; i < 0x80; i++) {
            int sx = 8;
            int ex = -1;
            for (int y = 0; y < 8; y++) {
                int px[8];
                px[0] = (ptn[0] & 0xF0) >> 4;
                px[1] = ptn[0] & 0x0F;
                px[2] = (ptn[1] & 0xF0) >> 4;
                px[3] = ptn[1] & 0x0F;
                px[4] = (ptn[2] & 0xF0) >> 4;
                px[5] = ptn[2] & 0x0F;
                px[6] = (ptn[3] & 0xF0) >> 4;
                px[7] = ptn[3] & 0x0F;
                ptn += 4;
                for (int x = 0; x < 8; x++) {
                    if (px[x]) {
                        if (x < sx) {
                            sx = x;
                        }
                    }
                    if (px[7 - x]) {
                        if (ex < 7 - x) {
                            ex = 7 - x;
                        }
                    }
                }
            }
            if (0 < ex) {
                this->ctx.pinfo[i].dx = -sx;
                this->ctx.pinfo[i].width = (ex - sx) + 2;
                switch (i) {
                    case '_':
                    case '.':
                    case ',':
                    case 'g':
                    case 'j':
                        this->ctx.pinfo[i].dy = 1;
                        break;
                    case 'p':
                    case 'q':
                    case 'y':
                        this->ctx.pinfo[i].dy = 2;
                        break;
                    default:
                        this->ctx.pinfo[i].dy = 0;
                }
            } else {
                this->ctx.pinfo[i].dx = 0;
                this->ctx.pinfo[i].dy = 0;
                if (i == ' ' || i == '\t') {
                    this->ctx.pinfo[i].width = 4; // space width
                } else {
                    this->ctx.pinfo[i].width = 0;
                }
            }
            // printf("%c: dx=%d, dy=%d, width=%d\n", isprint(i) ? i : '?', this->ctx.pinfo[i].dx, this->ctx.pinfo[i].dy, this->ctx.pinfo[i].width);
        }
    }

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
            memset(this->ctx.nametbl[n], *cp, 0x40000);
        } else {
            for (int i = 0; i < 0x10000; i++) {
                this->ctx.nametbl[n][i] = value;
            }
        }
    }

    inline void graphicDraw(uint32_t op)
    {
        static void (*func[])(VDP*) = {
            graphicDrawPixel,
            graphicDrawLine,
            graphicDrawBox,
            graphicDrawBoxFill,
            graphicDrawCharacter,
            graphicDrawJisX0201,
            graphicDrawJisX0208,
        };
        if (op < 7) {
            func[op](this);
        }
    }

    inline uint32_t readPixel()
    {
        int bg = this->ctx.reg.g_bg & 3;
        int x1 = (int)this->ctx.reg.g_x1;
        int y1 = (int)this->ctx.reg.g_y1;
        if (x1 < 0 || VDP_WIDTH <= x1 || y1 < 0 || VDP_HEIGHT <= y1) {
            return 0;
        } else {
            return this->ctx.nametbl[bg][y1 * VDP_WIDTH + x1];
        }
    }

    inline void bitmapScrollX(int bg, int vector)
    {
        if (!this->ctx.reg.bmp[bg]) {
            return; // Not Bitmap Mode
        }
        if (0 == vector) {
            return;
        }
        if (VDP_WIDTH <= vector || vector <= -VDP_WIDTH) {
            this->cls(bg, 0);
            return;
        }
        uint32_t* vram = this->ctx.nametbl[bg];
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
        if (!this->ctx.reg.bmp[bg]) {
            return; // Not Bitmap Mode
        }
        if (0 == vector) {
            return;
        }
        if (VDP_HEIGHT <= vector || vector <= -VDP_HEIGHT) {
            this->cls(bg, 0);
            return;
        }
        uint32_t* vram = this->ctx.nametbl[bg];
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
        if (this->ctx.reg.bmp[n]) {
            // Bitmap Mode
            for (int dptr = 0; dptr < VDP_HEIGHT * VDP_WIDTH; dptr++) {
                uint32_t col = this->ctx.nametbl[n][dptr];
                if (col) {
                    this->ctx.display[dptr] = col;
                }
            }
        } else {
            // Character Pattern Mode
            int dptr = 0;
            for (int dy = 0; dy < VDP_HEIGHT; dy++) {
                auto wy = (dy + this->ctx.reg.scrollY[n]) & 0x7FF;
                for (int dx = 0; dx < VDP_WIDTH; dx++) {
                    auto wx = (dx + this->ctx.reg.scrollX[n]) & 0x7FF;
                    auto attr = this->ctx.nametbl[n][(((wy >> 3) & 0xFF) << 8) | (wx >> 3) & 0xFF];
                    bool flipH = (attr & 0x80000000) ? true : false;
                    bool flipV = (attr & 0x40000000) ? true : false;
                    uint8_t pal = (attr & 0xF0000) >> 16;
                    uint8_t* ptn = this->ctx.ptn[attr & 0xFFFF];
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
                        this->ctx.display[dptr] = this->ctx.palette[pal][col];
                    }
                    dptr++;
                }
            }
        }
    }

    inline void renderSprites()
    {
        for (int i = 1023; 0 <= i; i--) {
            if (this->ctx.oam[i].visible) {
                renderSprite(&this->ctx.oam[i]);
            }
        }
    }

    inline void renderSprite(OAM* oam)
    {
        int psize = (oam->size & 0x1F) + 1; // 1 ~ 32
        int size = psize << 3;              // 8 ~ 256
        int ptn = oam->attr & 0xFFFF;
        int pal = (oam->attr & 0xF0000) >> 16;
        bool flipH = (oam->attr & 0x80000000) ? true : false;
        bool flipV = (oam->attr & 0x40000000) ? true : false;
        int32_t angle = 90 - oam->rotate;
        int scale = oam->scale < 400 ? oam->scale : 400;
        angle %= 360;
        if (angle < 0) {
            angle += 360;
        }
        if (90 == angle && (0 == scale || 100 == scale)) {
            // None-rotate & None-scale
            for (int dy = oam->y, py = 0; dy < oam->y + size; dy++, py++) {
                if (dy < 0) {
                    continue; // Out of screen top (check next line)
                }
                if (VDP_HEIGHT <= dy) {
                    break; // Out of screen bottom (end of rendering a sprite)
                }
                int wy = flipV ? size - py - 1 : py;
                for (int dx = oam->x, px = 0; dx < oam->x + size; dx++, px++) {
                    if (dx < 0) {
                        continue; // Out of screen left (check next pixel)
                    }
                    if (VDP_WIDTH <= dx) {
                        break; // Out of screen right (end of rendering a line)
                    }
                    // Render Pixel
                    int wx = flipH ? size - px - 1 : px;
                    uint8_t* ptnptr = this->ctx.ptn[(ptn + wx / 8 + (wy / 8) * psize) & 0xFFFF];
                    ptnptr += (wy & 0b0111) << 2;
                    ptnptr += (wx & 0b0110) >> 1;
                    int col;
                    if (wx & 1) {
                        col = (*ptnptr) & 0x0F;
                    } else {
                        col = ((*ptnptr) & 0xF0) >> 4;
                    }
                    if (col) {
                        this->renderSpritePixel(dy * VDP_WIDTH + dx, this->ctx.palette[pal][col], oam->alpha);
                    }
                }
            }
        } else if (90 == angle && 0 != scale && 100 != scale) {
            // Scale & None-rotate
            int scaledSize = size * scale / 100;
            double ratio = size;
            ratio /= scaledSize;
            int offset = (size - scaledSize) / 2;
            for (int dy = oam->y + offset, by = 0; by < scaledSize; dy++, by++) {
                if (dy < 0) {
                    continue; // Out of screen top (check next line)
                }
                if (VDP_HEIGHT <= dy) {
                    break; // Out of screen bottom (end of rendering a sprite)
                }
                int py = (int)(by * ratio);
                int wy = flipH ? size - py - 1 : py;
                for (int dx = oam->x + offset, bx = 0; bx < scaledSize; dx++, bx++) {
                    if (dx < 0) {
                        continue; // Out of screen left (check next pixel)
                    }
                    if (VDP_WIDTH <= dx) {
                        break; // Out of screen right (end of rendering a line)
                    }
                    // this->ctx.display[dy * VDP_WIDTH + dx] = 0xFF0000;
                    int px = (int)(bx * ratio);
                    int wx = flipH ? size - px - 1 : px;
                    // Render Pixel
                    uint8_t* ptnptr = this->ctx.ptn[(ptn + wx / 8 + (wy / 8) * psize) & 0xFFFF];
                    ptnptr += (wy & 0b0111) << 2;
                    ptnptr += (wx & 0b0110) >> 1;
                    int col;
                    if (wx & 1) {
                        col = (*ptnptr) & 0x0F;
                    } else {
                        col = ((*ptnptr) & 0xF0) >> 4;
                    }
                    if (col) {
                        this->renderSpritePixel(dy * VDP_WIDTH + dx, this->ctx.palette[pal][col], oam->alpha);
                    }
                }
            }
        } else if (0 == scale || 100 == scale) {
            // Rotate & None-scale
            int halfSize = size / 2;
            double rad = angle * M_PI / 180.0;
            for (int py = 0; py < size; py++) {
                int wy = flipV ? size - py - 1 : py;
                for (int px = 0; px < size; px++) {
                    int dy = oam->y + (py - halfSize) * sin(rad) + (px - halfSize) * cos(rad) + halfSize;
                    if (dy < 0 || VDP_HEIGHT <= dy) {
                        continue; // Out of screen top (check next line)
                    }
                    int dx = oam->x + (px - halfSize) * sin(rad) - (py - halfSize) * cos(rad) + halfSize;
                    if (dx < 0 || VDP_WIDTH <= dx) {
                        continue; // Out of screen left (check next pixel)
                    }
                    // Render Pixel
                    int wx = flipH ? size - px - 1 : px;
                    uint8_t* ptnptr = this->ctx.ptn[(ptn + wx / 8 + (wy / 8) * psize) & 0xFFFF];
                    ptnptr += (wy & 0b0111) << 2;
                    ptnptr += (wx & 0b0110) >> 1;
                    int col;
                    if (wx & 1) {
                        col = (*ptnptr) & 0x0F;
                    } else {
                        col = ((*ptnptr) & 0xF0) >> 4;
                    }
                    if (col) {
                        int ptr = dy * VDP_WIDTH + dx;
                        this->renderSpritePixel(ptr, this->ctx.palette[pal][col], oam->alpha);
                        if (dx < 319) {
                            this->renderSpritePixel(ptr + 1, this->ctx.palette[pal][col], oam->alpha);
                        }
                    }
                }
            }
        } else {
            // Scale & Rotate
            int scaledSize = size * scale / 100;
            double ratio = size;
            ratio /= scaledSize;
            int offset = (size - scaledSize) / 2;
            int halfSize = scaledSize / 2;
            double rad = angle * M_PI / 180.0;
            for (int dy = oam->y + offset, by = 0; by < scaledSize; dy++, by++) {
                int py = (int)(by * ratio);
                int wy = flipH ? size - py - 1 : py;
                for (int dx = oam->x + offset, bx = 0; bx < scaledSize; dx++, bx++) {
                    // this->ctx.display[dy * VDP_WIDTH + dx] = 0xFF0000;
                    int px = (int)(bx * ratio);
                    int wx = flipH ? size - px - 1 : px;
                    int ddy = (by - halfSize) * sin(rad) + (bx - halfSize) * cos(rad) + halfSize;
                    ddy += oam->y + offset;
                    if (ddy < 0 || VDP_HEIGHT <= ddy) {
                        continue; // Out of screen top (check next line)
                    }
                    int ddx = (bx - halfSize) * sin(rad) - (by - halfSize) * cos(rad) + halfSize;
                    ddx += oam->x + offset;
                    if (ddx < 0 || VDP_WIDTH <= ddx) {
                        continue; // Out of screen left (check next pixel)
                    }
                    // Render Pixel
                    uint8_t* ptnptr = this->ctx.ptn[(ptn + wx / 8 + (wy / 8) * psize) & 0xFFFF];
                    ptnptr += (wy & 0b0111) << 2;
                    ptnptr += (wx & 0b0110) >> 1;
                    int col;
                    if (wx & 1) {
                        col = (*ptnptr) & 0x0F;
                    } else {
                        col = ((*ptnptr) & 0xF0) >> 4;
                    }
                    if (col) {
                        this->renderSpritePixel(ddy * VDP_WIDTH + ddx, this->ctx.palette[pal][col], oam->alpha);
                        if (ddy < 199) {
                            this->renderSpritePixel((ddy + 1) * VDP_WIDTH + ddx, this->ctx.palette[pal][col], oam->alpha);
                        }
                        if (ddx < 319) {
                            this->renderSpritePixel(ddy * VDP_WIDTH + ddx + 1, this->ctx.palette[pal][col], oam->alpha);
                        }
                        if (ddy < 199 && ddx < 319) {
                            this->renderSpritePixel((ddy + 1) * VDP_WIDTH + ddx + 1, this->ctx.palette[pal][col], oam->alpha);
                        }
                    }
                }
            }
        }
    }

    inline void renderSpritePixel(int displayAddress, uint32_t color, uint32_t alpha)
    {
        if (0 == alpha || 0xFFFFFF == (alpha & 0xFFFFFF)) {
            this->ctx.display[displayAddress] = color;
            return;
        }
        uint32_t src = this->ctx.display[displayAddress];
        uint32_t ar = (alpha & 0xFF0000) >> 16;
        uint32_t ag = (alpha & 0x00FF00) >> 8;
        uint32_t ab = alpha & 0x0000FF;
        uint32_t sr = (src & 0xFF0000) >> 16;
        uint32_t sg = (src & 0x00FF00) >> 8;
        uint32_t sb = src & 0x0000FF;
        uint32_t dr = (color & 0xFF0000) >> 16;
        uint32_t dg = (color & 0x00FF00) >> 8;
        uint32_t db = color & 0x0000FF;
        sr *= ar;
        sg *= ag;
        sb *= ab;
        dr *= (255 - ar);
        dg *= (255 - ag);
        db *= (255 - ab);
        uint32_t color2 = ((sr + dr) & 0x00FF00) << 8;
        color2 |= (sg + dg) & 0x00FF00;
        color2 |= ((sb + db) & 0x00FF00) >> 8;
        this->ctx.display[displayAddress] = color2;
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
    drawPixel(vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3],
              (int32_t)vdp->ctx.reg.g_x1,
              (int32_t)vdp->ctx.reg.g_y1,
              vdp->ctx.reg.g_col);
}

static inline void graphicDrawLine(VDP* vdp)
{
    drawLine(vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3],
             (int32_t)vdp->ctx.reg.g_x1,
             (int32_t)vdp->ctx.reg.g_y1,
             (int32_t)vdp->ctx.reg.g_x2,
             (int32_t)vdp->ctx.reg.g_y2,
             vdp->ctx.reg.g_col);
}

static inline void graphicDrawBox(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t fx = (int32_t)vdp->ctx.reg.g_x1;
    int32_t fy = (int32_t)vdp->ctx.reg.g_y1;
    int32_t tx = (int32_t)vdp->ctx.reg.g_x2;
    int32_t ty = (int32_t)vdp->ctx.reg.g_y2;
    uint32_t col = vdp->ctx.reg.g_col;
    drawLine(vram, fx, fy, tx, fy, col);
    drawLine(vram, fx, fy, fx, ty, col);
    drawLine(vram, tx, ty, tx, fy, col);
    drawLine(vram, tx, ty, fx, ty, col);
}

static inline void graphicDrawBoxFill(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t fx = (int32_t)vdp->ctx.reg.g_x1;
    int32_t fy = (int32_t)vdp->ctx.reg.g_y1;
    int32_t tx = (int32_t)vdp->ctx.reg.g_x2;
    int32_t ty = (int32_t)vdp->ctx.reg.g_y2;
    uint32_t col = vdp->ctx.reg.g_col;
    if (ty < fy) {
        int w = fy;
        fy = ty;
        ty = w;
    }
    for (int32_t y = fy; y < ty; y++) {
        drawLine(vram, tx, y, fx, y, col);
    }
}

static inline void graphicDrawCharacter(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t x = (int32_t)vdp->ctx.reg.g_x1;
    int32_t y = (int32_t)vdp->ctx.reg.g_y1;
    uint8_t pal = (int32_t)vdp->ctx.reg.g_col & 0x0F;
    uint8_t* ptn = vdp->ctx.ptn[vdp->ctx.reg.g_opt & 0xFFFF];
    bool drawZero = vdp->ctx.reg.g_col & 0x80000000 ? true : false;

    if (x < -8 || y < -8 || 320 <= x || 200 <= y) {
        return;
    }
    for (int i = 0; i < 8; i++) {
        if (y + i < 0 || 200 <= y + i) {
            continue;
        }
        for (int j = 0; j < 4; j++) {
            int p0 = (ptn[i * 4 + j] & 0xF0) >> 4;
            int p1 = ptn[i * 4 + j] & 0x0F;
            uint32_t c0 = vdp->ctx.palette[pal][p0];
            uint32_t c1 = vdp->ctx.palette[pal][p1];
            if (0 <= x + j * 2 && x + j * 2 < 320) {
                if (p0) {
                    vram[(y + i) * 320 + x + j * 2] = c0;
                } else if (drawZero) {
                    vram[(y + i) * 320 + x + j * 2] = 0;
                }
            }
            if (0 <= x + j * 2 + 1 && x + j * 2 + 1 < 320) {
                if (p1) {
                    vram[(y + i) * 320 + x + j * 2 + 1] = c1;
                } else if (drawZero) {
                    vram[(y + i) * 320 + x + j * 2 + 1] = 0;
                }
            }
        }
    }
}

static inline void graphicDrawJisX0201(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t x = (int32_t)vdp->ctx.reg.g_x1;
    int32_t y = (int32_t)vdp->ctx.reg.g_y1;
    uint32_t col = vdp->ctx.reg.g_col;
    uint32_t code = (int32_t)vdp->ctx.reg.g_opt;
    if (255 < code) {
        return;
    }
    const uint8_t* ptn = &k8x12_jisx0201[code * 12];
    for (int iy = 0; iy < 12; iy++) {
        int p = ptn[iy];
        if (y + iy < 0 || 200 <= y + iy) {
            continue;
        }
        for (int ix = 0; ix < 4; ix++) {
            if (x + ix < 0 || 320 <= x + ix) {
                continue;
            }
            if (p & 0x80) {
                vram[(y + iy) * 320 + x + ix] = col;
            }
            p <<= 1;
        }
    }
}

static inline void graphicDrawJisX0208(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t x = (int32_t)vdp->ctx.reg.g_x1;
    int32_t y = (int32_t)vdp->ctx.reg.g_y1;
    uint32_t col = vdp->ctx.reg.g_col;
    uint32_t code = (int32_t)vdp->ctx.reg.g_opt;
    printf("jisx0208: 0x%04X\n", code);
    if (8835 < code) {
        return;
    }
    const uint8_t* ptn = &k8x12_jisx0208[code * 12];
    for (int iy = 0; iy < 12; iy++) {
        int p = ptn[iy];
        if (y + iy < 0 || 200 <= y + iy) {
            continue;
        }
        for (int ix = 0; ix < 8; ix++) {
            if (x + ix < 0 || 320 <= x + ix) {
                continue;
            }
            if (p & 0x80) {
                vram[(y + iy) * 320 + x + ix] = col;
            }
            p <<= 1;
        }
    }
}
