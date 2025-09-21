#include <vgs.h>

int main(int argc, char* argv[])
{
    vgs_bgm_play(0);
    vgs_sfx_play(0);
    vgs_print_bg(0, 1, 1, 0, "NEW GAME PROJECT READY!");
    vgs_print_bg(0, 1, 3, 0, "LET'S CREATE YOUR AMAZING GAME!");
    while (TRUE) {
        vgs_vsync();
    }
    return 0;
}
