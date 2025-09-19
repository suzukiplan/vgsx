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
    VGS_VREG_BMP0 = 1; // BG0: Bitmap Mode
    VGS_VREG_BMP1 = 1; // BG1: Bitmap Mode
    VGS_VREG_BMP2 = 1; // BG2: Bitmap Mode
    VGS_VREG_BMP3 = 0; // BG3: Character Pattern Mode
    VGS_VREG_SPOS = 3; // Sprite: Displayed on top of BG3
    vgs_print_bg(3, 11, 12, 0, "HELLO VGS-X WORLD!");

    uint32_t col = 1;
    int ptr = 0;
    for (int y = 0; y < VRAM_HEIGHT; y++) {
        vgs_draw_line(0, 0, y, VRAM_WIDTH - 1, y, col++);
    }

    for (int x = -1; x < VRAM_WIDTH; x += 32) {
        draw_star(x, 55);
        draw_star(x + 5, 120);
    }

    for (int x = -15; x < VRAM_WIDTH; x += 15) {
        vgs_draw_box(2, x, 45, x + 5, 40, 0x00FFFF);
    }

    for (int x = VRAM_WIDTH + 3; -10 < x; x -= 15) {
        vgs_draw_box(2, x, 150, x + 5, 155, 0x00FFFF);
    }

    vgs_draw_box(2, 11 * 8 - 8, 12 * 8 - 8, 11 * 8 + 19 * 8, 12 * 8 + 16, 0xFFFF00);
    vgs_draw_boxf(2, 11 * 8 - 8 + 1, 12 * 8 - 8 + 1, 11 * 8 + 19 * 8 - 1, 12 * 8 + 16 - 1, 0x102040);
    vgs_draw_box(2, 11 * 8 - 8 - 2, 12 * 8 - 8 - 2, 11 * 8 + 19 * 8 + 2, 12 * 8 + 16 + 2, 0xCFCF00);
    vgs_draw_box(2, 11 * 8 - 8 - 4, 12 * 8 - 8 - 4, 11 * 8 + 19 * 8 + 4, 12 * 8 + 16 + 4, 0x7F7F00);
    vgs_draw_boxf(2, 0, 0, VRAM_WIDTH - 1, 7, 0x00010101);

    for (int y = 0; y < VRAM_HEIGHT; y++) {
        vgs_draw_pixel(1, vgs_rand() % VRAM_WIDTH, y, vgs_rand32());
    }

    vgs_draw_box(2, 20, 90, 20 + 23, 90 + 23, 0xF02020);
    vgs_sprite(0, TRUE, 20, 90, 2, 0, '1');
    vgs_draw_box(2, VRAM_WIDTH - 20 - 32, 86, VRAM_WIDTH - 20 - 32 + 31, 86 + 31, 0xF02020);
    vgs_sprite(1, TRUE, VRAM_WIDTH - 20 - 32, 86, 3, 0, 'A');
    OAM[1].scale = 10;

    vgs_sprite(2, TRUE, VRAM_WIDTH / 2 - 32 - 8, 20, 0, 0, 'V');
    vgs_sprite(3, TRUE, VRAM_WIDTH / 2 - 4, 20, 0, 0, 'G');
    vgs_sprite(4, TRUE, VRAM_WIDTH / 2 + 32, 20, 0, 0, 'S');
    OAM[2].scale = 300;
    OAM[2].rotate = 90;
    OAM[3].scale = 200;
    OAM[3].rotate = 120;
    OAM[4].scale = 100;
    OAM[4].rotate = 150;
    int sa[4] = {10, 10, 10, 10};

    const int32_t baseX = 120;
    const int32_t baseY = 140;
    vgs_sprite(5, TRUE, baseX, baseY, 9, 0, 256);                   // gamepad
    vgs_sprite(6, FALSE, baseX + 14, baseY + 31, 0, 0, 256 + 101);  // up button
    vgs_sprite(7, FALSE, baseX + 14, baseY + 43, 0, 0, 256 + 101);  // down button
    vgs_sprite(8, FALSE, baseX + 7, baseY + 38, 0, 0, 256 + 100);   // left button
    vgs_sprite(9, FALSE, baseX + 19, baseY + 38, 0, 0, 256 + 100);  // right button
    vgs_sprite(10, FALSE, baseX + 33, baseY + 48, 1, 0, 256 + 102); // start button
    vgs_sprite(11, FALSE, baseX + 61, baseY + 44, 0, 0, 256 + 106); // A button
    vgs_sprite(12, FALSE, baseX + 68, baseY + 36, 0, 0, 256 + 106); // B button
    vgs_sprite(13, FALSE, baseX + 54, baseY + 36, 0, 0, 256 + 106); // X button
    vgs_sprite(14, FALSE, baseX + 61, baseY + 28, 0, 0, 256 + 106); // Y button

    vgs_bgm_play(0);
    while (1) {
        vgs_vsync();
        VGS_VREG_SY1 = 1;
        vgs_draw_pixel(1, vgs_rand() % VRAM_WIDTH, 0, vgs_rand32());
        // vgs_draw_character(1, vgs_rand() % 320 - 4, 0, FALSE, 0, 0x20 + (vgs_rand() & 0x3F));
        OAM[0].rotate += 3;

        for (int i = 0; i < 4; i++) {
            OAM[i + 1].scale += sa[i];
            if (400 < OAM[i + 1].scale && 0 < sa[i]) {
                sa[i] = -10;
                if (i == 0) {
                    vgs_sfx_play(0);
                }
            } else if (OAM[i + 1].scale < 20 && sa[i] < 0) {
                sa[i] = 10;
            }
            if (i) {
                OAM[i + 1].rotate += i * 3;
            }
        }
        OAM[6].visible = VGS_KEY_UP;
        OAM[7].visible = VGS_KEY_DOWN;
        OAM[8].visible = VGS_KEY_LEFT;
        OAM[9].visible = VGS_KEY_RIGHT;
        OAM[10].visible = VGS_KEY_START;
        OAM[11].visible = VGS_KEY_A;
        OAM[12].visible = VGS_KEY_B;
        OAM[13].visible = VGS_KEY_X;
        OAM[14].visible = VGS_KEY_Y;
    }

    return 0;
}
