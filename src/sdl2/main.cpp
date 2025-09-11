#include <ctype.h>
#include <iostream>
#include <fstream>
#include "vgsx.h"

static uint8_t* loadBinary(const char* path, int* size)
{
    try {
        std::ifstream ifs(path, std::ios::binary);
        ifs.seekg(0, std::ios::end);
        *size = (int)ifs.tellg();
        ifs.seekg(0);
        uint8_t* program = new uint8_t[*size];
        ifs.read((char*)program, *size);
        return program;
    } catch (...) {
        return nullptr;
    }
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        return 1;
    }
    uint8_t* program;
    int programSize;
    program = loadBinary(argv[1], &programSize);
    if (!vgsx.loadProgram(program, programSize)) {
        printf("Load failed: %s\n", vgsx.getLastError());
        exit(255);
    } else {
        puts("Load succeed.");
    }
    vgsx.tick();

    printf("\n[RAM DUMP]\n");
    uint8_t prevbin[16];
    for (int i = 0; i < sizeof(vgsx.context.ram); i += 16) {
        if (i != 0) {
            if (0 == memcmp(prevbin, &vgsx.context.ram[i], 16)) {
                continue; // skip same data
            }
        }
        memcpy(prevbin, &vgsx.context.ram[i], 16);
        printf("%06X", 0xF00000 + i);
        for (int j = 0; j < 16; j++) {
            if (8 == j) {
                printf(" - %02X", prevbin[j]);
            } else {
                printf(" %02X", prevbin[j]);
            }
        }
        printf("  ");
        for (int j = 0; j < 16; j++) {
            if (isprint(prevbin[j])) {
                putc(prevbin[j], stdout);
            } else {
                putc('.', stdout);
            }
        }
        printf("\n");
    }
    return 0;
}
