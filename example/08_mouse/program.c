#include <vgs.h>

#define MESSAGE_X 8
#define MESSAGE_Y 8
#define MESSAGE_WIDTH 304
#define MESSAGE_HEIGHT 14
#define MESSAGE_FRAMES 100
#define FONT_PATTERN_INDEX 0
#define FONT_PATTERN_COUNT 128
#define MOUSE_PATTERN_INDEX FONT_PATTERN_COUNT
#define COORD_WIDTH 120
#define COORD_HEIGHT 14

static char message[64];
static char coord[32];
static int messageFrames = 0;

static void draw_background(void)
{
    for (int y = 0; y < vgs_draw_height(); y++) {
        uint32_t col = 0x101820 + (y << 8) + (y >> 1);
        vgs_draw_lineH(0, 0, y, vgs_draw_width(), col);
    }
    vgs_draw_box(0, 4, 4, 312, 20, 0xE0F0FF);
    vgs_draw_boxf(0, 5, 5, 310, 18, 0x081018);
    vgs_draw_box(0, 10, 40, 300, 60, 0x80C0FF);
    vgs_draw_boxf(0, 11, 41, 298, 58, 0x142030);
    vgs_draw_box(0, 10, 112, 300, 72, 0xE0F0FF);
    vgs_draw_boxf(0, 11, 113, 298, 70, 0x081018);
    vgs_pfont_print(0, 12, 8, 0, FONT_PATTERN_INDEX, "Mouse Example");
    vgs_pfont_print(0, 18, 52, 0, FONT_PATTERN_INDEX, "Move the mouse cursor on the screen.");
    vgs_pfont_print(0, 18, 64, 0, FONT_PATTERN_INDEX, "Left click: show Left Clicked! for 100 frames.");
    vgs_pfont_print(0, 18, 76, 0, FONT_PATTERN_INDEX, "Right click: show Right Clicked! for 100 frames.");
    vgs_pfont_print(0, 18, 128, 0, FONT_PATTERN_INDEX, "The latest click cancels the previous message.");
}

static void draw_message(void)
{
    vgs_draw_boxf(0, MESSAGE_X, MESSAGE_Y, MESSAGE_WIDTH, MESSAGE_HEIGHT, 0x081018);
    if (0 < messageFrames) {
        vgs_pfont_print(0, MESSAGE_X, MESSAGE_Y, 0, FONT_PATTERN_INDEX, message);
    }
}

static void show_message(const char* prefix, int x, int y)
{
    char xbuf[12];
    char ybuf[12];
    vgs_d32str(xbuf, x);
    vgs_d32str(ybuf, y);
    vgs_strcpy(message, prefix);
    vgs_strcat(message, " (");
    vgs_strcat(message, xbuf);
    vgs_strcat(message, ",");
    vgs_strcat(message, ybuf);
    vgs_strcat(message, ")");
    messageFrames = MESSAGE_FRAMES;
    draw_message();
}

static void draw_coord(int x, int y)
{
    char xbuf[12];
    char ybuf[12];
    int textX;
    vgs_d32str(xbuf, x);
    vgs_d32str(ybuf, y);
    vgs_strcpy(coord, "(");
    vgs_strcat(coord, xbuf);
    vgs_strcat(coord, ",");
    vgs_strcat(coord, ybuf);
    vgs_strcat(coord, ")");
    textX = vgs_draw_width() - vgs_pfont_strlen(coord) - 8;
    vgs_draw_boxf(0, vgs_draw_width() - COORD_WIDTH - 8, 8, COORD_WIDTH, COORD_HEIGHT, 0x081018);
    vgs_pfont_print(0, textX, 8, 0, FONT_PATTERN_INDEX, coord);
}

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, ON);
    vgs_pfont_init(FONT_PATTERN_INDEX);
    vgs_mouse_setup(MOUSE_PATTERN_INDEX, 0);
    vgs_mouse_hidden(OFF);

    draw_background();
    draw_message();

    while (TRUE) {
        int x;
        int y;

        vgs_vsync();
        x = vgs_mouse_x();
        y = vgs_mouse_y();
        draw_coord(x, y);
        vgs_mouse_hidden(0 <= x && x < vgs_draw_width() && 0 <= y && y < vgs_draw_height() ? OFF : ON);

        if (vgs_mouse_left_clicked(&x, &y)) {
            show_message("Left Clicked!", x, y);
        } else if (vgs_mouse_right_clicked(&x, &y)) {
            show_message("Right Clicked!", x, y);
        } else if (0 < messageFrames) {
            messageFrames--;
            if (0 == messageFrames) {
                draw_message();
            }
        }
    }

    return 0;
}
