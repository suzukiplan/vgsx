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

#define VGS_KEY_BIT_UP 0b00000001
#define VGS_KEY_BIT_DOWN 0b00000010
#define VGS_KEY_BIT_LEFT 0b00000100
#define VGS_KEY_BIT_RIGHT 0b00001000
#define VGS_KEY_BIT_A 0b00010000
#define VGS_KEY_BIT_B 0b00100000
#define VGS_KEY_BIT_X 0b01000000
#define VGS_KEY_BIT_Y 0b10000000

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Check if the up directional pad is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_up() { return VGS_KEY_UP ? TRUE : FALSE; }

/**
 * @brief Check if the down directional pad is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_down() { return VGS_KEY_DOWN ? TRUE : FALSE; }

/**
 * @brief Check if the left directional pad is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_left() { return VGS_KEY_LEFT ? TRUE : FALSE; }

/**
 * @brief Check if the right directional pad is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_right() { return VGS_KEY_RIGHT ? TRUE : FALSE; }

/**
 * @brief Check if the A button is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_a() { return VGS_KEY_A ? TRUE : FALSE; }

/**
 * @brief Check if the B button is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_b() { return VGS_KEY_B ? TRUE : FALSE; }

/**
 * @brief Check if the X button is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_x() { return VGS_KEY_X ? TRUE : FALSE; }

/**
 * @brief Check if the Y button is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_y() { return VGS_KEY_Y ? TRUE : FALSE; }

/**
 * @brief Check if the start button is pressed.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_start() { return VGS_KEY_START ? TRUE : FALSE; }

/**
 * @brief Retrieve the state of the directional pad and ABXY buttons being pressed in `uint8_t` code format.
 * @return Input status in code format.
 * @remark This format is convenient for recording replay data and similar purposes.
 */
uint8_t vgs_key_code(void);

/**
 * @brief Key code check: D-pad Up
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_up(uint8_t code) { return code & VGS_KEY_BIT_UP ? TRUE : FALSE; }

/**
 * @brief Key code check: D-pad Down
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_down(uint8_t code) { return code & VGS_KEY_BIT_DOWN ? TRUE : FALSE; }

/**
 * @brief Key code check: D-pad Left
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_left(uint8_t code) { return code & VGS_KEY_BIT_LEFT ? TRUE : FALSE; }

/**
 * @brief Key code check: D-pad Right
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_right(uint8_t code) { return code & VGS_KEY_BIT_RIGHT ? TRUE : FALSE; }

/**
 * @brief Key code check: A button
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_a(uint8_t code) { return code & VGS_KEY_BIT_A ? TRUE : FALSE; }

/**
 * @brief Key code check: B button
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_b(uint8_t code) { return code & VGS_KEY_BIT_B ? TRUE : FALSE; }

/**
 * @brief Key code check: X button
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_x(uint8_t code) { return code & VGS_KEY_BIT_X ? TRUE : FALSE; }

/**
 * @brief Key code check: Y button
 * @param code The value obtained via `vgs_key_code`.
 * @return TRUE if pressed, FALSE if not pressed.
 */
static inline BOOL vgs_key_code_y(uint8_t code) { return code & VGS_KEY_BIT_Y ? TRUE : FALSE; }

/**
 * @brief Get the type of connected gamepad.
 * @return VGS_KEY_ID_*
 */
static inline uint32_t vgs_key_type(void) { return VGS_IO_KEY_TYPE; }

/**
 * @brief Get the button identifier for the A button on the connected gamepad.
 * @return VGS_BUTTON_ID_*
 */
static inline uint32_t vgs_button_id_a(void) { return VGS_IN_BUTTON_ID_A; }

/**
 * @brief Get the button identifier for the B button on the connected gamepad.
 * @return VGS_BUTTON_ID_*
 */
static inline uint32_t vgs_button_id_b(void) { return VGS_IN_BUTTON_ID_B; }

/**
 * @brief Get the button identifier for the X button on the connected gamepad.
 * @return VGS_BUTTON_ID_*
 */
static inline uint32_t vgs_button_id_x(void) { return VGS_IN_BUTTON_ID_X; }

/**
 * @brief Get the button identifier for the Y button on the connected gamepad.
 * @return VGS_BUTTON_ID_*
 */
static inline uint32_t vgs_button_id_y(void) { return VGS_IN_BUTTON_ID_Y; }

/**
 * @brief Get the button identifier for the START button on the connected gamepad.
 * @return VGS_BUTTON_ID_*
 */
static inline uint32_t vgs_button_id_start(void) { return VGS_IN_BUTTON_ID_START; }

/**
 * @brief Get the name string of the button identifier.
 * @param buttonId Button identifier for retrieving the name string.
 * @return a name string terminated by 0.
 */
const char* vgs_button_name(uint32_t buttonId);

#ifdef __cplusplus
};
#endif
