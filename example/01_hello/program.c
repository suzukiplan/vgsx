#include <vgs.h>

void draw_star(int x, int y)
{
    vgs_draw_line(0, x, y, x - 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x, y, x + 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x + 10, y + 7, x - 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x - 10, y + 7, x + 10, y + 20, 0xF0F000);
    vgs_draw_line(0, x - 10, y + 7, x + 10, y + 7, 0xF0F000);
}

void center_print(int y, const char* text)
{
    vgs_pfont_print(0, (vgs_draw_width() - vgs_pfont_strlen(text)) / 2, y, 0, 0, text);
}

int main(int argc, char* argv[])
{
    uint32_t bgmmv = 256;
    uint32_t sfxmv = 256;
    vgs_bgm_master_volume(bgmmv);
    vgs_sfx_master_volume(sfxmv);
    vgs_draw_mode(0, ON);   // BG0: Bitmap Mode
    vgs_draw_mode(1, ON);   // BG1: Bitmap Mode
    vgs_draw_mode(2, ON);   // BG2: Bitmap Mode
    vgs_draw_mode(3, OFF);  // BG3: Character Pattern Mode
    vgs_sprite_priority(3); // Sprite: Displayed on top of BG3
    vgs_pfont_init(0);

    const char* text = "Hello VGS-X World!";
    vgs_print_bg(3,
                 (vgs_chr_width() - vgs_strlen(text)) / 2,
                 12,
                 0,
                 text);

    uint32_t col = 1;
    int ptr = 0;
    for (int y = 0; y < vgs_draw_height(); y++) {
        vgs_draw_lineH(0, 0, y, vgs_draw_width(), col++);
    }

    for (int x = -1; x < vgs_draw_width(); x += 32) {
        draw_star(x, 55);
        draw_star(x + 5, 120);
    }

    for (int x = -15; x < vgs_draw_width(); x += 15) {
        vgs_draw_box(0, x, 40, 5, 5, 0x00FFFF);
    }

    for (int x = vgs_draw_width() + 3; -10 < x; x -= 15) {
        vgs_draw_box(0, x, 150, 5, 5, 0x00FFFF);
    }

    vgs_draw_box(0, 11 * 8 - 8, 12 * 8 - 8, 160, 24, 0xFFFF00);
    vgs_draw_boxf(0, 11 * 8 - 8 + 1, 12 * 8 - 8 + 1, 158, 22, 0x102040);
    vgs_draw_box(0, 11 * 8 - 8 - 2, 12 * 8 - 8 - 2, 164, 28, 0xCFCF00);
    vgs_draw_box(0, 11 * 8 - 8 - 4, 12 * 8 - 8 - 4, 168, 32, 0x7F7F00);
    vgs_draw_boxf(0, 0, 0, vgs_draw_width(), 8, 0x00010101);

    for (int y = 0; y < vgs_draw_height(); y++) {
        vgs_draw_pixel(1, vgs_rand() % vgs_draw_width(), y, vgs_rand32());
    }

    vgs_draw_box(0, 20, 90, 24, 24, 0xF02020);
    vgs_sprite(0, ON, 20, 90, 2, 0, '1');
    vgs_draw_box(0, vgs_draw_width() - 20 - 32, 86, 32, 32, 0xF02020);
    vgs_sprite(1, ON, vgs_draw_width() - 20 - 32, 86, 3, 0, 'A');
    vgs_oam(1)->scale = 10;

    vgs_sprite(2, ON, vgs_draw_width() / 2 - 32 - 8, 20, 0, 0, 'V');
    vgs_sprite(3, ON, vgs_draw_width() / 2 - 4, 20, 0, 0, 'G');
    vgs_sprite(4, ON, vgs_draw_width() / 2 + 32, 20, 0, 0, 'S');
    vgs_oam(2)->scale = 300;
    vgs_oam(2)->rotate = 90;
    vgs_oam(3)->scale = 200;
    vgs_oam(3)->rotate = 120;
    vgs_oam(4)->scale = 100;
    vgs_oam(4)->rotate = 150;
    int sa[4] = {10, 10, 10, 10};

    const int32_t baseX = (vgs_draw_width() - (10 << 3)) / 2;
    const int32_t baseY = 100;
    vgs_sprite(5, ON, baseX, baseY, 9, 0, 128);                   // gamepad
    vgs_sprite(6, OFF, baseX + 14, baseY + 31, 0, 0, 128 + 101);  // up button
    vgs_sprite(7, OFF, baseX + 14, baseY + 43, 0, 0, 128 + 101);  // down button
    vgs_sprite(8, OFF, baseX + 7, baseY + 38, 0, 0, 128 + 100);   // left button
    vgs_sprite(9, OFF, baseX + 19, baseY + 38, 0, 0, 128 + 100);  // right button
    vgs_sprite(10, OFF, baseX + 33, baseY + 48, 1, 0, 128 + 102); // start button
    vgs_sprite(11, OFF, baseX + 61, baseY + 44, 0, 0, 128 + 106); // A button
    vgs_sprite(12, OFF, baseX + 68, baseY + 36, 0, 0, 128 + 106); // B button
    vgs_sprite(13, OFF, baseX + 54, baseY + 36, 0, 0, 128 + 106); // X button
    vgs_sprite(14, OFF, baseX + 61, baseY + 28, 0, 0, 128 + 106); // Y button

    vgs_draw_boxf(0, baseX - 8, baseY + 20, 96, 40, 1);

    center_print(160, "The VGS-X is a 16-bits game console,");
    center_print(170, "powered by MC68030 and YM series FM sound chips.");
    center_print(180, "It's surprisingly easy to develop games,");
    center_print(190, "compatibility can be easily maintained.");

    vgs_sprite(16, ON, 10, 40, 3, 0, 238);
    vgs_sprite(17, ON, 30, 40, 3, 0, 238);
    vgs_sprite(18, ON, 50, 40, 3, 0, 238);
    vgs_sprite(19, ON, 70, 40, 3, 0, 238);

    vgs_oam(16)->alpha = 0xE0E0E0;
    vgs_oam(17)->alpha = 0xA0A0A0;
    vgs_oam(18)->alpha = 0x707070;
    vgs_oam(19)->alpha = 0x303030;
    vgs_oam(16)->mask = 0xFFFF00;
    vgs_oam(17)->mask = 0x00FFFF;
    vgs_oam(18)->mask = 0xFF00FF;
    vgs_oam(19)->mask = 0x4080FF;

    vgs_bgm_play(0);
    BOOL prevA = OFF;
    BOOL prevB = OFF;
    BOOL prevX = OFF;
    BOOL prevY = OFF;
    vgs_draw_window(0, 0, vgs_draw_height() / 2 - 1, vgs_draw_width(), 2);
    int wh = 2;
    while (1) {
        vgs_vsync();
        if (wh < 200) {
            wh += 2;
            vgs_draw_window(0, 0, vgs_draw_height() / 2 - wh / 2, vgs_draw_width(), wh);
        }
        vgs_scroll_y(1, 1);
        vgs_draw_pixel(1, vgs_rand() % vgs_draw_width(), 0, vgs_rand32());
        // vgs_draw_character(1, vgs_rand() % 320 - 4, 0, OFF, 0, 0x20 + (vgs_rand() & 0x3F));
        vgs_oam(0)->rotate += 3;

        for (int i = 0; i < 4; i++) {
            vgs_oam(i + 1)->scale += sa[i];
            if (400 < vgs_oam(i + 1)->scale && 0 < sa[i]) {
                sa[i] = -10;
                if (i == 0) {
                    vgs_sfx_play(0);
                }
            } else if (vgs_oam(i + 1)->scale < 20 && sa[i] < 0) {
                sa[i] = 10;
            }
            if (i) {
                vgs_oam(i + 1)->rotate += i * 3;
            }
        }
        vgs_oam(6)->visible = vgs_key_up();
        vgs_oam(7)->visible = vgs_key_down();
        vgs_oam(8)->visible = vgs_key_left();
        vgs_oam(9)->visible = vgs_key_right();
        vgs_oam(10)->visible = vgs_key_start();
        vgs_oam(11)->visible = vgs_key_a();
        vgs_oam(12)->visible = vgs_key_b();
        vgs_oam(13)->visible = vgs_key_x();
        vgs_oam(14)->visible = vgs_key_y();
        if (!prevA && vgs_key_a()) {
            vgs_bgm_pause();
        }
        if (!prevB && vgs_key_b()) {
            vgs_bgm_resume();
        }
        if (!prevX && vgs_key_x()) {
            vgs_bgm_fadeout();
        }
        if (!prevY && vgs_key_y()) {
            vgs_bgm_play(0);
        }
        prevA = vgs_key_a();
        prevB = vgs_key_b();
        prevX = vgs_key_x();
        prevY = vgs_key_y();
        if (vgs_key_up() && bgmmv < 256) vgs_bgm_master_volume(bgmmv++);
        if (vgs_key_down() && 0 < bgmmv) vgs_bgm_master_volume(bgmmv--);
        if (vgs_key_right() && sfxmv < 256) vgs_sfx_master_volume(sfxmv++);
        if (vgs_key_left() && 0 < sfxmv) vgs_sfx_master_volume(sfxmv--);

        vgs_draw_clear(2, 2, 2, 256, 8); // todo: need specifiec rect clear function

        if (vgs_bgm_master_volume_get()) {
            vgs_draw_boxf(2, 2, 2, vgs_bgm_master_volume_get(), 2, 0xFFFF00);
        }
        if (vgs_sfx_master_volume_get()) {
            vgs_draw_boxf(2, 2, 6, vgs_sfx_master_volume_get(), 2, 0x00FF00);
        }
    }

    return 0;
}
