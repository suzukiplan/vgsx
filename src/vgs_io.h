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

#define VGS_ADDR_VSYNC 0xE00000
#define VGS_ADDR_CONSOLE 0xE00000
#define VGS_ADDR_RANDOM 0xE00004
#define VGS_ADDR_DMA_SOURCE 0xE00008
#define VGS_ADDR_DMA_DESTINATION 0xE0000C
#define VGS_ADDR_DMA_ARGUMENT 0xE00010
#define VGS_ADDR_DMA_EXECUTE 0xE00014
#define VGS_ADDR_ANGLE_X1 0xE00100
#define VGS_ADDR_ANGLE_Y1 0xE00104
#define VGS_ADDR_ANGLE_X2 0xE00108
#define VGS_ADDR_ANGLE_Y2 0xE0010C
#define VGS_ADDR_ANGLE_DEGREE 0xE00110
#define VGS_ADDR_ANGLE_SIN 0xE00114
#define VGS_ADDR_ANGLE_COS 0xE00118
#define VGS_ADDR_VGM_PLAY 0xE01000
#define VGS_ADDR_SFX_PLAY 0xE01100
#define VGS_ADDR_KEY_UP 0xE02000
#define VGS_ADDR_KEY_DOWN 0xE02004
#define VGS_ADDR_KEY_LEFT 0xE02008
#define VGS_ADDR_KEY_RIGHT 0xE0200C
#define VGS_ADDR_KEY_A 0xE02010
#define VGS_ADDR_KEY_B 0xE02014
#define VGS_ADDR_KEY_X 0xE02018
#define VGS_ADDR_KEY_Y 0xE0201C
#define VGS_ADDR_KEY_START 0xE02020
#define VGS_ADDR_SAVE_ADDRESS 0xE03000
#define VGS_ADDR_SAVE_EXECUTE 0xE03004
#define VGS_ADDR_SAVE_CHECK 0xE03008
#define VGS_ADDR_SEQ_OPEN_W 0xE03100
#define VGS_ADDR_SEQ_WRITE 0xE03104
#define VGS_ADDR_SEQ_COMMIT 0xE03108
#define VGS_ADDR_SEQ_OPEN_R 0xE03110
#define VGS_ADDR_SEQ_READ 0xE03114
#define VGS_ADDR_EXIT 0xE7FFFC

#define VGS_IN_VSYNC *((uint32_t*)VGS_ADDR_VSYNC)
#define VGS_OUT_CONSOLE *((uint32_t*)VGS_ADDR_CONSOLE)
#define VGS_IO_RANDOM *((uint32_t*)VGS_ADDR_RANDOM)
#define VGS_OUT_DMA_SOURCE *((uint32_t*)VGS_ADDR_DMA_SOURCE)
#define VGS_OUT_DMA_DESTINATION *((uint32_t*)VGS_ADDR_DMA_DESTINATION)
#define VGS_OUT_DMA_ARGUMENT *((uint32_t*)VGS_ADDR_DMA_ARGUMENT)
#define VGS_IO_DMA_EXECUTE *((uint32_t*)VGS_ADDR_DMA_EXECUTE)
#define VGS_OUT_ANGLE_X1 *((int32_t*)VGS_ADDR_ANGLE_X1)
#define VGS_OUT_ANGLE_Y1 *((int32_t*)VGS_ADDR_ANGLE_Y1)
#define VGS_OUT_ANGLE_X2 *((int32_t*)VGS_ADDR_ANGLE_X2)
#define VGS_OUT_ANGLE_Y2 *((int32_t*)VGS_ADDR_ANGLE_Y2)
#define VGS_IO_ANGLE_DEGREE *((int32_t*)VGS_ADDR_ANGLE_DEGREE)
#define VGS_IN_ANGLE_SIN *((int32_t*)VGS_ADDR_ANGLE_SIN)
#define VGS_IN_ANGLE_COS *((int32_t*)VGS_ADDR_ANGLE_COS)
#define VGS_OUT_VGM_PLAY *((uint32_t*)VGS_ADDR_VGM_PLAY)
#define VGS_OUT_SFX_PLAY *((uint32_t*)VGS_ADDR_SFX_PLAY)
#define VGS_KEY_UP *((uint32_t*)VGS_ADDR_KEY_UP)
#define VGS_KEY_DOWN *((uint32_t*)VGS_ADDR_KEY_DOWN)
#define VGS_KEY_LEFT *((uint32_t*)VGS_ADDR_KEY_LEFT)
#define VGS_KEY_RIGHT *((uint32_t*)VGS_ADDR_KEY_RIGHT)
#define VGS_KEY_A *((uint32_t*)VGS_ADDR_KEY_A)
#define VGS_KEY_B *((uint32_t*)VGS_ADDR_KEY_B)
#define VGS_KEY_X *((uint32_t*)VGS_ADDR_KEY_X)
#define VGS_KEY_Y *((uint32_t*)VGS_ADDR_KEY_Y)
#define VGS_KEY_START *((uint32_t*)VGS_ADDR_KEY_START)
#define VGS_OUT_SAVE_ADDRESS *((uint32_t*)VGS_ADDR_SAVE_ADDRESS)
#define VGS_IO_SAVE_EXECUTE *((uint32_t*)VGS_ADDR_SAVE_EXECUTE)
#define VGS_IN_SAVE_CHECK *((uint32_t*)VGS_ADDR_SAVE_CHECK)
#define VGS_OUT_SEQ_OPEN_W *((uint32_t*)VGS_ADDR_SEQ_OPEN_W)
#define VGS_OUT_SEQ_WRITE *((volatile uint32_t*)VGS_ADDR_SEQ_WRITE)
#define VGS_OUT_SEQ_COMMIT *((uint32_t*)VGS_ADDR_SEQ_COMMIT)
#define VGS_OUT_SEQ_OPEN_R *((uint32_t*)VGS_ADDR_SEQ_OPEN_R)
#define VGS_IN_SEQ_READ *((volatile uint32_t*)VGS_ADDR_SEQ_READ)
#define VGS_OUT_EXIT *((int32_t*)VGS_ADDR_EXIT)
