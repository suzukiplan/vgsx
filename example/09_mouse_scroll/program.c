#include <vgs.h>

#define FONT_PATTERN_INDEX 0
#define FONT_PATTERN_COUNT 128
#define MOUSE_PATTERN_INDEX FONT_PATTERN_COUNT

#define COLUMN_COUNT 26
#define ROW_COUNT 100
#define HEADER_WIDTH 48
#define HEADER_HEIGHT 24
#define CELL_WIDTH 48
#define CELL_HEIGHT 24

#define SCROLL_UNIT_X 12
#define SCROLL_UNIT_Y 12

#define COLOR_BG 0x09121C
#define COLOR_GRID 0x31516C
#define COLOR_CELL 0x163D63
#define COLOR_CELL_HOVER 0x2B6FB0
#define COLOR_HEADER 0x808890
#define COLOR_HEADER_BORDER 0xB8C0C8
#define COLOR_TEXT 0

static int16_t cellValues[ROW_COUNT][COLUMN_COUNT];

static int clamp(int value, int min, int max)
{
    if (value < min) return min;
    if (max < value) return max;
    return value;
}

static int abs_value(int value)
{
    return value < 0 ? -value : value;
}

static int digit_count(int value)
{
    int count = 1;
    int n = abs_value(value);
    while (10 <= n) {
        n /= 10;
        count++;
    }
    return value < 0 ? count + 1 : count;
}

static void value_to_text(int value, char* buf)
{
    int pos = 0;
    int n = value;
    char rev[8];
    int len = 0;

    if (0 == value) {
        buf[0] = 0;
        return;
    }
    if (value < 0) {
        buf[pos++] = '-';
        n = -value;
    }
    do {
        rev[len++] = '0' + (n % 10);
        n /= 10;
    } while (0 < n);
    while (0 < len) {
        buf[pos++] = rev[--len];
    }
    buf[pos] = 0;
}

static void center_print(int bg, int x, int y, int width, int height, const char* text)
{
    int px = x + (width - vgs_pfont_strlen(text)) / 2;
    int py = y + (height - 12) / 2;
    vgs_pfont_print(bg, px, py, COLOR_TEXT, FONT_PATTERN_INDEX, text);
}

static void draw_cell(int bg, int x, int y, int width, int height, uint32_t fill, uint32_t border)
{
    vgs_draw_boxf(bg, x, y, width, height, fill);
    vgs_draw_box(bg, x, y, width, height, border);
}

static void draw_data_cell(int x, int y, int col, int row, int hoverCol, int hoverRow)
{
    int value = cellValues[row][col];
    char text[8];
    uint32_t fill = (col == hoverCol && row == hoverRow) ? COLOR_CELL_HOVER : COLOR_CELL;

    draw_cell(0, x, y, CELL_WIDTH, CELL_HEIGHT, fill, COLOR_GRID);
    value_to_text(value, text);
    if (text[0]) {
        center_print(0, x, y, CELL_WIDTH, CELL_HEIGHT, text);
    }
}

static void draw_headers(int scrollX, int scrollY)
{
    int firstCol = scrollX / CELL_WIDTH;
    int firstRow = scrollY / CELL_HEIGHT;
    int offsetX = scrollX % CELL_WIDTH;
    int offsetY = scrollY % CELL_HEIGHT;

    vgs_draw_boxf(1, 0, 0, vgs_draw_width(), vgs_draw_height(), 0);
    draw_cell(1, 0, 0, HEADER_WIDTH, HEADER_HEIGHT, COLOR_HEADER, COLOR_HEADER_BORDER);

    for (int col = firstCol, sx = HEADER_WIDTH - offsetX; col < COLUMN_COUNT && sx < vgs_draw_width(); col++, sx += CELL_WIDTH) {
        char text[2];
        text[0] = 'A' + col;
        text[1] = 0;
        draw_cell(1, sx, 0, CELL_WIDTH, HEADER_HEIGHT, COLOR_HEADER, COLOR_HEADER_BORDER);
        center_print(1, sx, 0, CELL_WIDTH, HEADER_HEIGHT, text);
    }

    for (int row = firstRow, sy = HEADER_HEIGHT - offsetY; row < ROW_COUNT && sy < vgs_draw_height(); row++, sy += CELL_HEIGHT) {
        char text[12];
        vgs_d32str(text, row + 1);
        draw_cell(1, 0, sy, HEADER_WIDTH, CELL_HEIGHT, COLOR_HEADER, COLOR_HEADER_BORDER);
        center_print(1, 0, sy, HEADER_WIDTH, CELL_HEIGHT, text);
    }
}

static void draw_table(int scrollX, int scrollY, int hoverCol, int hoverRow)
{
    int firstCol = scrollX / CELL_WIDTH;
    int firstRow = scrollY / CELL_HEIGHT;
    int offsetX = scrollX % CELL_WIDTH;
    int offsetY = scrollY % CELL_HEIGHT;

    vgs_draw_boxf(0, 0, 0, vgs_draw_width(), vgs_draw_height(), COLOR_BG);

    for (int row = firstRow, sy = HEADER_HEIGHT - offsetY; row < ROW_COUNT && sy < vgs_draw_height(); row++, sy += CELL_HEIGHT) {
        for (int col = firstCol, sx = HEADER_WIDTH - offsetX; col < COLUMN_COUNT && sx < vgs_draw_width(); col++, sx += CELL_WIDTH) {
            draw_data_cell(sx, sy, col, row, hoverCol, hoverRow);
        }
    }
}

static int screen_to_column(int mouseX, int scrollX)
{
    if (mouseX < HEADER_WIDTH) {
        return -1;
    }
    return (scrollX + mouseX - HEADER_WIDTH) / CELL_WIDTH;
}

static int screen_to_row(int mouseY, int scrollY)
{
    if (mouseY < HEADER_HEIGHT) {
        return -1;
    }
    return (scrollY + mouseY - HEADER_HEIGHT) / CELL_HEIGHT;
}

static void update_value(int col, int row, int delta)
{
    int value;

    if (col < 0 || COLUMN_COUNT <= col || row < 0 || ROW_COUNT <= row) {
        return;
    }
    value = cellValues[row][col] + delta;
    cellValues[row][col] = clamp(value, -1024, 1023);
}

int main(int argc, char* argv[])
{
    const int maxScrollX = COLUMN_COUNT * CELL_WIDTH - (vgs_draw_width() - HEADER_WIDTH);
    const int maxScrollY = ROW_COUNT * CELL_HEIGHT - (vgs_draw_height() - HEADER_HEIGHT);
    int scrollX = 0;
    int scrollY = 0;

    vgs_draw_mode(0, ON);
    vgs_draw_mode(1, ON);
    vgs_draw_mode(2, ON);
    vgs_pfont_init(FONT_PATTERN_INDEX);
    vgs_mouse_setup(MOUSE_PATTERN_INDEX, 0);
    vgs_mouse_hidden(OFF);
    vgs_memset(cellValues, 0, sizeof(cellValues));
    vgs_draw_boxf(2, 0, 0, 48, 24, COLOR_HEADER);
    vgs_draw_box(2, 0, 0, 48, 24, COLOR_HEADER_BORDER);

    while (TRUE) {
        int mouseX;
        int mouseY;
        int hoverCol;
        int hoverRow;
        int wheelV;
        int wheelH;

        vgs_vsync();

        mouseX = vgs_mouse_x();
        mouseY = vgs_mouse_y();
        vgs_mouse_hidden(0 <= mouseX && mouseX < vgs_draw_width() && 0 <= mouseY && mouseY < vgs_draw_height() ? OFF : ON);

        wheelV = vgs_mouse_scroll_vertical();
        wheelH = vgs_mouse_scroll_horizontal();
        if (wheelV) {
            scrollY = clamp(scrollY - wheelV * SCROLL_UNIT_Y, 0, maxScrollY);
        }
        if (wheelH) {
            scrollX = clamp(scrollX - wheelH * SCROLL_UNIT_X, 0, maxScrollX);
        }

        hoverCol = screen_to_column(mouseX, scrollX);
        hoverRow = screen_to_row(mouseY, scrollY);
        if (hoverCol < 0 || COLUMN_COUNT <= hoverCol) hoverCol = -1;
        if (hoverRow < 0 || ROW_COUNT <= hoverRow) hoverRow = -1;

        if (vgs_mouse_left_clicked(&mouseX, &mouseY)) {
            update_value(screen_to_column(mouseX, scrollX), screen_to_row(mouseY, scrollY), 1);
        }
        if (vgs_mouse_right_clicked(&mouseX, &mouseY)) {
            update_value(screen_to_column(mouseX, scrollX), screen_to_row(mouseY, scrollY), -1);
        }

        draw_table(scrollX, scrollY, hoverCol, hoverRow);
        draw_headers(scrollX, scrollY);
    }

    return 0;
}
