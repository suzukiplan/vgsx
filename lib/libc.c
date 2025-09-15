/**
 * VGS-X Runtime Library for MC68030
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

static volatile uint32_t _vsync;
static volatile uint32_t _random;
static uint32_t* _bg[4] = {
    VGS_VRAM_BG0,
    VGS_VRAM_BG1,
    VGS_VRAM_BG2,
    VGS_VRAM_BG3};
extern int main(int argc, char* argv[]);

void crt0(void)
{
    main(0, (char**)0);

    // Hang-up after return
    while (1) {
        vgs_vsync();
    }
}

void vgs_vsync(void)
{
    _vsync = *VGS_IN_VSYNC;
}

void vgs_srand(uint16_t seed)
{
    *VGS_IO_RANDOM = seed;
}

uint16_t vgs_rand(void)
{
    _random = *VGS_IO_RANDOM;
    return _random;
}

uint32_t vgs_rand32(void)
{
    uint32_t result = vgs_rand();
    result <<= 16;
    result |= vgs_rand();
    return result;
}

void vgs_console_print(const char* text)
{
    while (*text) {
        *VGS_OUT_CONSOLE = *text;
        text++;
    }
}

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
    *VGS_VREG_CLSA = value;
}

void vgs_cls_bg(uint8_t n, uint32_t value)
{
    VGS_VREG_CLS0[n & 3] = value;
}

void vgs_draw_pixel(uint8_t n, int32_t x, int32_t y, uint32_t col)
{
    *VGS_VREG_G_BG = n;
    *VGS_VREG_G_X1 = (uint32_t)x;
    *VGS_VREG_G_Y1 = (uint32_t)y;
    *VGS_VREG_G_COL = col;
    *VGS_VREG_G_EXE = VGS_DRAW_PIXEL;
}

void vgs_draw_line(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    *VGS_VREG_G_BG = n;
    *VGS_VREG_G_X1 = (uint32_t)x1;
    *VGS_VREG_G_Y1 = (uint32_t)y1;
    *VGS_VREG_G_X2 = (uint32_t)x2;
    *VGS_VREG_G_Y2 = (uint32_t)y2;
    *VGS_VREG_G_COL = col;
    *VGS_VREG_G_EXE = VGS_DRAW_LINE;
}

void vgs_draw_box(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    *VGS_VREG_G_BG = n;
    *VGS_VREG_G_X1 = (uint32_t)x1;
    *VGS_VREG_G_Y1 = (uint32_t)y1;
    *VGS_VREG_G_X2 = (uint32_t)x2;
    *VGS_VREG_G_Y2 = (uint32_t)y2;
    *VGS_VREG_G_COL = col;
    *VGS_VREG_G_EXE = VGS_DRAW_BOX;
}

void vgs_draw_boxf(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col)
{
    *VGS_VREG_G_BG = n;
    *VGS_VREG_G_X1 = (uint32_t)x1;
    *VGS_VREG_G_Y1 = (uint32_t)y1;
    *VGS_VREG_G_X2 = (uint32_t)x2;
    *VGS_VREG_G_Y2 = (uint32_t)y2;
    *VGS_VREG_G_COL = col;
    *VGS_VREG_G_EXE = VGS_DRAW_BOXF;
}

void vgs_draw_character(uint8_t n, int32_t x, int32_t y, int draw0, uint8_t pal, uint16_t ptn)
{
    *VGS_VREG_G_BG = n;
    *VGS_VREG_G_X1 = (uint32_t)x;
    *VGS_VREG_G_Y1 = (uint32_t)y;
    *VGS_VREG_G_OPT = (uint32_t)ptn;
    *VGS_VREG_G_COL = ((uint32_t)pal) | (draw0 ? 0x80000000 : 0);
    *VGS_VREG_G_EXE = VGS_DRAW_CHR;
}

void vgs_sprite(uint16_t n, uint8_t visible, int16_t x, int16_t y, uint8_t size, uint8_t pal, uint16_t ptn)
{
    n &= 0x3FF;
    VGS_OAM[n].visible = visible;
    VGS_OAM[n].x = x;
    VGS_OAM[n].y = y;
    VGS_OAM[n].size = size;
    VGS_OAM[n].attr = pal & 0x0F;
    VGS_OAM[n].attr <<= 16;
    VGS_OAM[n].attr |= ptn;
    VGS_OAM[n].rotate = 0;
    VGS_OAM[n].scale = 0;
}

void vgs_sprite_visible(uint16_t n, uint8_t visible)
{
    n &= 0x3FF;
    VGS_OAM[n].visible = visible;
}

void vgs_sprite_position(uint16_t n, int16_t x, int16_t y)
{
    n &= 0x3FF;
    VGS_OAM[n].x = x;
    VGS_OAM[n].y = y;
}

void vgs_sprite_size(uint16_t n, int8_t size)
{
    n &= 0x3FF;
    VGS_OAM[n].size = size;
}

void vgs_sprite_palette(uint16_t n, uint8_t pal)
{
    n &= 0x3FF;
    VGS_OAM[n].attr &= 0xFFF0FFFF;
    uint32_t w = pal;
    w &= 0x0F;
    w <<= 16;
    VGS_OAM[n].attr = w;
}

void vgs_sprite_pattern(uint16_t n, uint16_t ptn)
{
    VGS_OAM[n].attr &= 0xFFFF0000;
    VGS_OAM[n].attr |= ptn;
}

void vgs_sprite_flip(uint16_t n, int8_t h, int8_t v)
{
    VGS_OAM[n].attr &= 0x3FFFFFFF;
    VGS_OAM[n].attr |= h ? 0x80000000 : 0;
    VGS_OAM[n].attr |= v ? 0x40000000 : 0;
}

void vgs_music_play(uint16_t n)
{
    *VGS_OUT_VGM_PLAY = n;
}
