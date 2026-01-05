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

void* memcpy(void* dest, const void* src, uint32_t size)
{
    vgs_memcpy(dest, src, size);
    return dest;
}

void* memset(void* dest, uint32_t c, uint32_t size)
{
    vgs_memset(dest, c, size);
    return dest;
}

uint32_t strlen(const char* str) { return vgs_strlen(str); }
char* strchr(const char* str, int c) { return vgs_strchr(str, c); }
char* strrchr(const char* str, int c) { return vgs_strrchr(str, c); }
int strcmp(const char* str1, const char* str2) { return vgs_strcmp(str1, str2); }
int strcasecmp(const char* str1, const char* str2) { return vgs_stricmp(str1, str2); }
int stricmp(const char* str1, const char* str2) { return vgs_stricmp(str1, str2); }
int strncmp(const char* str1, const char* str2, int n) { return vgs_strncmp(str1, str2, n); }
char* strstr(const char* str1, const char* str2) { return vgs_strstr(str1, str2); }
char* strcpy(char* dest, const char* src) { return vgs_strcpy(dest, src); }
char* strcat(char* dest, const char* src) { return vgs_strcat(dest, src); }
