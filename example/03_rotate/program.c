#include <vgs.h>

// Index of Pattern
#define P_FONT 0
#define P_PLAYER 128

// Index of OAM
enum OamIndex {
    O_PLAYER,
};

struct GlobalVariables {
    BOOL gameover;
    BOOL open_flag;
    BOOL playback_replay;
    uint8_t key_code;
    struct Player {
        int32_t x;
        int32_t y;
        int32_t degree;
        int32_t speed;
        int32_t max_speed;
        BOOL collision;
    } player;
} g;

void game_init(void)
{
    vgs_cls_bg_all(0);
    vgs_draw_mode(0, TRUE);
    vgs_draw_mode(1, TRUE);
    vgs_sprite_priority(1);

    for (int i = 0; i < 8; i++) {
        uint32_t c1 = 0x0F * i + 0x0F;
        uint32_t c2 = c1;
        c2 <<= 8;
        c2 |= c1;
        c2 <<= 8;
        c2 |= c1;
        vgs_draw_box(1, i, i, vgs_draw_width() - i - 1, vgs_draw_height() - i - 1, c2);
    }
    vgs_draw_boxf(0, 8, 8, vgs_draw_width() - 9, vgs_draw_height() - 9, 0x0F2F60);

    vgs_memset(&g.player, 0, sizeof(g.player));
    g.player.x = ((vgs_draw_width() - 16) / 2) << 8;
    g.player.y = ((vgs_draw_height() - 16) / 2) << 8;
    vgs_sprite(O_PLAYER, TRUE, g.player.x >> 8, g.player.y >> 8, 1, 0, P_PLAYER);
}

void print_center(int diff_y, const char* text)
{
    vgs_print_bg(2, (vgs_chr_width() - vgs_strlen(text)) / 2, vgs_chr_height() / 2 + diff_y, 0, text);
}

BOOL game_main(void)
{
    vgs_vsync();

    // Game Over
    if (g.player.collision) {
        if (!g.gameover) {
            char buf[80];
            print_center(-1, "GAME  OVER");
            vgs_strcpy(buf, "PUSH ");
            vgs_strcat(buf, vgs_button_name(vgs_button_id_start()));
            vgs_strcat(buf, " TO TRY AGAIN");
            print_center(+1, buf);
            vgs_strcpy(buf, "PUSH ");
            vgs_strcat(buf, vgs_button_name(vgs_button_id_y()));
            vgs_strcat(buf, " TO PLAYBACK RETRY");
            print_center(+2, buf);
            g.gameover = TRUE;
            if (!g.playback_replay) {
                vgs_seq_commit();
            }
        }
        g.open_flag = FALSE;
        g.playback_replay = vgs_key_y();
        if (vgs_key_start() || g.playback_replay) {
            g.gameover = FALSE;
            return FALSE;
        }
        return TRUE;
    }

    // Open replay data
    if (!g.open_flag) {
        g.open_flag = TRUE;
        if (g.playback_replay) {
            vgs_seq_open_r(0);
        } else {
            vgs_seq_open_w(0);
        }
    }

    // Acquire key_code
    if (g.playback_replay) {
        g.key_code = vgs_seq_read();
    } else {
        g.key_code = vgs_key_code();
        vgs_seq_write(g.key_code);
    }

    // Turn left or right
    if (vgs_key_code_left(g.key_code)) {
        g.player.degree -= 5;
        vgs_oam(O_PLAYER)->rotate = g.player.degree;
    } else if (vgs_key_code_right(g.key_code)) {
        g.player.degree += 5;
        vgs_oam(O_PLAYER)->rotate = g.player.degree;
    }

    // Set max speed
    if (vgs_key_code_a(g.key_code)) {
        g.player.max_speed = 300;
    } else if (vgs_key_code_b(g.key_code)) {
        g.player.max_speed = 80;
    } else {
        g.player.max_speed = 160;
    }

    // Update current speed
    g.player.speed += g.player.speed < g.player.max_speed ? 4 : -2;

    // Move player
    int32_t vx = vgs_cos(g.player.degree);
    vx *= g.player.speed;
    vx /= 100;
    g.player.x += vx;
    int32_t vy = vgs_sin(g.player.degree);
    vy *= g.player.speed;
    vy /= 100;
    g.player.y += vy;

    int32_t ox = g.player.x >> 8;
    int32_t oy = g.player.y >> 8;
    vgs_oam(O_PLAYER)->x = ox;
    vgs_oam(O_PLAYER)->y = oy;
    vgs_draw_pixel(1, ox + 7, oy + 7, 0xF0F0F0);
    vgs_draw_pixel(1, ox + 8, oy + 7, 0xA0A0A0);
    vgs_draw_pixel(1, ox + 7, oy + 8, 0x808080);
    vgs_draw_pixel(1, ox + 8, oy + 8, 0x505050);

    // Collision check
    g.player.collision = vgs_read_pixel(1, ox + 7 + (vgs_cos(g.player.degree) * 8 / 256), oy + 7 + (vgs_sin(g.player.degree) * 8 / 256));
    // ox < 4 || oy < 4 || vgs_draw_width() - 20 < ox || vgs_draw_height() - 20 < oy;
    return TRUE;
}

int main(int argc, char* argv[])
{
    vgs_memset(&g, 0, sizeof(g));
    while (TRUE) {
        game_init();
        while (game_main()) {
            ;
        }
    }
    return 0;
}
