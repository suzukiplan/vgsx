/**
 * VGS Standard Library for MC68030
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
#include <vgs.h>

static uint32_t* _bg[4] = {
    BG0,
    BG1,
    BG2,
    BG3};

void vgs_put_bg(uint8_t n, uint8_t x, uint8_t y, uint32_t data)
{
    uint16_t ptr = y;
    ptr <<= 8;
    ptr |= x;
    _bg[n & 3][ptr] = data;
}

void vgs_print_bg(uint8_t n, uint8_t x, uint8_t y, uint8_t pal, const char* text)
{
    uint32_t attr;
    attr = pal;
    attr <<= 16;
    while (*text) {
        vgs_put_bg(n, x++, y, attr | *text);
        text++;
    }
}

void vgs_cls_bg_all(uint32_t value)
{
    VGS_VREG_CLSA = value;
}

void vgs_cls_bg(uint8_t n, uint32_t value)
{
    VGS_VREG_CLS[n & 3] = value;
}

uint32_t vgs_read_pixel(uint8_t n, int32_t x, int32_t y)
{
    VGS_VREG_G_BG = n;
    VGS_VREG_G_X1 = (uint32_t)x;
    VGS_VREG_G_Y1 = (uint32_t)y;
    return VGS_VREG_G_EXE;
}

void vgs_draw_pixel(uint8_t n, int32_t x, int32_t y, uint32_t col)
{
    VGS_VREG_G_BG = n;
    VGS_VREG_G_X1 = (uint32_t)x;
    VGS_VREG_G_Y1 = (uint32_t)y;
    VGS_VREG_G_COL = col;
    VGS_VREG_G_EXE = VGS_DRAW_PIXEL;
}

void vgs_draw_line(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    VGS_VREG_G_BG = n;
    VGS_VREG_G_X1 = (uint32_t)x1;
    VGS_VREG_G_Y1 = (uint32_t)y1;
    VGS_VREG_G_X2 = (uint32_t)x2;
    VGS_VREG_G_Y2 = (uint32_t)y2;
    VGS_VREG_G_COL = col;
    VGS_VREG_G_EXE = VGS_DRAW_LINE;
}

void vgs_draw_box(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    VGS_VREG_G_BG = n;
    VGS_VREG_G_X1 = (uint32_t)x1;
    VGS_VREG_G_Y1 = (uint32_t)y1;
    VGS_VREG_G_X2 = (uint32_t)x2;
    VGS_VREG_G_Y2 = (uint32_t)y2;
    VGS_VREG_G_COL = col;
    VGS_VREG_G_EXE = VGS_DRAW_BOX;
}

void vgs_draw_boxf(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    VGS_VREG_G_BG = n;
    VGS_VREG_G_X1 = (uint32_t)x1;
    VGS_VREG_G_Y1 = (uint32_t)y1;
    VGS_VREG_G_X2 = (uint32_t)x2;
    VGS_VREG_G_Y2 = (uint32_t)y2;
    VGS_VREG_G_COL = col;
    VGS_VREG_G_EXE = VGS_DRAW_BOXF;
}

void vgs_draw_character(uint8_t n, int32_t x, int32_t y, BOOL draw0, uint8_t pal, uint16_t ptn)
{
    VGS_VREG_G_BG = n;
    VGS_VREG_G_X1 = (uint32_t)x;
    VGS_VREG_G_Y1 = (uint32_t)y;
    VGS_VREG_G_OPT = (uint32_t)ptn;
    VGS_VREG_G_COL = ((uint32_t)pal) | (draw0 ? 0x80000000 : 0);
    VGS_VREG_G_EXE = VGS_DRAW_CHR;
}

void vgs_sprite(uint16_t n, BOOL visible, int16_t x, int16_t y, uint8_t size, uint8_t pal, uint16_t ptn)
{
    n &= 0x3FF;
    OAM[n].visible = visible;
    OAM[n].x = x;
    OAM[n].y = y;
    OAM[n].size = size;
    OAM[n].attr = pal & 0x0F;
    OAM[n].attr <<= 16;
    OAM[n].attr |= ptn;
    OAM[n].rotate = 0;
    OAM[n].scale = 0;
}

void vgs_pfont_print(uint8_t n, int32_t x, int32_t y, uint8_t pal, uint16_t ptn, const char* text)
{
    int32_t dx, dy, width;
    for (; *text && x < VRAM_WIDTH; text++) {
        uint8_t c = (uint8_t)*text;
        if (c < 0x80) {
            vgs_pfont_get(c, &dx, &dy, &width);
        } else {
            width = 0;
        }
        if (0 < width) {
            vgs_draw_character(n, x + dx, y + dy, FALSE, pal, ptn + c);
            x += width;
        } else {
            vgs_draw_character(n, x, y, FALSE, pal, ptn + c);
            x += 8;
        }
    }
}

int32_t vgs_pfont_strlen(const char* text)
{
    int32_t result = 0;
    int32_t dx, dy, width;
    for (; *text; text++) {
        uint8_t c = (uint8_t)*text;
        if (c < 0x80) {
            vgs_pfont_get(c, &dx, &dy, &width);
        } else {
            width = 0;
        }
        if (0 < width) {
            result += width;
        } else {
            result += 8;
        }
    }
    return 0 < result ? result - 1 : result;
}
