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
    VGS_VREG_CLSA = 0;
    VGS_VREG_BMP0 = TRUE;
    VGS_VREG_BMP1 = TRUE;
    VGS_VREG_SPOS = 0;

    for (int i = 0; i < 8; i++) {
        uint32_t c1 = 0x0F * i + 0x0F;
        uint32_t c2 = c1;
        c2 <<= 8;
        c2 |= c1;
        c2 <<= 8;
        c2 |= c1;
        vgs_draw_box(1, i, i, VRAM_WIDTH - i - 1, VRAM_HEIGHT - i - 1, c2);
    }
    vgs_draw_boxf(0, 8, 8, VRAM_WIDTH - 9, VRAM_HEIGHT - 9, 0x0F2F60);

    vgs_memset(&g, 0, sizeof(g));
    g.player.x = ((VRAM_WIDTH - 16) / 2) << 8;
    g.player.y = ((VRAM_HEIGHT - 16) / 2) << 8;
    vgs_sprite(O_PLAYER, TRUE, g.player.x >> 8, g.player.y >> 8, 1, 0, P_PLAYER);
}

BOOL game_main(void)
{
    vgs_vsync();

    // Game Over
    if (g.player.collision) {
        if (!g.gameover) {
            vgs_print_bg(2, 15, 11, 0, "GAME  OVER");
            vgs_print_bg(2, 11, 13, 0, "PRESS START BUTTON");
            g.gameover = TRUE;
        }
        return VGS_KEY_START ? FALSE : TRUE;
    }

    // Turn left or right
    if (VGS_KEY_LEFT) {
        g.player.degree -= 5;
        OAM[O_PLAYER].rotate = g.player.degree;
    } else if (VGS_KEY_RIGHT) {
        g.player.degree += 5;
        OAM[O_PLAYER].rotate = g.player.degree;
    }

    // Set max speed
    if (VGS_KEY_A) {
        g.player.max_speed = 300;
    } else if (VGS_KEY_B) {
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
    OAM[O_PLAYER].x = ox;
    OAM[O_PLAYER].y = oy;
    vgs_draw_pixel(0, ox + 7, oy + 7, 0xF10);
    vgs_draw_pixel(0, ox + 8, oy + 7, 0xF30);
    vgs_draw_pixel(0, ox + 7, oy + 8, 0xF30);
    vgs_draw_pixel(0, ox + 8, oy + 8, 0xF50);

    // Collision check
    g.player.collision = ox < 4 || oy < 4 || VRAM_WIDTH - 20 < ox || VRAM_HEIGHT - 20 < oy;
    return TRUE;
}

int main(int argc, char* argv[])
{
    while (TRUE) {
        game_init();
        while (game_main()) {
            ;
        }
    }
    return 0;
}
