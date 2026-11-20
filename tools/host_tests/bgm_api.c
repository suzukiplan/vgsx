#include "vgs_bgm.h"

/* Execute the real MC68030 inline API and FPU conversion in the emulator. */
void test_entry(void)
{
    while (1) {
        volatile uint32_t* result = (volatile uint32_t*)0xF01000;
        result[0] = vgs_bgm_position_samples();
        *(volatile double*)(result + 2) = vgs_bgm_position_seconds();
        result[1] = (uint32_t)(vgs_bgm_position_seconds() * 1000.0);
        switch (*(volatile uint32_t*)0xF01020) {
            case 1: vgs_bgm_stop(); break;
            case 2: vgs_bgm_pause(); break;
            case 3: vgs_bgm_resume(); break;
        }
        *(volatile uint32_t*)0xF01020 = 0;
        result[4] = VGS_IN_VSYNC;
    }
}
