#include <SDL.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <chrono>
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

    SDL_version sdlVersion;
    SDL_GetVersion(&sdlVersion);
    printf("SDL version: %d.%d.%d\n", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(-1);
    }
    auto window = SDL_CreateWindow(
        "VGS-X for SDL2",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        400,
        0);
    auto windowSurface = SDL_GetWindowSurface(window);
    if (!windowSurface) {
        printf("SDL_GetWindowSurface failed: %s\n", SDL_GetError());
        exit(-1);
    }
    if (4 != windowSurface->format->BytesPerPixel) {
        printf("unsupported pixel format (support only 4 bytes / pixel)\n");
        exit(-1);
    }
    SDL_UpdateWindowSurface(window);

    printf("Start main loop.\n");
    SDL_Event event;
    unsigned int loopCount = 0;
    const int waitFps60[3] = {17000, 17000, 16000};
    bool quit = false;
    while (!quit) {
        loopCount++;
        auto start = std::chrono::system_clock::now();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q: quit = true; break;
                }
            }
        }
        if (!quit) {
            vgsx.tick();
            auto vgsDisplay = vgsx.getDisplay();
            auto pcDisplay = (unsigned int*)windowSurface->pixels;
            auto pitch = windowSurface->pitch / windowSurface->format->BytesPerPixel;
            const int offsetX = 0;
            const int offsetY = 0;
            pcDisplay += offsetY;
            for (int y = 0; y < vgsx.getDisplayHeight(); y++) {
                for (int x = 0; x < vgsx.getDisplayWidth(); x++) {
                    uint32_t rgb888 = *vgsDisplay++;
                    pcDisplay[offsetX + x * 2] = rgb888;
                    pcDisplay[offsetX + x * 2 + 1] = rgb888;
                    pcDisplay[offsetX + pitch + x * 2] = rgb888;
                    pcDisplay[offsetX + pitch + x * 2 + 1] = rgb888;
                }
                pcDisplay += pitch * 2;
            }
        }
    }

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
