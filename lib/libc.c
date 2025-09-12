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
        attr &= 0xFFFF0000;
        attr |= *text;
        vgs_put_bg(n, x++, y, attr);
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
