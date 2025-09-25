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
#pragma once
#include "vgs_cg.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Proportional Font Initialization.
 * @param ptn The starting number for character patterns used in the font.
 * @remark Assume that 128 character patterns are set from ptn, with images corresponding to ASCII codes 0x00 to 0x7F.
 */
static inline void vgs_pfont_init(uint16_t ptn)
{
    VGS_VREG_PF_INIT = ptn;
}

/**
 * @brief Acquiring Proportional Font Information.
 * @param code Character code to be acquired (0x00 to 0x7F)
 * @param dx X-coordinate difference.
 * @param dy Y-coordinate difference.
 * @param width Font display width.
 */
static inline void vgs_pfont_get(uint8_t code, int32_t* dx, int32_t* dy, int32_t* width)
{
    VGS_VREG_PF_PTN = code;
    *dx = VGS_VREG_PF_DX;
    *dy = VGS_VREG_PF_DY;
    *width = VGS_VREG_PF_WIDTH;
}

/**
 * @brief Setting Proportional Font Information.
 * @param code Character code to be acquired (0x00 to 0x7F)
 * @param dx X-coordinate difference.
 * @param dy Y-coordinate difference.
 * @param width Font display width.
 */
static inline void vgs_pfont_set(uint8_t code, int32_t dx, int32_t dy, int32_t width)
{
    VGS_VREG_PF_PTN = code;
    VGS_VREG_PF_DX = dx;
    VGS_VREG_PF_DY = dy;
    VGS_VREG_PF_WIDTH = width;
}

/**
 * @brief Drawing strings using proportional fonts
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate
 * @param y Y-coordinate
 * @param pal Palette Number
 * @param ptn The starting number for character patterns used in the font.
 * @param text A pointer to a null-terminated buffer containing the string to be displayed.
 */
void vgs_pfont_print(uint8_t n, int32_t x, int32_t y, uint8_t pal, uint16_t ptn, const char* text);

/**
 * @brief Width of a string displayed in a proportional font (in pixels).
 * @param text A pointer to a null-terminated buffer containing the string to be displayed.
 * @return Display width (in pixels).
 */
int32_t vgs_pfont_strlen(const char* text);

/**
 * @brief Drawing strings using k8x12 Japanese Font
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate
 * @param y Y-coordinate
 * @param col RGB888 color format
 * @param sjis A pointer to a null-terminated buffer containing the Shift-JIS string to be displayed.
 */
void vgs_k8x12_print(uint8_t n, int32_t x, int32_t y, uint32_t col, const char* sjis);

#ifdef __cplusplus
};
#endif
