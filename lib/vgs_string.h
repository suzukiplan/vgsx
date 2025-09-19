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
 * @brief Convert a 32-bit signed integer to a string
 * @param buf12 Pointer to the beginning of a buffer allocated with 12 bytes or more
 * @param n Numbers converted to strings
 */
void vgs_d32str(char* buf12, int32_t n);

/**
 * @brief Convert a 32-bit unsigned integer to a string
 * @param buf11 Pointer to the beginning of a buffer allocated with 11 bytes or more
 * @param n Numbers converted to strings
 */
void vgs_u32str(char* buf11, uint32_t n);

/**
 * @brief Transfer the data from the address specified in `source` to the address specified in `destination`, for the number of bytes specified in `size`.
 * @param destination must be a RAM Address (0xF00000 to 0xFFFFFF).
 * @param source must be either a Program Address (0x000000 to Size-of-Program) or a RAM Address (0xF00000 to 0xFFFFFF).
 * @param size Transfer size in byte.
 * @remark When both `source` and `destination` point to RAM addresses, overlapping copy ranges are acceptable. (A copy equivalent to `memmove` is performed.)
 * @remark If an invalid address range (including the result of the addition) is specified, this function will not be executed.
 */
void vgs_memcpy(void* destination, const void* source, uint32_t size);

/**
 * @brief Write the value specified by `value` to the address specified by `destination` for the number of bytes specified by `size`.
 * @param destination must be a RAM Address (0xF00000 to 0xFFFFFF).
 * @param value The value to write to the `destination` address
 * @param size Transfer size in byte.
 * @remark If an invalid address range (including the result of the addition) is specified, this function will not be executed.
 */
void vgs_memset(void* destination, uint8_t value, uint32_t size);

/**
 * @brief Get the length of a null-terminated string buffer.
 * @param str Null-terminated string buffer
 * @return Length or 0
 * @remark `str` must be either a Program Address (0x000000 to Size-of-Program) or a RAM Address (0xF00000 to 0xFFFFFF).
 * @remark If an invalid address range (including the result of the addition) is specified, this function will be return 0.
 */
uint32_t vgs_strlen(const char* str);

/**
 * @brief Search for specific characters in a string.
 * @param str Null-terminated string buffer
 * @param c A search character
 * @return If the character being searched for is found, it returns a pointer to its position; if not found, it returns NULL.
 */
char* vgs_strchr(const char* str, int c);

/**
 * @brief Search for specific characters in a string that right to left.
 * @param str Null-terminated string buffer
 * @param c A search character
 * @return If the character being searched for is found, it returns a pointer to its position; if not found, it returns NULL.
 */
char* vgs_strrchr(const char* str, int c);

/**
 * @brief Compare strings.
 * @param str1 Null-terminated string buffer
 * @param str2 Null-terminated string buffer
 * @return 0: str1 == str2, -1(<0): str1 < str2, 1(>0): str1 > str2
 */
int vgs_strcmp(const char* str1, const char* str2);

/**
 * @brief Comparing strings of a specific length
 * @param str1 Null-terminated string buffer
 * @param str2 Null-terminated string buffer
 * @param n Length
 * @return 0: str1 == str2, -1(<0): str1 < str2, 1(>0): str1 > str2
 */
int vgs_strncmp(const char* str1, const char* str2, int n);

/**
 * @brief Search for a specific string in a string.
 * @param str1 Search string
 * @param str2 String to search for
 * @return Returns the pointer to the first occurrence of str2 found within str1.
 * @return If the search string is not found, NULL is returned.
 */
char* vgs_strstr(const char* str1, const char* str2);

#ifdef __cplusplus
};
#endif
