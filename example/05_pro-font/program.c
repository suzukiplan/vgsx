#include <vgs.h>

typedef struct {
    int32_t dx;
    int32_t dy;
    int32_t width;
} ProportionalInfo;

const ProportionalInfo pinfo[] = {
    // 0x00 ~ 0x0F
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    // 0x10 ~ 0x1F
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    {0, 0, 6},
    // 0x20 ~ 0x2F
    {0, 0, 4}, // space
    {2, 0, 3}, // !
    {0, 0, 6}, // "
    {0, 0, 7}, // #
    {0, 0, 7}, // $
    {0, 0, 7}, // %
    {0, 0, 7}, // &
    {0, 0, 3}, // '
    {2, 0, 5}, // (
    {0, 0, 5}, // )
    {0, 0, 6}, // *
    {0, 0, 7}, // +
    {0, 1, 3}, // ,
    {0, 0, 7}, // -
    {0, 1, 3}, // .
    {0, 0, 6}, // /
    // 0x30 ~ 0x3F
    {0, 0, 7}, // 0
    {1, 0, 5}, // 1
    {0, 0, 7}, // 2
    {0, 0, 7}, // 3
    {0, 0, 7}, // 4
    {0, 0, 7}, // 5
    {0, 0, 7}, // 6
    {0, 0, 7}, // 7
    {0, 0, 7}, // 8
    {0, 0, 7}, // 9
    {2, 0, 3}, // :
    {2, 0, 3}, // ;
    {2, 0, 5}, // <
    {0, 0, 7}, // =
    {0, 0, 5}, // >
    {0, 0, 7}, // ?
    // 0x40 ~ 0x4F
    {0, 0, 7}, // @
    {0, 0, 7}, // A
    {0, 0, 7}, // B
    {0, 0, 7}, // C
    {0, 0, 7}, // D
    {0, 0, 7}, // E
    {0, 0, 7}, // F
    {0, 0, 7}, // G
    {0, 0, 7}, // H
    {1, 0, 5}, // I
    {0, 0, 7}, // J
    {0, 0, 7}, // K
    {0, 0, 7}, // L
    {0, 0, 7}, // M
    {0, 0, 7}, // N
    {0, 0, 7}, // O
    // 0x50 ~ 0x5F
    {0, 0, 7}, // P
    {0, 0, 7}, // Q
    {0, 0, 7}, // R
    {0, 0, 7}, // S
    {0, 0, 7}, // T
    {0, 0, 7}, // U
    {0, 0, 7}, // V
    {0, 0, 7}, // W
    {0, 0, 7}, // X
    {0, 0, 7}, // Y
    {0, 0, 7}, // Z
    {4, 0, 3}, // [
    {1, 0, 6}, // backslash
    {0, 0, 3}, // ]
    {1, 0, 4}, // ^
    {0, 0, 7}, // _
    // 0x60 ~ 0x6F
    {1, 0, 3}, // `
    {1, 0, 6}, // a
    {1, 0, 5}, // b
    {1, 0, 5}, // c
    {1, 0, 6}, // d
    {1, 0, 5}, // e
    {1, 0, 5}, // f
    {1, 1, 6}, // g
    {1, 0, 5}, // h
    {2, 0, 2}, // i
    {1, 1, 5}, // j
    {1, 0, 5}, // k
    {1, 0, 4}, // l
    {0, 0, 6}, // m
    {1, 0, 5}, // n
    {1, 0, 5}, // o
    // 0x70 ~ 0x7F
    {1, 2, 5}, // p
    {1, 2, 5}, // q
    {1, 0, 5}, // r
    {1, 0, 5}, // s
    {1, 0, 5}, // t
    {1, 0, 6}, // u
    {0, 0, 6}, // v
    {0, 0, 6}, // w
    {0, 0, 6}, // x
    {1, 2, 5}, // y
    {0, 0, 6}, // z
    {3, 0, 5}, // {
    {2, 0, 2}, // |
    {0, 0, 4}, // }
    {0, 0, 5}, // ~
    {0, 0, 0}, // n/a
};

void pfont_render(uint8_t n, int32_t x, int32_t y, const char* text)
{
    for (; *text; text++) {
        uint8_t c = (uint8_t)*text;
        if (c < 0x80) {
            vgs_draw_character(n, x - pinfo[c].dx, y + pinfo[c].dy, FALSE, 0, c);
            x += pinfo[c].width;
            vgs_vsync();
        }
    }
}

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, TRUE);
    pfont_render(0, 8, 8, "This is a test to display proportional fonts on the VGS-X.");
    pfont_render(0, 8, 24, "This was implemented as a software feature, not hardware.");
    pfont_render(0, 8, 40, "For all font images used, we simply define the display coordinate");
    pfont_render(0, 8, 50, "offsets (dx, dy) and width information, then draw the character");
    pfont_render(0, 8, 60, "images in Bitmap Mode.");
    pfont_render(0, 8, 76, "Since proportional coordinate information varies depending on the");
    pfont_render(0, 8, 86, "type of font image, I hesitated to implement this as a hardware");
    pfont_render(0, 8, 96, "feature.");
    pfont_render(0, 8, 112, "However, upon closer inspection, mechanically calculating propor-");
    pfont_render(0, 8, 122, "tional coordinate information isn't difficult. Adjusting Y-coordi-");
    pfont_render(0, 8, 132, "nates like `g`, `j`, `p`, `q` or `y` is a bit trickier, though.");
    pfont_render(0, 8, 148, "An interface that allows mechanical initialization while enabling");
    pfont_render(0, 8, 158, "subsequent updates to proportional definitions would likely be");
    pfont_render(0, 8, 168, "PERFECT!");
    while (TRUE) {
        vgs_vsync();
    }
    return 0;
}