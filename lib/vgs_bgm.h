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
#include "vgs_io.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Set master volume of background music
 * @param m Master Volume (0 to 256)
 */
static inline void vgs_bgm_master_volume(uint32_t m) { VGS_IO_VGM_MASTER = m; }

/**
 * @brief Get master volume of background music
 * @return Master Volume (0 to 256)
 */
static inline uint32_t vgs_bgm_master_volume_get(void) { return VGS_IO_VGM_MASTER; }

/**
 * @brief Play background music
 * @param n Number of Music (0 to 255)
 */
static inline void vgs_bgm_play(uint16_t n) { VGS_OUT_VGM_PLAY = n; }

/**
 * @brief Pause background music
 */
static inline void vgs_bgm_pause(void) { VGS_OUT_VGM_PLAY_OPT = VGS_VGM_OPT_PAUSE; }

/**
 * @brief Resume background music
 */
static inline void vgs_bgm_resume(void) { VGS_OUT_VGM_PLAY_OPT = VGS_VGM_OPT_RESUME; }

/**
 * @brief Fadeout background music
 */
static inline void vgs_bgm_fadeout(void) { VGS_OUT_VGM_PLAY_OPT = VGS_VGM_OPT_FADEOUT; }

/**
 * @brief Check the FM sound chip is available.
 * @param chip Chip ID (0:SSG, 1: OPM, 2:OPN, 3:OPNA, 4:OPNB, 5:OPN2)
 */
static inline int vgs_bgm_chip_check(int chip)
{
    VGS_OUT_FM_CHIP = chip;
    return VGS_IN_FM_CHECK;
}

/**
 * @brief Get a regisgter value of the FM sound chip.
 * @param chip Chip ID (0:SSG, 1: OPM, 2:OPN, 3:OPNA, 4:OPNB, 5:OPN2)
 * @param offset Offset of the register.
 * @return A register value.
 */
static inline uint32_t vgs_bgm_chip_read(int chip, uint32_t offset)
{
    VGS_OUT_FM_CHIP = chip;
    VGS_OUT_FM_REG = offset;
    return VGS_IN_FM_READ;
}

#ifdef __cplusplus
};
#endif
