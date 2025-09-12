#include <vgs.h>

int main(int argc, char* argv[])
{
    vgs_console_print("Hello, World!\n\0");
    vgs_print_bg(1, 11, 12, 0, "HELLO VGS-X WORLD!");

    vgs_vsync();
    *VGS_VREG_BMP0 = 1;
    uint32_t col = 1;
    int ptr = 0;
    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 320; x++) {
            VGS_VRAM_BG0[ptr++] = col;
        }
        col++;
    }
    return 0;
}
