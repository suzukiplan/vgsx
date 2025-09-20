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
 * @brief Write save.dat from RAM.
 * @param addr RAM address
 * @param size Save size
 */
void vgs_save(void* addr, uint32_t size);

/**
 * @brief Read save.dat to RAM.
 * @param addr RAM address
 * @return Loaded save data size
 */
uint32_t vgs_load(void* addr);

/**
 * @brief Check save.dat size without to read.
 * @return Loadable save data size
 */
uint32_t vgs_save_check(void);

/**
 * @brief Open a large sequencial file for write.
 * @param index Index of a large sequencial file.
 */
static inline void vgs_seq_open_w(uint8_t index)
{
    VGS_OUT_SEQ_OPEN_W = index;
}

/**
 * @brief Write a byte data to a large sequencial file.
 * @param data Data to write.
 */
static inline void vgs_seq_write(uint8_t data)
{
    VGS_OUT_SEQ_WRITE = data;
}

/**
 * @brief Commit a large sequencial file for write.
 */
static inline void vgs_seq_commit(void)
{
    VGS_OUT_SEQ_COMMIT = 0x12345678;
}

/**
 * @brief Open a large sequencial file for write.
 * @param index Index of a large sequencial file.
 */
static inline void vgs_seq_open_r(uint8_t index)
{
    VGS_OUT_SEQ_OPEN_R = index;
}

/**
 * @brief Read a byte data to a large sequencial file.
 * @return 0x00~0xFF: Valid sequencial data
 * @return 0xFFFFFFFF: EOF
 */
static inline uint32_t vgs_seq_read(void)
{
    return VGS_IN_SEQ_READ;
}

#ifdef __cplusplus
};
#endif
