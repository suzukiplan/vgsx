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
#define VGS_ADDR_KEY_TYPE 0xE02100
#define VGS_ADDR_BUTTON_ID_A 0xE02104
#define VGS_ADDR_BUTTON_ID_B 0xE02108
#define VGS_ADDR_BUTTON_ID_X 0xE0210C
#define VGS_ADDR_BUTTON_ID_Y 0xE02110
#define VGS_ADDR_BUTTON_ID_START 0xE02114
#define VGS_ADDR_BUTTON_ID 0xE02118
#define VGS_ADDR_BUTTON_NAME 0xE0211C
#define VGS_ADDR_SAVE_ADDRESS 0xE03000
#define VGS_ADDR_SAVE_EXECUTE 0xE03004
#define VGS_ADDR_SAVE_CHECK 0xE03008
#define VGS_ADDR_SEQ_OPEN_W 0xE03100
#define VGS_ADDR_SEQ_WRITE 0xE03104
#define VGS_ADDR_SEQ_COMMIT 0xE03108
#define VGS_ADDR_SEQ_OPEN_R 0xE03110
#define VGS_ADDR_SEQ_READ 0xE03114
#define VGS_ADDR_EXIT 0xE7FFFC

#define VGS_IN_VSYNC *((volatile uint32_t*)VGS_ADDR_VSYNC)
#define VGS_OUT_CONSOLE *((volatile uint32_t*)VGS_ADDR_CONSOLE)
#define VGS_IO_RANDOM *((volatile uint32_t*)VGS_ADDR_RANDOM)
#define VGS_OUT_DMA_SOURCE *((volatile uint32_t*)VGS_ADDR_DMA_SOURCE)
#define VGS_OUT_DMA_DESTINATION *((volatile uint32_t*)VGS_ADDR_DMA_DESTINATION)
#define VGS_OUT_DMA_ARGUMENT *((volatile uint32_t*)VGS_ADDR_DMA_ARGUMENT)
#define VGS_IO_DMA_EXECUTE *((volatile uint32_t*)VGS_ADDR_DMA_EXECUTE)
#define VGS_OUT_ANGLE_X1 *((volatile int32_t*)VGS_ADDR_ANGLE_X1)
#define VGS_OUT_ANGLE_Y1 *((volatile int32_t*)VGS_ADDR_ANGLE_Y1)
#define VGS_OUT_ANGLE_X2 *((volatile int32_t*)VGS_ADDR_ANGLE_X2)
#define VGS_OUT_ANGLE_Y2 *((volatile int32_t*)VGS_ADDR_ANGLE_Y2)
#define VGS_IO_ANGLE_DEGREE *((volatile int32_t*)VGS_ADDR_ANGLE_DEGREE)
#define VGS_IN_ANGLE_SIN *((volatile int32_t*)VGS_ADDR_ANGLE_SIN)
#define VGS_IN_ANGLE_COS *((volatile int32_t*)VGS_ADDR_ANGLE_COS)
#define VGS_OUT_VGM_PLAY *((volatile uint32_t*)VGS_ADDR_VGM_PLAY)
#define VGS_OUT_SFX_PLAY *((volatile uint32_t*)VGS_ADDR_SFX_PLAY)
#define VGS_KEY_UP *((volatile uint32_t*)VGS_ADDR_KEY_UP)
#define VGS_KEY_DOWN *((volatile uint32_t*)VGS_ADDR_KEY_DOWN)
#define VGS_KEY_LEFT *((volatile uint32_t*)VGS_ADDR_KEY_LEFT)
#define VGS_KEY_RIGHT *((volatile uint32_t*)VGS_ADDR_KEY_RIGHT)
#define VGS_KEY_A *((volatile uint32_t*)VGS_ADDR_KEY_A)
#define VGS_KEY_B *((volatile uint32_t*)VGS_ADDR_KEY_B)
#define VGS_KEY_X *((volatile uint32_t*)VGS_ADDR_KEY_X)
#define VGS_KEY_Y *((volatile uint32_t*)VGS_ADDR_KEY_Y)
#define VGS_KEY_START *((volatile uint32_t*)VGS_ADDR_KEY_START)
#define VGS_IO_KEY_TYPE *((volatile uint32_t*)VGS_ADDR_KEY_TYPE)
#define VGS_IN_BUTTON_ID_A *((volatile uint32_t*)VGS_ADDR_BUTTON_ID_A)
#define VGS_IN_BUTTON_ID_B *((volatile uint32_t*)VGS_ADDR_BUTTON_ID_B)
#define VGS_IN_BUTTON_ID_X *((volatile uint32_t*)VGS_ADDR_BUTTON_ID_X)
#define VGS_IN_BUTTON_ID_Y *((volatile uint32_t*)VGS_ADDR_BUTTON_ID_Y)
#define VGS_IN_BUTTON_ID_START *((volatile uint32_t*)VGS_ADDR_BUTTON_ID_START)
#define VGS_OUT_BUTTON_ID *((volatile uint32_t*)VGS_ADDR_BUTTON_ID)
#define VGS_OUT_BUTTON_NAME *((volatile uint32_t*)VGS_ADDR_BUTTON_NAME)
#define VGS_OUT_SAVE_ADDRESS *((volatile uint32_t*)VGS_ADDR_SAVE_ADDRESS)
#define VGS_IO_SAVE_EXECUTE *((volatile uint32_t*)VGS_ADDR_SAVE_EXECUTE)
#define VGS_IN_SAVE_CHECK *((volatile uint32_t*)VGS_ADDR_SAVE_CHECK)
#define VGS_OUT_SEQ_OPEN_W *((volatile uint32_t*)VGS_ADDR_SEQ_OPEN_W)
#define VGS_OUT_SEQ_WRITE *((volatile uint32_t*)VGS_ADDR_SEQ_WRITE)
#define VGS_OUT_SEQ_COMMIT *((volatile uint32_t*)VGS_ADDR_SEQ_COMMIT)
#define VGS_OUT_SEQ_OPEN_R *((volatile uint32_t*)VGS_ADDR_SEQ_OPEN_R)
#define VGS_IN_SEQ_READ *((volatile uint32_t*)VGS_ADDR_SEQ_READ)
#define VGS_OUT_EXIT *((volatile int32_t*)VGS_ADDR_EXIT)

#define VGS_KEY_ID_UNKNOWN 0
#define VGS_KEY_ID_KEYBOARD 1
#define VGS_KEY_ID_XBOX 2
#define VGS_KEY_ID_SWITCH 3
#define VGS_KEY_ID_PS 4

#define VGS_BUTTON_ID_UNKNOWN 0
#define VGS_BUTTON_ID_A 1
#define VGS_BUTTON_ID_B 2
#define VGS_BUTTON_ID_X 3
#define VGS_BUTTON_ID_Y 4
#define VGS_BUTTON_ID_Z 5
#define VGS_BUTTON_ID_S 6
#define VGS_BUTTON_ID_CROSS 7
#define VGS_BUTTON_ID_CIRCLE 8
#define VGS_BUTTON_ID_TRIANGLE 9
#define VGS_BUTTON_ID_SQUARE 10
#define VGS_BUTTON_ID_START 11
#define VGS_BUTTON_ID_SPACE 12
#define VGS_BUTTON_ID_PLUS 13
#define VGS_BUTTON_ID_OPTIONS 14
