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
#include <vgs.h>

static volatile uint32_t _vsync;
extern int main(int argc, char* argv[]);
extern void __vgs_vectors(void);
static void (*const __keep_vectors)(void) __attribute__((used)) = __vgs_vectors;

static inline void hang_up(void)
{
    while (1) {
        vgs_vsync();
    }
}

void crt0(void)
{
    vgs_exit(main(0, (char**)0));
    hang_up();
}

void vgs_vsync(void)
{
    _vsync = VGS_IN_VSYNC;
}

void vgs_abort(uint32_t code)
{
    VGS_OUT_ABORT = code;
    hang_up();
}

void _bus_error(void)
{
    vgs_abort(0xDEAD0000);
}

void _address_error(void)
{
    vgs_abort(0xDEAD0001);
}

void _illegal(void)
{
    vgs_abort(0xDEAD0002);
}

void _zero_div(void)
{
    vgs_abort(0xDEAD0003);
}

void _chk_inst(void)
{
    vgs_abort(0xDEAD0004);
}

void _trapv(void)
{
    vgs_abort(0xDEAD0005);
}
