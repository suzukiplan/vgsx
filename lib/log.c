/**
 * VGS-X Runtime Library for MC68030
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
#include "log.h"

void vgs_print(const char* text)
{
    while (*text) {
        VGS_OUT_CONSOLE = *text;
        text++;
    }
}

void vgs_println(const char* text)
{
    vgs_print(text);
    VGS_OUT_CONSOLE = '\n';
}

void vgs_putlog(const char* format, ...)
{
    char d32[12];
    va_list arg;
    va_start(arg, format);
    while (*format) {
        if (format[0] == '%' && format[1] == 'd') {
            vgs_d32str(d32, va_arg(arg, int32_t));
            vgs_print(d32);
            format += 2;
        } else if (format[0] == '%' && format[1] == 'u') {
            vgs_u32str(d32, va_arg(arg, uint32_t));
            vgs_print(d32);
            format += 2;
        } else if (format[0] == '%' && format[1] == 's') {
            vgs_print(va_arg(arg, const char*));
            format += 2;
        } else {
            VGS_OUT_CONSOLE = *format;
            format++;
        }
    }
    VGS_OUT_CONSOLE = '\n';
    va_end(arg);
}
