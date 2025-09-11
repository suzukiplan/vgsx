/**
 * VGS-X Runtime Library for MC68000
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

// Scroll Register
#define VGS_VREG_SX0 ((uint16_t*)0xD20000)
#define VGS_VREG_SY0 ((uint16_t*)0xD20002)
#define VGS_VREG_SX1 ((uint16_t*)0xD20004)
#define VGS_VREG_SY1 ((uint16_t*)0xD20006)
#define VGS_VREG_SX2 ((uint16_t*)0xD20008)
#define VGS_VREG_SY2 ((uint16_t*)0xD2000A)
#define VGS_VREG_SX3 ((uint16_t*)0xD2000C)
#define VGS_VREG_SY3 ((uint16_t*)0xD2000E)

// I/O
#define VGS_IN_VSYNC ((uint32_t*)0xE00000)
#define VGS_OUT_CONSOLE ((uint32_t*)0xE00000)

#ifdef __cplusplus
extern "C" {
#endif

void vgs_vsync(void);
void vgs_console_print(const char* text);
void vgs_put_bg0(uint8_t x, uint8_t y, uint32_t data);
void vgs_print_bg0(uint8_t x, uint8_t y, uint8_t pal, const char* text);

#ifdef __cplusplus
};
#endif
