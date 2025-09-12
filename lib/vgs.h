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
    uint32_t hidden;      // Hidden (0 or not 0)
    int32_t y;            // Position (Y)
    int32_t x;            // Position (X)
    uint32_t attr;        // Attribute
    uint32_t reserved[4]; // Reserved
} VGS_Oam;

#define VGS_MAX_SPRITE 1024

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

// Palette Table
// 16 x 16 x 2bits
#define VGS_PALETTE ((uint32_t*)0xD10000)

// VDP Register
#define VGS_VREG_SKIP ((uint32_t*)0xD20000)
#define VGS_VREG_SPOS ((uint32_t*)0xD20004)
#define VGS_VREG_SX0 ((uint32_t*)0xD20008)
#define VGS_VREG_SX1 ((uint32_t*)0xD2000C)
#define VGS_VREG_SX2 ((uint32_t*)0xD20010)
#define VGS_VREG_SX3 ((uint32_t*)0xD20014)
#define VGS_VREG_SY0 ((uint32_t*)0xD20018)
#define VGS_VREG_SY1 ((uint32_t*)0xD2001C)
#define VGS_VREG_SY2 ((uint32_t*)0xD20020)
#define VGS_VREG_SY3 ((uint32_t*)0xD20024)
#define VGS_VREG_BMP0 ((uint32_t*)0xD20028)
#define VGS_VREG_BMP1 ((uint32_t*)0xD2002C)
#define VGS_VREG_BMP2 ((uint32_t*)0xD20030)
#define VGS_VREG_BMP3 ((uint32_t*)0xD20034)
#define VGS_VREG_CLSA ((uint32_t*)0xD20038)
#define VGS_VREG_CLS0 ((uint32_t*)0xD2003C)
#define VGS_VREG_CLS1 ((uint32_t*)0xD20040)
#define VGS_VREG_CLS2 ((uint32_t*)0xD20044)
#define VGS_VREG_CLS3 ((uint32_t*)0xD20048)

// I/O
#define VGS_IN_VSYNC ((uint32_t*)0xE00000)
#define VGS_OUT_CONSOLE ((uint32_t*)0xE00000)

#ifdef __cplusplus
extern "C" {
#endif

void vgs_vsync(void);
void vgs_console_print(const char* text);
void vgs_put_bg(uint8_t n, uint8_t x, uint8_t y, uint32_t data);
void vgs_print_bg(uint8_t n, uint8_t x, uint8_t y, uint8_t pal, const char* text);
void vgs_cls_bg_all(uint32_t value);
void vgs_cls_bg(uint8_t n, uint32_t value);

#ifdef __cplusplus
};
#endif
