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
#include "vgs.h"

int32_t vgs_degree(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    VGS_OUT_ANGLE_X1 = x1;
    VGS_OUT_ANGLE_Y1 = y1;
    VGS_OUT_ANGLE_X2 = x2;
    VGS_OUT_ANGLE_Y2 = y2;
    return VGS_IO_ANGLE_DEGREE;
}

int32_t vgs_sin(int32_t degree)
{
    VGS_IO_ANGLE_DEGREE = degree;
    return VGS_IN_ANGLE_SIN;
}

int32_t vgs_cos(int32_t degree)
{
    VGS_IO_ANGLE_DEGREE = degree;
    return VGS_IN_ANGLE_COS;
}
