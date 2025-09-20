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
 * @brief Check if a character is a number
 * @param c A character to check
 * @return TRUE or FALSE
 */
static inline BOOL vgs_isdigit(int c)
{
    return '0' <= c && c <= '9' ? TRUE : FALSE;
}

/**
 * @brief Check if a character is an uppercase
 * @param c A character to check
 * @return TRUE or FALSE
 */
static inline BOOL vgs_isupper(int c)
{
    return ('A' <= c && c <= 'Z') ? TRUE : FALSE;
}

/**
 * @brief Check if a character is a lowercase
 * @param c A character to check
 * @return TRUE or FALSE
 */
static inline BOOL vgs_islower(int c)
{
    return ('a' <= c && c <= 'z') ? TRUE : FALSE;
}

/**
 * @brief Check if a character is an alphabet
 * @param c A character to check
 * @return TRUE or FALSE
 */
static inline BOOL vgs_isalpha(int c)
{
    return vgs_isupper(c) | vgs_islower(c);
}

/**
 * @brief Check if a character is an alphabet or a digit
 * @param c A character to check
 * @return TRUE or FALSE
 */
static inline BOOL vgs_isalnum(int c)
{
    return vgs_isdigit(c) | vgs_isupper(c) | vgs_islower(c);
}

/**
 * @brief Convert a lowercase letter to an uppercase letter.
 * @param c A character to convert
 * @return If the input was a lowercase letter, it returns the uppercase version; otherwise, it returns the argument c as-is.
 */
static inline int vgs_toupper(int c)
{
    return vgs_islower(c) ? c - ('a' - 'A') : c;
}

/**
 * @brief Convert an uppercase letter to a lowercase letter.
 * @param c A character to convert
 * @return If the input was an uppercase letter, it returns the lowercase version; otherwise, it returns the argument c as-is.
 */
static inline int vgs_tolower(int c)
{
    return vgs_isupper(c) ? c + ('a' - 'A') : c;
}

/**
 * @brief Convert a string to an integer.
 * @param str String to convert
 * @return Converted integer
 * @remark If an unrecognizable string is specified, return 0.
 */
int32_t vgs_atoi(const char* str);

#ifdef __cplusplus
};
#endif
