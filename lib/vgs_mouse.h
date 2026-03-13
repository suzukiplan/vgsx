/**
 * VGS Standard Library for MC68030
 * The MIT License (MIT)
 *
 * Copyright (c) 2026 Yoji Suzuki.
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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check Eanbled/Disabled the mouse
 * @return ON: enabled, OFF: disabled
 */
static inline int vgs_mouse_enabled(void) { return VGS_IN_MOUSE_ENABLED ? ON : OFF; }

/**
 * @brief Setup the mouse cursor pattern and palette
 * @param ptn Character pattern index of cursor
 * @param pal Palette index of cursor
 */
static inline void vgs_mouse_setup(uint16_t ptn, uint8_t pal)
{
    VGS_IO_MOUSE_PATTERN = ptn;
    VGS_IO_MOUSE_PALETTE = pal;
}

/**
 * @brief Visible/Hidden the mouse cursor
 * @param on ON: hidden, OFF: visible
 */
static inline void vgs_mouse_hidden(int on) { VGS_IO_MOUSE_HIDDEN = on; }

/**
 * @brief Check the mouse has moving in the currnt frame.
 * @return ON: moving, OFF: not moving
 */
static inline int vgs_mouse_moving(void) { return VGS_IN_MOUSE_MOVING; }

/**
 * @brief Get the mouse X-coordinate
 * @return Mouse X-coordinate
 */
static inline int vgs_mouse_x(void) { return VGS_IN_MOUSE_X; }

/**
 * @brief Get the mouse Y-coordinate
 * @return Mouse Y-coordinate
 */
static inline int vgs_mouse_y(void) { return VGS_IN_MOUSE_Y; }

/**
 * @brief Get the mouse left button status
 * @return ON: pushing, OFF: not pushing
 */
static inline int vgs_mouse_left(void) { return VGS_IN_MOUSE_LEFT; }

/**
 * @brief Get the mouse right button status
 * @return ON: pushing, OFF: not pushing
 */
static inline int vgs_mouse_right(void) { return VGS_IN_MOUSE_RIGHT; }

/**
 * @brief Get the mouse left button clicked
 * @param x Clicked position (x-coordinate)
 * @param y Clicked position (y-coordinate)
 * @return ON: clicked, OFF: not clicked
 */
static inline int vgs_mouse_left_clicked(int* x, int* y)
{
    if (VGS_IN_MOUSE_LEFT_CLICK) {
        if (x) *x = VGS_IN_MOUSE_LEFT_CLICK_X;
        if (y) *y = VGS_IN_MOUSE_LEFT_CLICK_Y;
        return ON;
    } else {
        return OFF;
    }
}

/**
 * @brief Get the mouse right button clicked
 * @param x Clicked position (x-coordinate)
 * @param y Clicked position (y-coordinate)
 * @return ON: clicked, OFF: not clicked
 */
static inline int vgs_mouse_right_clicked(int* x, int* y)
{
    if (VGS_IN_MOUSE_RIGHT_CLICK) {
        if (x) *x = VGS_IN_MOUSE_RIGHT_CLICK_X;
        if (y) *y = VGS_IN_MOUSE_RIGHT_CLICK_Y;
        return ON;
    } else {
        return OFF;
    }
}

#ifdef __cplusplus
};
#endif
