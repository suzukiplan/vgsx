/* Host-only MMIO substitute for testing the example's movement and camera. */
#pragma once
#include <assert.h>
#include <stdint.h>
#include <math.h>
#include <string.h>
#define TRUE 1
#define FALSE 0
#define ON 1
static uint32_t test_bg[4][65536];
static uint32_t test_regs[89];
static uint32_t test_console;
#define VGS_OUT_CONSOLE test_console
#define VGS_VREG_SX0 test_regs[2]
#define VGS_VREG_SY0 test_regs[6]
#define VGS_VREG_M7_FOCAL0 test_regs[85]
#define VGS_VREG_M7_FRAC0 test_regs[81]
#define VGS_VREG_M7_DEPTH0 test_regs[77]
#define VGS_VREG_M7_EN0 test_regs[41]
#define VGS_VREG_M7_A0 test_regs[45]
#define VGS_VREG_M7_B0 test_regs[49]
#define VGS_VREG_M7_C0 test_regs[53]
#define VGS_VREG_M7_D0 test_regs[57]
#define VGS_VREG_M7_CX0 test_regs[61]
#define VGS_VREG_M7_CY0 test_regs[65]
#define VGS_VREG_M7_TX0 test_regs[69]
#define VGS_VREG_M7_TY0 test_regs[73]
static inline void vgs_vsync(void) {}
static inline void vgs_cls_bg_all(uint32_t value) { (void)value; memset(test_bg, 0, sizeof(test_bg)); }
static inline void vgs_draw_mode(int bg, int mode) { test_regs[10 + bg] = mode; }
static inline void vgs_skip_bg(int bg, int skip) { test_regs[27 + bg] = skip; }
static inline void vgs_put_bg(int bg, int x, int y, uint32_t value) { test_bg[bg][y * 256 + x] = value; }
static inline int32_t vgs_sin(int32_t degrees) { return (int32_t)(sin(degrees * 3.141592653589793 / 180) * 256); }
static inline int32_t vgs_cos(int32_t degrees) { return (int32_t)(cos(degrees * 3.141592653589793 / 180) * 256); }
static inline int32_t vgs_degree(int32_t x1, int32_t y1, int32_t x2, int32_t y2)
{
    return ((int32_t)(atan2(y2 - y1, x2 - x1) * 180 / 3.141592653589793) + 360) % 360;
}

static unsigned test_pixel_calls;
static uint32_t test_random_state = 1;
static inline uint16_t vgs_rand(void)
{
    test_random_state = test_random_state * 1664525U + 1013904223U;
    return test_random_state >> 16;
}
static inline void vgs_cls_bg(int bg, uint32_t value)
{
    for (int i = 0; i < 65536; i++) test_bg[bg][i] = value;
}
static inline void vgs_draw_pixel(int bg, int x, int y, uint32_t color)
{
    assert(bg >= 0 && bg < 4 && x >= 0 && x < 320 && y >= 0 && y < 200);
    test_bg[bg][y * 320 + x] = color;
    test_pixel_calls++;
}
static inline void vgs_draw_lineH(int bg, int x, int y, int width, uint32_t color)
{
    for (int i = 0; i < width; i++) vgs_draw_pixel(bg, x + i, y, color);
}
