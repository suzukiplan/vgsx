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
    *VGS_VREG_BMP1 = 1; // BG1: Bitmap Mode
    *VGS_VREG_BMP2 = 1; // BG2: Bitmap Mode
    *VGS_VREG_BMP3 = 0; // BG3: Character Pattern Mode
    vgs_print_bg(3, 11, 12, 0, "HELLO VGS-X WORLD!");

    vgs_vsync();
    uint32_t col = 1;
    int ptr = 0;
    for (int y = 0; y < 200; y++) {
        vgs_draw_line(0, 0, y, 319, y, col++);
    }

    for (int x = -1; x < 320; x += 32) {
        draw_star(x, 55);
        draw_star(x + 5, 120);
    }

    for (int x = -15; x < 320; x += 15) {
        vgs_draw_box(2, x, 45, x + 5, 40, 0x00FFFF);
    }

    for (int x = 323; -10 < x; x -= 15) {
        vgs_draw_box(2, x, 150, x + 5, 155, 0x00FFFF);
    }

    vgs_draw_box(2, 11 * 8 - 8, 12 * 8 - 8, 11 * 8 + 19 * 8, 12 * 8 + 16, 0xFFFF00);
    vgs_draw_boxf(2, 11 * 8 - 8 + 1, 12 * 8 - 8 + 1, 11 * 8 + 19 * 8 - 1, 12 * 8 + 16 - 1, 0x102040);
    vgs_draw_box(2, 11 * 8 - 8 - 2, 12 * 8 - 8 - 2, 11 * 8 + 19 * 8 + 2, 12 * 8 + 16 + 2, 0xCFCF00);
    vgs_draw_box(2, 11 * 8 - 8 - 4, 12 * 8 - 8 - 4, 11 * 8 + 19 * 8 + 4, 12 * 8 + 16 + 4, 0x7F7F00);
    vgs_draw_boxf(2, 0, 0, 319, 7, 0x00010101);

    for (int y = 0; y < 200; y++) {
        vgs_draw_pixel(1, vgs_rand() % 320, y, vgs_rand32());
    }

    while (1) {
        vgs_vsync();
        *VGS_VREG_SY1 = 1;
        vgs_draw_pixel(1, vgs_rand() % 320, 0, vgs_rand32());
        vgs_draw_character(1, vgs_rand() % 320 - 4, 0, 0, 0, 0x20 + (vgs_rand() & 0x3F));
    }

    return 0;
}
