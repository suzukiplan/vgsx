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

#define VGS_IN_VSYNC *((uint32_t*)0xE00000)
#define VGS_OUT_CONSOLE *((uint32_t*)0xE00000)
#define VGS_IO_RANDOM *((uint32_t*)0xE00004)
#define VGS_OUT_DMA_SOURCE *((uint32_t*)0xE00008)
#define VGS_OUT_DMA_DESTINATION *((uint32_t*)0xE0000C)
#define VGS_OUT_DMA_ARGUMENT *((uint32_t*)0xE00010)
#define VGS_IO_DMA_EXECUTE *((uint32_t*)0xE00014)
#define VGS_OUT_VGM_PLAY *((uint32_t*)0xE01000)
#define VGS_OUT_SFX_PLAY *((uint32_t*)0xE01100)
#define VGS_KEY_UP *((uint32_t*)0xE20000)
#define VGS_KEY_DOWN *((uint32_t*)0xE20004)
#define VGS_KEY_LEFT *((uint32_t*)0xE20008)
#define VGS_KEY_RIGHT *((uint32_t*)0xE2000C)
#define VGS_KEY_A *((uint32_t*)0xE20010)
#define VGS_KEY_B *((uint32_t*)0xE20014)
#define VGS_KEY_X *((uint32_t*)0xE20018)
#define VGS_KEY_Y *((uint32_t*)0xE2001C)
#define VGS_KEY_START *((uint32_t*)0xE20020)
#define VGS_OUT_EXIT *((int32_t*)0xE7FFFC)
