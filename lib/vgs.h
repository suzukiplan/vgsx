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
#pragma once
#include <stdint.h>

typedef struct {
    uint32_t visible;     // Visible (0 or not 0)
    int32_t y;            // Position (Y)
    int32_t x;            // Position (X)
    uint32_t attr;        // Attribute
    uint32_t size;        // Size (0: 8x8, 1: 16x16, 2: 24x24, 3: 32x32 ... 31: 256x256)
    int32_t rotate;       // Rotate (-360 ~ 360)
    uint32_t scale;       // Scale (0: disabled or 1 ~ 400 percent)
    uint32_t reserved[9]; // Reserved
} OAM;

// Name table (256x256)
// Bit Layout:
// - attribute (12bit): F/H F/V 0 0 - 0 0 0 0 - 0 0 0 0
//   - F/H: Flip Horizontal
//   - F/V: Flip Vertical
// - palette (4bit)
// - pattern number (16bit)
#define VGS_VRAM_BG0 ((uint32_t*)0xC00000)
#define VGS_VRAM_BG1 ((uint32_t*)0xC40000)
#define VGS_VRAM_BG2 ((uint32_t*)0xC80000)
#define VGS_VRAM_BG3 ((uint32_t*)0xCC0000)

// Sprites
#define VGS_OAM_MAX 1024
#define VGS_OAM ((OAM*)0xD00000)

// Palette Table
// 16 x 16 x 2bits
#define VGS_PALETTE ((uint32_t*)0xD10000)

// VDP Register
#define VGS_VREG_SKIP *((uint32_t*)0xD20000)
#define VGS_VREG_SPOS *((uint32_t*)0xD20004)
#define VGS_VREG_SX0 *((uint32_t*)0xD20008)
#define VGS_VREG_SX1 *((uint32_t*)0xD2000C)
#define VGS_VREG_SX2 *((uint32_t*)0xD20010)
#define VGS_VREG_SX3 *((uint32_t*)0xD20014)
#define VGS_VREG_SY0 *((uint32_t*)0xD20018)
#define VGS_VREG_SY1 *((uint32_t*)0xD2001C)
#define VGS_VREG_SY2 *((uint32_t*)0xD20020)
#define VGS_VREG_SY3 *((uint32_t*)0xD20024)
#define VGS_VREG_BMP0 *((uint32_t*)0xD20028)
#define VGS_VREG_BMP1 *((uint32_t*)0xD2002C)
#define VGS_VREG_BMP2 *((uint32_t*)0xD20030)
#define VGS_VREG_BMP3 *((uint32_t*)0xD20034)
#define VGS_VREG_CLSA *((uint32_t*)0xD20038)
#define VGS_VREG_CLS0 *((uint32_t*)0xD2003C)
#define VGS_VREG_CLS1 *((uint32_t*)0xD20040)
#define VGS_VREG_CLS2 *((uint32_t*)0xD20044)
#define VGS_VREG_CLS3 *((uint32_t*)0xD20048)
#define VGS_VREG_G_BG *((uint32_t*)0xD2004C)
#define VGS_VREG_G_X1 *((uint32_t*)0xD20050)
#define VGS_VREG_G_Y1 *((uint32_t*)0xD20054)
#define VGS_VREG_G_X2 *((uint32_t*)0xD20058)
#define VGS_VREG_G_Y2 *((uint32_t*)0xD2005C)
#define VGS_VREG_G_COL *((uint32_t*)0xD20060)
#define VGS_VREG_G_OPT *((uint32_t*)0xD20064)
#define VGS_VREG_G_EXE *((uint32_t*)0xD20068)

// Graphic Draw Function Identifer
#define VGS_DRAW_PIXEL 0
#define VGS_DRAW_LINE 1
#define VGS_DRAW_BOX 2
#define VGS_DRAW_BOXF 3
#define VGS_DRAW_CHR 4

// I/O
#define VGS_IN_VSYNC *((uint32_t*)0xE00000)
#define VGS_OUT_CONSOLE *((uint32_t*)0xE00000)
#define VGS_IO_RANDOM *((uint32_t*)0xE00004)
#define VGS_OUT_VGM_PLAY *((uint32_t*)0xE01000)
#define VGS_OUT_SFX_PLAY *((uint32_t*)0xE01100)
#define VGS_KEY_UP *((uint32_t*)0xE20000)
#define VGS_KEY_DOWN *((uint32_t*)0xE20004)
#define VGS_KEY_LEFT *((uint32_t*)0xE20008)
#define VGS_KEY_RIGHT *((uint32_t*)0xE2000C)
#define VGS_KEY_A *((uint32_t*)0xE20010)
#define VGS_KEY_B *((uint32_t*)0xE20014)
#define VGS_KEY_X *((uint32_t*)0xE20018)
#define VGS_KEY_Y *((uint32_t*)0xE2001C)
#define VGS_KEY_START *((uint32_t*)0xE20020)
#define VGS_OUT_EXIT *((int32_t*)0xE7FFFC)

#define TRUE 1
#define FALSE 0

#ifdef __cplusplus
extern "C" {
#endif

void vgs_vsync(void);
void vgs_srand(uint16_t seed);
uint16_t vgs_rand(void);
uint32_t vgs_rand32(void);
void vgs_console_print(const char* text);
void vgs_console_println(const char* text);
void vgs_d32str(char* buf12, int32_t n);
void vgs_u32str(char* buf11, uint32_t n);
void vgs_put_bg(uint8_t n, uint8_t x, uint8_t y, uint32_t data);
void vgs_print_bg(uint8_t n, uint8_t x, uint8_t y, uint8_t pal, const char* text);
void vgs_cls_bg_all(uint32_t value);
void vgs_cls_bg(uint8_t n, uint32_t value);
void vgs_draw_pixel(uint8_t n, int32_t x, int32_t y, uint32_t col);
void vgs_draw_line(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);
void vgs_draw_box(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);
void vgs_draw_boxf(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);
void vgs_draw_character(uint8_t n, int32_t x, int32_t y, int draw0, uint8_t pal, uint16_t ptn);
void vgs_sprite(uint16_t n, uint8_t visible, int16_t x, int16_t y, uint8_t size, uint8_t pal, uint16_t ptn);
void vgs_sprite_visible(uint16_t n, uint8_t visible);
void vgs_sprite_position(uint16_t n, int16_t x, int16_t y);
void vgs_sprite_palette(uint16_t n, uint8_t size);
void vgs_sprite_palette(uint16_t n, uint8_t pal);
void vgs_sprite_pattern(uint16_t n, uint16_t ptn);
void vgs_sprite_flip(uint16_t n, int8_t h, int8_t v);
void vgs_bgm_play(uint16_t n);
void vgs_sfx_play(uint8_t n);
void vgs_exit(int32_t code);

#ifdef __cplusplus
};
#endif
