#include <vgs.h>

int main(int argc, char* argv[])
{
    vgs_console_print("Hello, World!\n\0");
    vgs_print_bg0(11, 12, 0, "HELLO VGS-X WORLD!");
    return 0;
}
