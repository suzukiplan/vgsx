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

void vgs_k8x12_print(uint8_t n, int32_t x, int32_t y, uint32_t col, const char* sjis)
{
    VGS_VREG_G_BG = n & 3;
    VGS_VREG_G_Y1 = y;
    VGS_VREG_G_COL = col;
    while (*sjis) {
        VGS_VREG_G_X1 = x;
        uint16_t h = (uint8_t)*sjis;
        if ((0x80 < h && h < 0xA0) || (0xE0 <= h && h < 0xF0)) {
            uint8_t l = (uint8_t)sjis[1];
            if (0 == l) {
                return;
            }
            h -= h < 0x9F ? 0x71 : 0xB1;
            h <<= 1;
            h++;
            if (l >= 0x7F) {
                l -= 0x01;
            }
            if (l >= 0x9E) {
                l -= 0x7D;
                h++;
            } else {
                l -= 0x1f;
            }
            h = (h - 0x21) * 94;
            h += l - 0x21;
            VGS_VREG_G_OPT = h;
            VGS_VREG_G_EXE = VGS_DRAW_JISX0208;
            x += 8;
            sjis += 2;
        } else {
            VGS_VREG_G_OPT = h;
            VGS_VREG_G_EXE = VGS_DRAW_JISX0201;
            x += 4;
            sjis++;
        }
    }
}
