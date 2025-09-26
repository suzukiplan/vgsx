#include <vgs.h>

void put_pfont_n(uint8_t n, int32_t x, int32_t y, uint8_t pal, uint16_t ptn, const char* text, int len)
{
    int32_t dx, dy, width;
    for (; *text && 0 < len; text++, len--) {
        uint8_t c = (uint8_t)*text;
        if (c < 0x80) {
            vgs_pfont_get(c, &dx, &dy, &width);
        } else {
            width = 0;
        }
        if (0 < width) {
            vgs_draw_character(n, x + dx, y + dy, FALSE, pal, ptn + c);
            x += width;
        } else {
            vgs_draw_character(n, x, y, FALSE, pal, ptn + c);
            x += 8;
        }
    }
}

void ram_check()
{
    static uint32_t checked = 0;
    if (0 == checked) {
        vgs_k8x12_print(1, 4, 0, 0xFFFFFF, "RAM CHECK: ");
    }
    if (checked < 1024) {
        char buf[32];
        checked += 4;
        vgs_u32str(buf, checked);
        if (checked < 1024) {
            vgs_strcat(buf, "KB");
        } else {
            vgs_strcat(buf, "KB ... OK!");
        }
        vgs_draw_clear(1, 12 * 4, 0, 32 * 4, 12);
        vgs_k8x12_print(1, 12 * 4, 0, 0xFFFFFF, buf);
    }

    static const char* loading1 = "Now Loading.  ";
    static const char* loading2 = "Now Loading.. ";
    static const char* loading3 = "Now Loading...";
    static const char* loading4 = "Now Loading.. ";
    const char* loading[] = {
        loading1,
        loading2,
        loading3,
        loading4,
    };
    static int n;
    if (n < 196) {
        n++;
        int lx = vgs_draw_width() - vgs_pfont_strlen(loading3) - 4;
        vgs_draw_clear(1, lx, 188, vgs_draw_width() - lx, 12);
        vgs_pfont_print(1, lx, 188, 0, 0, loading[(n & 0b11000) >> 3]);
    } else if (n < 220) {
        n++;
    } else {
        n++;
        int lx = vgs_draw_width() - vgs_pfont_strlen(loading3) - 4;
        vgs_draw_clear(1, lx, 188, vgs_draw_width() - lx, 12);
        vgs_pfont_print(1, lx + (n - 220) * 3, 188, 0, 0, loading[(n & 0b11000) >> 3]);
    }
}

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, ON);
    vgs_draw_mode(1, ON);
    vgs_draw_mode(2, ON);
    vgs_draw_mode(3, ON);
    vgs_sprite_priority(1);
    vgs_pfont_init(0);
    vgs_cls_bg(0, 1);

    vgs_sprite(1, ON, (vgs_draw_width() - 168) / 2, (vgs_draw_height() - 168) / 2, 168 / 8 - 1, 1, 128);
    vgs_oam(1)->scale = 1;
    int logo_scale = 1;
    int logo_rotate = 18;
    int logo_rotate_speed = 1;

    vgs_bgm_play(0);
    int i = 0;
    while (logo_scale < 150) {
        ram_check();
        if (i < 100) {
            i++;
            uint32_t col = ((i / 2) << 8) | i;
            vgs_draw_lineH(0, 0, vgs_draw_height() / 2 - i, vgs_draw_width(), col | 1);
            vgs_draw_lineH(0, 0, vgs_draw_height() / 2 + i, vgs_draw_width(), col | 1);
        }
        logo_scale += 2;
        logo_rotate += logo_rotate_speed;
        if (logo_rotate_speed < 48) {
            logo_rotate_speed++;
        }
        vgs_oam(1)->scale = logo_scale;
        vgs_oam(1)->rotate = logo_rotate;
        vgs_vsync();
    }

    while (100 != logo_scale || logo_rotate % 360) {
        ram_check();
        if (i < 80) {
            i++;
            uint32_t col = ((i / 2) << 8) | i;
            vgs_draw_lineH(0, 0, vgs_draw_height() / 2 - i, vgs_draw_width(), col | 1);
            vgs_draw_lineH(0, 0, vgs_draw_height() / 2 + i, vgs_draw_width(), col | 1);
        }
        if (100 != logo_scale) {
            logo_scale--;
            vgs_oam(1)->scale = logo_scale;
        }
        if (logo_rotate % 360) {
            logo_rotate += logo_rotate_speed;
            vgs_oam(1)->rotate = logo_rotate;
            if (1 < logo_rotate_speed) {
                logo_rotate_speed--;
            }
        }
        vgs_vsync();
    }
    int len = 0;
    const char* t0 = "16-bit Game Console";
    int t0x = (vgs_draw_width() - vgs_pfont_strlen(t0)) / 2;
    const char* t1 = "Video Game System X";
    int t1x = (vgs_draw_width() - vgs_pfont_strlen(t1)) / 2;
    const char* pt = "Powered by MC68030, YM series FM sound chips";
    int ptx = (vgs_draw_width() - vgs_pfont_strlen(pt)) / 2;
    const char* vt = "and SUZUKI PLAN Video Display Processor X";
    int vtx = (vgs_draw_width() - vgs_pfont_strlen(vt)) / 2;
    vgs_sprite(0, ON, (vgs_draw_width() - 168) / 2, (vgs_draw_height() - 168) / 2, 168 / 8 - 1, 1, 128);
    int alpha = 0x010101;
    vgs_oam(0)->alpha = alpha;
    vgs_oam(0)->mask = 0xFFFFFF;
    while (len < 220) {
        ram_check();
        len++;
        put_pfont_n(1, t0x, 50, 1, 0, t0, len);
        put_pfont_n(1, t1x, 60, 0, 0, t1, len / 2);
        put_pfont_n(1, ptx, 158, 1, 0, pt, len);
        put_pfont_n(1, vtx, 168, 1, 0, vt, len - vgs_strlen(pt));
        vgs_vsync();
        if (len < 80) {
            alpha += 0x020202;
            vgs_oam(0)->alpha = alpha;
        } else if (len < 155) {
            alpha -= 0x020202;
            vgs_oam(0)->alpha = alpha;
        }
    }

    for (int i = 0; i < 100; i += 2) {
        vgs_draw_lineH(3, 0, vgs_draw_height() / 2 - i, vgs_draw_width(), 1);
        vgs_draw_lineH(3, 0, vgs_draw_height() / 2 + i, vgs_draw_width(), 1);
        vgs_draw_lineH(3, 0, i + 1, vgs_draw_width(), 1);
        vgs_draw_lineH(3, 0, vgs_draw_height() - i + 1, vgs_draw_width(), 1);
        vgs_vsync();
    }

    vgs_cls_bg_all(1);
    vgs_sprite_hide_all();
    while (ON) {
        vgs_vsync();
    }
    return 0;
}
