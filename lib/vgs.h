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
#include "vgs_bgm.h"
#include "vgs_bmpfont.h"
#include "vgs_calendar.h"
#include "vgs_cg.h"
#include "vgs_ctype.h"
#include "vgs_io.h"
#include "vgs_key.h"
#include "vgs_math.h"
#include "vgs_save.h"
#include "vgs_sfx.h"
#include "vgs_stdint.h"
#include "vgs_stdlib.h"
#include "vgs_string.h"
#include "vgs_system.h"

#ifdef __INTELLISENSE__
#define LAMBDA(rettype, ARG_LIST, BODY) (rettype(*) ARG_LIST)0
#else
#define LAMBDA(rettype, ARG_LIST, BODY)               \
    ({                                                \
        rettype __lambda_funcion__ ARG_LIST { BODY; } \
        __lambda_funcion__;                           \
    })
#endif
