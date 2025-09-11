#include <iostream>
#include <fstream>
#include "vgsx.h"

static uint8_t* loadBinary(const char* path, int* size)
{
    std::ifstream ifs(path, std::ios::binary);
    ifs.seekg(0, std::ios::end);
    *size = (int)ifs.tellg();
    ifs.seekg(0);
    uint8_t* program = new uint8_t[*size];
    ifs.read((char*)program, *size);
    return program;
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
        puts("Load failed!");
    } else {
        puts("Load succeed.");
    }
    return 0;
}
