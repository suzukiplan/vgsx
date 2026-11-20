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
 * @param m Master Volume (0 to 255)
 */
static inline void vgs_bgm_master_volume(uint32_t m) { VGS_IO_VGM_MASTER = m; }

/**
 * @brief Get master volume of background music
 * @return Master Volume (0 to 255)
 */
static inline uint32_t vgs_bgm_master_volume_get(void) { return VGS_IO_VGM_MASTER; }

/**
 * @brief Play background music
 * @param n Number of Music (0 to 255)
 */
static inline void vgs_bgm_play(uint16_t n) { VGS_OUT_VGM_PLAY = n; }

/**
 * @brief Current song position in 44,100Hz VGM samples (generated audio).
 * @return 0..0xFFFFFFFF, saturating; rewinds at loops, holds at natural end.
 * One 32-bit MMIO read gives a coherent snapshot. Play/reset/stop clear it.
 */
static inline uint32_t vgs_bgm_position_samples(void) { return VGS_IN_VGM_POSITION; }

/**
 * @brief Current song position in fractional seconds.
 * Uses the emulated FPU; compare integer samples for lower-cost synchronization.
 */
static inline double vgs_bgm_position_seconds(void) { return (double)vgs_bgm_position_samples() / 44100.0; }

/**
 * @brief Stop background music and clear its position.
 */
static inline void vgs_bgm_stop(void) { VGS_OUT_VGM_PLAY_OPT = VGS_VGM_OPT_STOP; }

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

#ifdef __cplusplus
};
#endif
