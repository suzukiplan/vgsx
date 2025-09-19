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
#include "vgs.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Output text to the debug console (no line breaks)
 * @param text Buffer pointer pointing to the beginning of a buffer containing text terminated by a null character
 */
void vgs_print(const char* text);

/**
 * @brief Output text to the debug console (with line breaks)
 * @param text Buffer pointer pointing to the beginning of a buffer containing text terminated by a null character
 */
void vgs_println(const char* text);

/**
 * @brief Output formatted string logs to the debug console.
 * @param format Buffer pointer pointing to the beginning of a buffer containing format-string terminated by a null character
 * @remark You can specify embedded characters using the following format: %d, %u, %s
 * @remark %d is must be int32_t
 * @remark %u is must be uint32_t
 * @remark %s is must be string pointer terminated by a null character
 */
void vgs_putlog(const char* format, ...);

#ifdef __cplusplus
};
#endif
