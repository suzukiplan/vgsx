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
#include "vgs_stdint.h"

// Name table (256x256)
// Bit Layout:
// - attribute (12bit): F/H F/V 0 0 - 0 0 0 0 - 0 0 0 0
//   - F/H: Flip Horizontal
//   - F/V: Flip Vertical
// - palette (4bit)
// - pattern number (16bit)
#define BG_WIDTH 256
#define BG_HEIGHT 256
#define BG0 ((uint32_t*)0xC00000)
#define BG1 ((uint32_t*)0xC40000)
#define BG2 ((uint32_t*)0xC80000)
#define BG3 ((uint32_t*)0xCC0000)

// VRAM: 320x200 x 32bit (RGB888)
#define VRAM_WIDTH 320
#define VRAM_HEIGHT 200
#define VRAM0 ((uint32_t*)0xC00000)
#define VRAM1 ((uint32_t*)0xC40000)
#define VRAM2 ((uint32_t*)0xC80000)
#define VRAM3 ((uint32_t*)0xCC0000)

// Sprites
typedef struct {
    uint32_t visible;     // Visible (0 or not 0)
    int32_t y;            // Position (Y)
    int32_t x;            // Position (X)
    uint32_t attr;        // Attribute
    uint32_t size;        // Size (0: 8x8, 1: 16x16, 2: 24x24, 3: 32x32 ... 31: 256x256)
    int32_t rotate;       // Rotate (-360 ~ 360)
    uint32_t scale;       // Scale (0: disabled or 1 ~ 400 percent)
    uint32_t reserved[9]; // Reserved
} ObjectAttributeMemory;

#define OAM_MAX 1024
#define OAM ((ObjectAttributeMemory*)0xD00000)

// Palette Table
// 16 x 16 x 4bits
#define PALETTE ((uint32_t*)0xD10000)

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
#define VGS_VREG_CLS ((uint32_t*)0xD2003C)
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
#define VGS_VREG_SKIP_BG0 *((uint32_t*)0xD2006C)
#define VGS_VREG_SKIP_BG1 *((uint32_t*)0xD20070)
#define VGS_VREG_SKIP_BG2 *((uint32_t*)0xD20074)
#define VGS_VREG_SKIP_BG3 *((uint32_t*)0xD20078)

// Graphic Draw Function Identifer
#define VGS_DRAW_PIXEL 0
#define VGS_DRAW_LINE 1
#define VGS_DRAW_BOX 2
#define VGS_DRAW_BOXF 3
#define VGS_DRAW_CHR 4

// DMA Function Identifer
#define VGS_DMA_MEMCPY 0
#define VGS_DMA_MEMSET 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Get the Name Table width in Character Pattern Mode.
 * @return The Name Table width in Character Pattern Mode. (VGS-X returns 256)
 */
static inline int vgs_bg_width()
{
    return BG_WIDTH;
}

/**
 * @brief Get the Name Table height in Character Pattern Mode.
 * @return The Name Table height in Character Pattern Mode. (VGS-X returns 256)
 */
static inline int vgs_bg_height()
{
    return BG_HEIGHT;
}

/**
 * @brief Get the Visible Name Table width in Character Pattern Mode.
 * @return The Visible Name Table width in Character Pattern Mode. (VGS-X returns 40)
 */
static inline int vgs_chr_width()
{
    return VRAM_WIDTH >> 3;
}

/**
 * @brief Get the Visible Name Table height in Character Pattern Mode.
 * @return The Visible Name Table height in Character Pattern Mode. (VGS-X returns 25)
 */
static inline int vgs_chr_height()
{
    return VRAM_HEIGHT >> 3;
}

/**
 * @brief Display a character on the BG in Character Pattern Mode
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate of nametable (0 to 255)
 * @param y Y-coordinate of nametable (0 to 255)
 * @param attr Attribute
 */
void vgs_put_bg(uint8_t n, uint8_t x, uint8_t y, uint32_t attr);

/**
 * @brief Display string on the BG in Character Pattern Mode
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate of nametable (0 to 255)
 * @param y Y-coordinate of nametable (0 to 255)
 * @param pal Palette number
 * @param text Buffer pointer pointing to the beginning of a buffer containing text terminated by a null character
 */
void vgs_print_bg(uint8_t n, uint8_t x, uint8_t y, uint8_t pal, const char* text);

/**
 * @brief Clear all BGs
 * @param value It will be cleared with the value specified by this parameter.
 * @remark If the value is 0, a fast clear is performed.
 */
void vgs_cls_bg_all(uint32_t value);

/**
 * @brief Clear a specific BG
 * @param n Number of BG (0 to 3)
 * @param value It will be cleared with the value specified by this parameter.
 * @remark If the value is 0, a fast clear is performed.
 */
void vgs_cls_bg(uint8_t n, uint32_t value);

/**
 * @brief BG Mode Switching: Bitmap or Character Pattern
 * @param n Number of BG (0 to 3)
 * @param isBitmap If TRUE is specified, switch to Bitmap Mode; if FALSE is specified, switch to Character Pattern Mode.
 */
static inline void vgs_draw_mode(uint8_t n, BOOL isBitmap)
{
    ((uint32_t*)0xD20028)[n & 3] = isBitmap;
}

/**
 * @brief Get the display width in Bitmap Mode.
 * @return The display width in Bitmap Mode. (VGS-X returns 320)
 */
static inline int vgs_draw_width()
{
    return VRAM_WIDTH;
}

/**
 * @brief Get the display height in Bitmap Mode.
 * @return The display height in Bitmap Mode. (VGS-X returns 200)
 */
static inline int vgs_draw_height()
{
    return VRAM_HEIGHT;
}

/**
 * @brief Read a pixel on the BG in Bitmap Mode
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate of VRAM (0 to 319)
 * @param y Y-coordinate of VRAM (0 to 199)
 * @return A pixel (RGB888 color format)
 */
uint32_t vgs_read_pixel(uint8_t n, int32_t x, int32_t y);

/**
 * @brief Draw a pixel on the BG in Bitmap Mode
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate of VRAM (0 to 319)
 * @param y Y-coordinate of VRAM (0 to 199)
 * @param col RGB888 color format
 * @remark Drawing outside the screen area will be skipped.
 */
void vgs_draw_pixel(uint8_t n, int32_t x, int32_t y, uint32_t col);

/**
 * @brief Draw a line on the BG in Bitmap Mode
 * @param n Number of BG (0 to 3)
 * @param x1 X-coordinate of VRAM (0 to 319)
 * @param y1 Y-coordinate of VRAM (0 to 199)
 * @param x2 X-coordinate of VRAM (0 to 319)
 * @param y2 Y-coordinate of VRAM (0 to 199)
 * @param col RGB888 color format
 * @remark Drawing outside the screen area will be skipped.
 */
void vgs_draw_line(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);

/**
 * @brief Draw a rectangle on the BG in Bitmap Mode
 * @param n Number of BG (0 to 3)
 * @param x1 X-coordinate of VRAM (0 to 319)
 * @param y1 Y-coordinate of VRAM (0 to 199)
 * @param x2 X-coordinate of VRAM (0 to 319)
 * @param y2 Y-coordinate of VRAM (0 to 199)
 * @param col RGB888 color format
 * @remark Drawing outside the screen area will be skipped.
 */
void vgs_draw_box(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);

/**
 * @brief Draw a filled-rectangle on the BG in Bitmap Mode
 * @param n Number of BG (0 to 3)
 * @param x1 X-coordinate of VRAM (0 to 319)
 * @param y1 Y-coordinate of VRAM (0 to 199)
 * @param x2 X-coordinate of VRAM (0 to 319)
 * @param y2 Y-coordinate of VRAM (0 to 199)
 * @param col RGB888 color format
 * @remark Drawing outside the screen area will be skipped.
 */
void vgs_draw_boxf(uint8_t n, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t col);

/**
 * @brief Draw a character-pattern on the BG in Bitmap Mode
 * @param n Number of BG (0 to 3)
 * @param x X-coordinate of VRAM (0 to 319)
 * @param y Y-coordinate of VRAM (0 to 199)
 * @param draw0 TRUE: Draw color number 0, FLASE: Skip color number 0
 * @param pal Palette number (0 to 15)
 * @param ptn Character pattern number (0 to 65535)
 * @remark Drawing outside the screen area will be skipped.
 */
void vgs_draw_character(uint8_t n, int32_t x, int32_t y, BOOL draw0, uint8_t pal, uint16_t ptn);

/**
 * @brief Scroll BG (X)
 * @param n Number of BG (0 to 3)
 * @param sx X-coordinate
 */
static inline void vgs_scroll_x(uint8_t n, int32_t sx)
{
    ((uint32_t*)0xD20008)[n & 3] = sx;
}

/**
 * @brief Scroll BG (Y)
 * @param n Number of BG (0 to 3)
 * @param sy Y-coordinate
 */
static inline void vgs_scroll_y(uint8_t n, int32_t sy)
{
    ((uint32_t*)0xD20018)[n & 3] = sy;
}

/**
 * @brief Scroll BG
 * @param n Number of BG (0 to 3)
 * @param sx X-coordinate
 * @param sy Y-coordinate
 */
static inline void vgs_scroll(uint8_t n, int32_t sx, int32_t sy)
{
    n &= 3;
    ((uint32_t*)0xD20008)[n] = sx;
    ((uint32_t*)0xD20018)[n] = sy;
}

/**
 * @brief Set sprite display priority
 * @param bg Specify the BG number (0 to 3) displayed beneath the sprite.
 */
static inline void vgs_sprite_priority(uint8_t bg)
{
    VGS_VREG_SPOS = bg;
}

/**
 * @brief Set OAM attribute values in bulk
 * @param n Sprite number (0 to 1023)
 * @param visible TRUE: Visible, FALSE: Hidden
 * @param x X-coordinate of VRAM (0 to 319)
 * @param y Y-coordinate of VRAM (0 to 199)
 * @param size Sprite size (0: 8x8, 1: 16x16, 2: 24x24 ... 15: 256x256)
 * @param pal Palette number (0 to 15)
 * @param ptn Character pattern number (0 to 65535)
 * @remark Scale and rotation are reset to disabled (0).
 * @remark Drawing outside the screen area will be skipped.
 * @remark Individual parameters can be referenced and/or updated via OAM[n].
 */
void vgs_sprite(uint16_t n, BOOL visible, int16_t x, int16_t y, uint8_t size, uint8_t pal, uint16_t ptn);

/**
 * @brief Make all sprites invisible.
 * @remark This function resets the visible flag for all OAM records.
 */
static inline void vgs_sprite_hide_all(void)
{
    for (int i = 0; i < OAM_MAX; i++) {
        OAM[i].visible = FALSE;
    }
}

/**
 * @brief Get an OAM record
 * @param n Sprite number (0 to 1023)
 */
static inline ObjectAttributeMemory* vgs_oam(uint16_t n)
{
    return &OAM[n];
}

#ifdef __cplusplus
};
#endif
