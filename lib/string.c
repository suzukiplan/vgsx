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
#include <vgs.h>

void vgs_u32str(char* buf11, uint32_t n)
{
    int32_t kt = 1000000000;
    int detect = 0;
    int w;
    if (0 == n) {
        *buf11 = '0';
        buf11++;
    } else {
        while (0 < kt) {
            w = (int)(n / kt);
            if (w) {
                detect = 1;
                *buf11 = '0' + w;
                buf11++;
            } else if (detect) {
                *buf11 = '0';
                buf11++;
            }
            n %= kt;
            kt /= 10;
        }
    }
    *buf11 = 0;
}

void vgs_d32str(char* buf12, int32_t n)
{
    if (n < 0) {
        *buf12 = '-';
        n = -n;
        buf12++;
    }
    vgs_u32str(buf12, (uint32_t)n);
}

void vgs_memcpy(void* destination, const void* source, uint32_t size)
{
    VGS_OUT_DMA_DESTINATION = (uint32_t)destination;
    VGS_OUT_DMA_SOURCE = (uint32_t)source;
    VGS_OUT_DMA_ARGUMENT = size;
    VGS_IO_DMA_EXECUTE = VGS_DMA_MEMCPY;
}

void vgs_memset(void* destination, uint8_t value, uint32_t size)
{
    VGS_OUT_DMA_DESTINATION = (uint32_t)destination;
    VGS_OUT_DMA_SOURCE = value;
    VGS_OUT_DMA_ARGUMENT = size;
    VGS_IO_DMA_EXECUTE = VGS_DMA_MEMSET;
}

uint32_t vgs_strlen(const char* str)
{
    VGS_OUT_DMA_SOURCE = (uint32_t)str;
    VGS_OUT_DMA_ARGUMENT = 0;
    const char* end = (const char*)VGS_IO_DMA_EXECUTE;
    return end ? (uint32_t)(end - str) : 0;
}

char* vgs_strchr(const char* str, int c)
{
    while (*str) {
        if (c == *str) {
            return (char*)str;
        }
        str++;
    }
    return (char*)NULL;
}

char* vgs_strrchr(const char* str, int c)
{
    c &= 0xFF;
    for (int32_t ptr = ((int32_t)vgs_strlen(str)) - 1; 0 < ptr; ptr--) {
        if (str[ptr] == c) {
            return (char*)&str[ptr];
        }
    }
    return (char*)NULL;
}

int vgs_strcmp(const char* str1, const char* str2)
{
    while (*str1 == *str2) {
        if (0 == *str1) {
            return 0;
        }
        str1++;
        str2++;
    }
    return *str1 < *str2 ? -1 : 1;
}

int vgs_strncmp(const char* str1, const char* str2, int n)
{
    while (*str1 == *str2) {
        if (0 == *str1) {
            return 0;
        }
        if (--n == 0) {
            return 0;
        }
        str1++;
        str2++;
    }
    return *str1 < *str2 ? -1 : 1;
}

char* vgs_strstr(const char* str1, const char* str2)
{
    int32_t length = vgs_strlen(str2);
    if (0 == length) {
        return (char*)str1; // searched an empty string
    }
    while (NULL != (str1 = vgs_strchr(str1, *str2))) {
        if (0 == vgs_strncmp(str1, str2, length)) {
            return (char*)str1;
        }
        str1++;
    }
    return NULL;
}
