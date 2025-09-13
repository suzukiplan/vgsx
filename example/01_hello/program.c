#include <vgs.h>

void draw_star(int x, int y)
{
    vgs_draw_line(0, x, y, x - 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x, y, x + 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x + 10, y + 7, x - 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x - 10, y + 7, x + 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x - 10, y + 7, x + 10, y + 7, 0xF0F000);
}

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

    for (int x = -1; x < 320; x += 32) {
        draw_star(x, 50);
        draw_star(x + 5, 120);
    }

    while (1) {
        vgs_draw_pixel(1, vgs_rand() % 320, vgs_rand() % 200, vgs_rand32());
        vgs_vsync();
    }

    return 0;
}
