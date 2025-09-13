#include <vgs.h>

int main(int argc, char* argv[])
{
    vgs_console_print("Hello, World!\n\0");
    *VGS_VREG_BMP0 = 1; // BG0: Bitmap Mode
    *VGS_VREG_BMP1 = 1; // BG0: Bitmap Mode
    *VGS_VREG_BMP2 = 0; // BG1: Character Pattern Mode
    vgs_print_bg(2, 11, 12, 0, "HELLO VGS-X WORLD!");

    vgs_vsync();
    uint32_t col = 1;
    int ptr = 0;
    for (int y = 0; y < 200; y++) {
        vgs_draw_line(0, 0, y, 319, y, col++);
    }

    vgs_draw_line(0, 159, 10, 149, 30, 0xF0F000);
    vgs_draw_line(0, 159, 10, 169, 30, 0xF0F000);
    vgs_draw_line(0, 169, 17, 149, 30, 0xF0F000);
    vgs_draw_line(0, 149, 17, 169, 30, 0xF0F000);
    vgs_draw_line(0, 149, 17, 169, 17, 0xF0F000);

    while (1) {
        vgs_draw_pixel(1, vgs_rand() % 320, vgs_rand() % 200, vgs_rand32());
        vgs_vsync();
    }

    return 0;
}
