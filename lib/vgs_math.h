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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Calculate the angle between two points (in degrees)
 * @param x1 X of Coordinate 1
 * @param y1 Y of Coordinate 1
 * @param x2 X of Coordinate 2
 * @param y2 Y of Coordinate 2
 * @return The angle between two points is measured in degrees, from 0 to 359.
 */
int32_t vgs_degree(int32_t x1, int32_t y1, int32_t x2, int32_t y2);

/**
 * @brief Calculate integer sine from the angle in degrees
 * @param degree Angle in degrees
 * @return The integer sine ranges from -256 to 256.
 */
int32_t vgs_sin(int32_t degree);

/**
 * @brief Calculate integer cosine from the angle in degrees
 * @param degree Angle in degrees
 * @return The integer cosine ranges from -256 to 256.
 */
int32_t vgs_cos(int32_t degree);

/**
 * @brief Calculate the absolute value of an integer.
 * @param value Target value
 * @return The absolute value.
 */
static inline int32_t vgs_abs(int32_t value)
{
    return value < 0 ? -value : value;
}

/**
 * @brief Determine whether an integer is positive, negative or zero.
 * @param value Target value
 * @return Return -1 for negative numbers, 1 for positive numbers, and 0 for neither.
 */
static inline int32_t vgs_sgn(int32_t value)
{
    if (0 == value) {
        return 0;
    }
    return value < 0 ? -1 : 1;
}

/**
 * @brief 2 Rectangular Collision Detection.
 * @param x1 X-coordinate of Rectangle 1
 * @param y1 Y-coordinate of Rectangle 1
 * @param w1 Width of Rectangle 1
 * @param h1 Height of Rectangle 1
 * @param x2 X-coordinate of Rectangle 2
 * @param y2 Y-coordinate of Rectangle 2
 * @param w2 Width of Rectangle 2
 * @param h2 Height of Rectangle 2
 * @return Returns TRUE if a collision is detected, FALSE otherwise.
 */
static inline BOOL vgs_hitchk(int x1, int y1, int w1, int h1, int x2, int y2, int w2, int h2)
{
    return y1 < y2 + h2 && y2 < y1 + h1 && x1 < x2 + w2 && x2 < x1 + w1 ? TRUE : FALSE;
}

#ifdef __cplusplus
};
#endif
