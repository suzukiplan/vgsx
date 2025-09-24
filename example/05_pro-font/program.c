#include <vgs.h>

void print_center(int y, const char* text)
{
    vgs_pfont_print(0, (vgs_draw_width() - vgs_pfont_strlen(text)) / 2, y, 0, 0, text);
}

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, TRUE);
    vgs_pfont_init(0);

    const int lx = 16;
    int y = 8;
    print_center(y, "- Propotional Font for VGS-X -");
    y += 16;
    vgs_pfont_print(0, lx, y, 0, 0, "The VGS-X features hardware capabilities for displaying");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "proportional fonts, which were difficult on retro PCs prior to");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "16-bit systems, enabling beautiful English text display.");

    y += 16;
    vgs_pfont_print(0, lx, y, 0, 0, "To use this function, first call the vgs_pfont_init() function,");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "specifying the starting index of the character pattern.");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "Then the VGS-X VDP analyzes the character pattern,");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "calculates the coordinates for displaying the proportional font,");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "and enables it in VRAM.");

    y += 16;
    vgs_pfont_print(0, lx, y, 0, 0, "Then, you can display a string using a proportional font by");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "calling the vgs_pfont_print() function.");

    y += 16;
    vgs_pfont_print(0, lx, y, 0, 0, "Additionally, the pixel length of the string displayed by the");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "vgs_pfont_print() function can be obtained using the");
    y += 10;
    vgs_pfont_print(0, lx, y, 0, 0, "vgs_pfont_strlen() function.");

    y += 16;
    print_center(y, "Enjoy Programming!");

    while (TRUE) {
        vgs_vsync();
    }
    return 0;
}