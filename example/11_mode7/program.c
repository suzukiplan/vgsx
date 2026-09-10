#include "vgs.h"

extern const uint8_t rom_map[16392]; // map.c

int main()
{
    while (ON) {
        vgs_vsync();
    }
    return 0;
}
