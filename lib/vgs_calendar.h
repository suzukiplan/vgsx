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
#include <vgs.h>

/**
 * @brief Retrieves the current year in UTC.
 * @return Year value (e.g., 2025)
 */
static inline int vgs_calendar_year() { return VGS_IN_CAL_YEAR; }

/**
 * @brief Retrieves the current month in UTC.
 * @return Month value in the range 1–12 (1 = January)
 */
static inline int vgs_calendar_month() { return VGS_IN_CAL_MONTH; }

/**
 * @brief Retrieves the current day of the month in UTC.
 * @return Day value in the range 1–31
 */
static inline int vgs_calendar_mday() { return VGS_IN_CAL_MDAY; }

/**
 * @brief Retrieves the current hour in UTC.
 * @return Hour value in the range 0–23
 */
static inline int vgs_calendar_hour() { return VGS_IN_CAL_HOUR; }

/**
 * @brief Retrieves the current minute in UTC.
 * @return Minute value in the range 0–59
 */
static inline int vgs_calendar_minute() { return VGS_IN_CAL_MINUTE; }

/**
 * @brief Retrieves the current second in UTC.
 * @return Second value in the range 0–59
 */
static inline int vgs_calendar_second() { return VGS_IN_CAL_SECOND; }
