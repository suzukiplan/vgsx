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
 * @brief Set master volume of sound effect
 * @param m Master Volume (0 to 256)
 */
static inline void vgs_sfx_master_volume(uint32_t m) { VGS_IO_SFX_MASTER = m; }

/**
 * @brief Get master volume of sound effect
 * @return Master Volume (0 to 256)
 */
static inline uint32_t vgs_sfx_master_volume_get(void) { return VGS_IO_SFX_MASTER; }

/**
 * @brief Play sound effect
 * @param n Number of SFX (0 to 255)
 */
static inline void vgs_sfx_play(uint8_t n) { VGS_OUT_SFX_PLAY = n; }

/**
 * @brief Stop sound effect
 * @param n Number of SFX (0 to 255)
 */
static inline void vgs_sfx_stop(uint8_t n) { VGS_OUT_SFX_STOP = n; }

/**
 * @brief Stop the all o0f sound effects
 */
static inline void vgs_sfx_stop_all()
{
    for (int i = 0; i < 256; i++) {
        VGS_OUT_SFX_STOP = i;
    }
}

#ifdef __cplusplus
};
#endif
