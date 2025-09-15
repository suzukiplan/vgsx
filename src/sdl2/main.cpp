#include <SDL.h>
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "vgsx.h"

typedef struct BitmapHeader_ {
    int isize;             /* 情報ヘッダサイズ */
    int width;             /* 幅 */
    int height;            /* 高さ */
    unsigned short planes; /* プレーン数 */
    unsigned short bits;   /* 色ビット数 */
    unsigned int ctype;    /* 圧縮形式 */
    unsigned int gsize;    /* 画像データサイズ */
    int xppm;              /* X方向解像度 */
    int yppm;              /* Y方向解像度 */
    unsigned int cnum;     /* 使用色数 */
    unsigned int inum;     /* 重要色数 */
} BitmapHeader;

static void screenShot()
{
    static unsigned char buf[14 + 40 + VDP_WIDTH * 2 * VDP_HEIGHT * 2 * 4];
    int iSize = (int)sizeof(buf);
    memset(buf, 0, sizeof(buf));
    int ptr = 0;
    buf[ptr++] = 'B';
    buf[ptr++] = 'M';
    memcpy(&buf[ptr], &iSize, 4);
    ptr += 4;
    ptr += 4;
    iSize = 14 + 40;
    memcpy(&buf[ptr], &iSize, 4);
    ptr += 4;
    BitmapHeader header;
    header.isize = 40;
    header.width = VDP_WIDTH * 2;
    header.height = VDP_HEIGHT * 2;
    header.planes = 1;
    header.bits = 32;
    header.ctype = 0;
    header.gsize = header.width * header.height * (header.bits / 8);
    header.xppm = 1;
    header.yppm = 1;
    header.cnum = 0;
    header.inum = 0;
    memcpy(&buf[ptr], &header, sizeof(header));
    ptr += sizeof(header);
    uint32_t* display = vgsx.getDisplay();
    for (int y = 0; y < VDP_HEIGHT; y++) {
        for (int x = 0; x < VDP_WIDTH; x++) {
            auto rgb888 = display[(VDP_HEIGHT - 1 - y) * VDP_WIDTH + x];
            memcpy(&buf[ptr + VDP_WIDTH * 8], &rgb888, 4);
            memcpy(&buf[ptr + VDP_WIDTH * 8 + 4], &rgb888, 4);
            memcpy(&buf[ptr], &rgb888, 4);
            memcpy(&buf[ptr + 4], &rgb888, 4);
            ptr += 8;
        }
        ptr += VDP_WIDTH * 8;
    }
    FILE* fp = fopen("screen.bmp", "wb");
    if (fp) {
        fwrite(buf, 1, sizeof(buf), fp);
        fclose(fp);
        puts("Capture screen.bmp");
    }
    return;
}

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

static void put_usage()
{
    puts("usage: vgsx [-g /path/to/pattern.chr]");
    puts("            [-c /path/to/palette.bin]");
    puts("                /path/to/program.elf");
}

int main(int argc, char* argv[])
{
    const char* programPath = nullptr;
    uint8_t pindex = 0;
    for (int i = 1; i < argc; i++) {
        if ('-' == argv[i][0]) {
            switch (tolower(argv[i][1])) {
                case 'g': {
                    if (argc <= i + 1) {
                        put_usage();
                        return 1;
                    }
                    i++;
                    int size;
                    void* data;
                    data = loadBinary(argv[i], &size);
                    if (!vgsx.loadPattern(pindex, data, size)) {
                        exit(255);
                    } else {
                        if (32 < size) {
                            printf("Loaded character patterns: %d to %d (%s)\n", pindex, pindex + size / 32 - 1, argv[i]);
                        } else {
                            printf("Loaded character pattern: %d\n", pindex);
                        }
                    }
                    free(data);
                    pindex += size / 32;
                    break;
                }
                case 'c': {
                    if (argc <= i + 1) {
                        put_usage();
                        return 1;
                    }
                    i++;
                    int size;
                    void* data;
                    data = loadBinary(argv[i], &size);
                    if (!vgsx.loadPalette(data, size)) {
                        exit(255);
                    } else {
                        printf("Loaded default palette: %s\n", argv[i]);
                    }
                    free(data);
                    break;
                }
                default:
                    put_usage();
                    return 1;
            }
        } else {
            if (programPath) {
                put_usage();
                return 1;
            }
            programPath = argv[i];
        }
    }

    if (!programPath) {
        put_usage();
        return 1;
    }

    uint8_t* program;
    int programSize;
    program = loadBinary(programPath, &programSize);
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
    bool stabled = true;
    double totalClocks = 0.0;
    uint32_t maxClocks = 0;
    while (!quit) {
        loopCount++;
        auto start = std::chrono::system_clock::now();
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_q: quit = true; break;
                    case SDLK_s: screenShot(); break;
                }
            }
        }
        if (!quit) {
            vgsx.tick();
            totalClocks += vgsx.context.frameClocks;
            if (maxClocks < vgsx.context.frameClocks) {
                maxClocks = vgsx.context.frameClocks;
                printf("Update the peak CPU clock rate: %dHz per frame.\n", maxClocks);
            }
            auto vgsDisplay = vgsx.getDisplay();
            auto pcDisplay = (unsigned int*)windowSurface->pixels;
            auto pitch = windowSurface->pitch / windowSurface->format->BytesPerPixel;
            const int offsetX = 0;
            const int offsetY = 0;
            pcDisplay += offsetY;
            for (int y = 0; y < vgsx.getDisplayHeight(); y++) {
                for (int x = 0; x < vgsx.getDisplayWidth(); x++) {
                    uint32_t rgb888 = *vgsDisplay;
                    pcDisplay[offsetX + x * 2] = rgb888;
                    pcDisplay[offsetX + x * 2 + 1] = rgb888;
                    pcDisplay[offsetX + pitch + x * 2] = rgb888;
                    pcDisplay[offsetX + pitch + x * 2 + 1] = rgb888;
                    vgsDisplay++;
                }
                pcDisplay += pitch * 2;
            }
            SDL_UpdateWindowSurface(window);
            // sync 60fps
            std::chrono::duration<double> diff = std::chrono::system_clock::now() - start;
            int us = (int)(diff.count() * 1000000);
            int wait = waitFps60[loopCount % 3];
            if (us < wait) {
                usleep(wait - us);
                if (!stabled) {
                    stabled = true;
                    printf("Frame rate stabilized at 60 fps (%dus per frame)\n", us);
                }
            } else if (stabled) {
                stabled = false;
                printf("warning: Frame rate is lagging (%dus per frame)\n", us);
            }
        }
    }

    printf("\n[RAM DUMP]\n");
    uint8_t prevbin[16];
    uint32_t ramUsage = 0;
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
        ramUsage += 16;
    }

    if (0 < loopCount) {
        totalClocks /= loopCount;
        totalClocks *= 1000.0 / 60.0;
        if (totalClocks < 1000) {
            printf("\nAverage MC68030 Clocks: %.1fHz per second.\n", totalClocks);
        } else if (totalClocks < 1000000) {
            printf("\nAverage MC68030 Clocks: %.1fkHz per second.\n", totalClocks / 1000);
        } else {
            printf("\nAverage MC68030 Clocks: %.1fMHz per second.\n", totalClocks / 1000000);
        }
    }
    maxClocks *= 1000.0 / 60.0;
    if (maxClocks < 1000) {
        printf("Maximum MC68030 Clocks: %dHz per second.\n", maxClocks);
    } else if (maxClocks < 1000000) {
        printf("Maximum MC68030 Clocks: %d.%dkHz per second.\n", maxClocks / 1000, maxClocks % 1000 / 100);
    } else {
        printf("Maximum MC68030 Clocks: %d.%dMHz per second.\n", maxClocks / 1000000, maxClocks % 1000000 / 100000);
    }
    printf("RAM usage: %d/%d (%d%%)\n", ramUsage, 1024 * 1024, ramUsage * 100 / 1024 / 1024);
    return 0;
}
