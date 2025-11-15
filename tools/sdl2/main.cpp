#include <pthread.h>
#include <SDL.h>
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "vgsx.h"

static pthread_mutex_t soundMutex = PTHREAD_MUTEX_INITIALIZER;

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
    header.width = vgsx.getDisplayWidth();
    header.height = vgsx.getDisplayHeight();
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
    for (int y = 0; y < vgsx.getDisplayHeight(); y++) {
        for (int x = 0; x < vgsx.getDisplayWidth(); x++) {
            auto rgb888 = display[(vgsx.getDisplayHeight() - 1 - y) * vgsx.getDisplayWidth() + x];
            memcpy(&buf[ptr], &rgb888, 4);
            ptr += 4;
        }
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
    puts("usage: vgsx [-i]");
    puts("            [-d]");
    puts("            [-g /path/to/pattern.chr]");
    puts("            [-c /path/to/palette.bin]");
    puts("            [-b /path/to/bgm.vgm]");
    puts("            [-s /path/to/sfx.wav]");
    puts("            [-x expected_exit_code]");
    puts("            { /path/to/program.elf | /path/to/program.rom }");
}

static void audioCallback(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, len);
    pthread_mutex_lock(&soundMutex);
    vgsx.tickSound((int16_t*)stream, len / 2);
    pthread_mutex_unlock(&soundMutex);
}

static void file_dump(const char* fname)
{
    FILE* fp = fopen(fname, "rb");
    if (fp) {
        printf("\n[%s]\n", fname);
        uint8_t buf[16];
        int n;
        int offset = 0;
        int totalSize = 0;
        while (0 < (n = fread(buf, 1, 16, fp))) {
            totalSize += n;
            printf("%06X", offset);
            char ascii[17];
            memset(ascii, 0, sizeof(ascii));
            int j;
            for (j = 0; j < n; j++) {
                if (8 == j) {
                    printf(" - %02X", buf[j]);
                } else {
                    printf(" %02X", buf[j]);
                }
                ascii[j] = isprint(buf[j]) ? buf[j] : '.';
            }
            for (; j < 16; j++) {
                if (8 == j) {
                    printf("     ");
                } else {
                    printf("   ");
                }
            }
            printf("  %s\n", ascii);
            offset += 0x10;
        }
        fclose(fp);
        printf("Size: %d bytes\n", totalSize);
    }
}
int main(int argc, char* argv[])
{
    vgsx.setLogCallback([](VGSX::LogLevel level, const char* msg) {
        std::string lv;
        switch (level) {
            case VGSX::LogLevel::I: lv = "info"; break;
            case VGSX::LogLevel::N: lv = "notice"; break;
            case VGSX::LogLevel::W: lv = "warning"; break;
            case VGSX::LogLevel::E: lv = "error"; break;
            default: lv = "unknown"; break;
        }
        printf("[%s] %s\n", lv.c_str(), msg);
    });
    const char* programPath = nullptr;
    uint16_t pindex = 0;
    uint16_t bindex = 0;
    uint8_t sindex = 0;
    bool consoleMode = false;
    int32_t expectedExitCode = 0;
    bool isFirstOption = true;
    bool print_dump = false;
    vgsx.disableBootBios();
    for (int i = 1; i < argc; i++) {
        if ('-' == argv[i][0]) {
            switch (tolower(argv[i][1])) {
                case 'x': {
                    if (argc <= i + 1) {
                        put_usage();
                        return 1;
                    }
                    i++;
                    expectedExitCode = atoi(argv[i]);
                    consoleMode = true;
                    break;
                }
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
                case 'b': {
                    if (argc <= i + 1) {
                        put_usage();
                        return 1;
                    }
                    i++;
                    int size;
                    void* data;
                    data = loadBinary(argv[i], &size);
                    if (!vgsx.loadVgm(bindex, data, size)) {
                        puts(vgsx.getLastError());
                        exit(255);
                    } else {
                        printf("Loaded VGM #%d: %s (%d bytes)\n", bindex, argv[i], size);
                        bindex++;
                    }
                    break;
                }
                case 's': {
                    if (argc <= i + 1) {
                        put_usage();
                        return 1;
                    }
                    i++;
                    int size;
                    void* data;
                    data = loadBinary(argv[i], &size);
                    printf("Loading SFX #%d: %s (%d bytes)\n", sindex, argv[i], size);
                    if (!vgsx.loadWav(sindex, data, size)) {
                        puts(vgsx.getLastError());
                        exit(255);
                    } else {
                        sindex++;
                    }
                    break;
                }
                case 'i':
                    if (!isFirstOption) {
                        puts("The `-i` option must be specified first.");
                        put_usage();
                        return 1;
                    }
                    vgsx.enableBootBios();
                    break;
                case 'd':
                    print_dump = true;
                    break;
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
        isFirstOption = false;
    }

    if (!programPath) {
        put_usage();
        return 1;
    }

    uint8_t* program;
    int programSize;
    program = loadBinary(programPath, &programSize);
    if (0 == memcmp(program, "VGSX", 4)) {
        if (!vgsx.loadRom(program, programSize)) {
            printf("Load failed: %s\n", vgsx.getLastError());
            exit(255);
        }
    } else {
        if (!vgsx.loadProgram(program, programSize)) {
            printf("Load failed: %s\n", vgsx.getLastError());
            exit(255);
        }
    }
    puts("Load succeed.");

    SDL_AudioDeviceID audioDeviceId = 0;
    SDL_Window* window = nullptr;
    SDL_Surface* windowSurface = nullptr;

    if (!consoleMode) {
        SDL_version sdlVersion;
        SDL_GetVersion(&sdlVersion);
        printf("SDL version: %d.%d.%d\n", sdlVersion.major, sdlVersion.minor, sdlVersion.patch);
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS)) {
            printf("SDL_Init failed: %s\n", SDL_GetError());
            exit(-1);
        }

        puts("Initializing AudioDriver");
        SDL_AudioSpec desired;
        SDL_AudioSpec obtained;
        desired.freq = 44100;
        desired.format = AUDIO_S16LSB;
        desired.channels = 2;
        desired.samples = 2048;
        desired.callback = audioCallback;
        desired.userdata = &vgsx;
        audioDeviceId = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
        if (0 == audioDeviceId) {
            printf(" ... SDL_OpenAudioDevice failed: %s\n", SDL_GetError());
            exit(-1);
        }
        printf("- obtained.freq = %d\n", obtained.freq);
        printf("- obtained.format = %X\n", obtained.format);
        printf("- obtained.channels = %d\n", obtained.channels);
        printf("- obtained.samples = %d\n", obtained.samples);

        window = SDL_CreateWindow(
            "VGS-X for SDL2",
            SDL_WINDOWPOS_UNDEFINED,
            SDL_WINDOWPOS_UNDEFINED,
            vgsx.getDisplayWidth(),
            vgsx.getDisplayHeight(),
            0);
        windowSurface = SDL_GetWindowSurface(window);
        if (!windowSurface) {
            printf("SDL_GetWindowSurface failed: %s\n", SDL_GetError());
            exit(-1);
        }
        if (4 != windowSurface->format->BytesPerPixel) {
            printf("unsupported pixel format (support only 4 bytes / pixel)\n");
            exit(-1);
        }
        SDL_UpdateWindowSurface(window);
    }

    printf("Start main loop.\n");
    SDL_Event event;
    unsigned int loopCount = 0;
    const int waitFps60[3] = {17000, 17000, 16000};
    bool quit = false;
    bool stabled = true;
    double totalClocks = 0.0;
    uint32_t maxClocks = 0;
    if (!consoleMode) {
        SDL_PauseAudioDevice(audioDeviceId, 0);
    }
    vgsx.reset();
    while (!quit && !vgsx.isExit()) {
        loopCount++;
        auto start = std::chrono::system_clock::now();
        while (!consoleMode && SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                quit = true;
            } else if (event.type == SDL_KEYDOWN) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: vgsx.key.up = 1; break;
                    case SDLK_DOWN: vgsx.key.down = 1; break;
                    case SDLK_LEFT: vgsx.key.left = 1; break;
                    case SDLK_RIGHT: vgsx.key.right = 1; break;
                    case SDLK_z: vgsx.key.a = 1; break;
                    case SDLK_x: vgsx.key.b = 1; break;
                    case SDLK_a: vgsx.key.x = 1; break;
                    case SDLK_s: vgsx.key.y = 1; break;
                    case SDLK_SPACE: vgsx.key.start = 1; break;
                    case SDLK_q: quit = true; break;
                    case SDLK_r: vgsx.reset(); break;
                    case SDLK_c: screenShot(); break;
                }
            } else if (event.type == SDL_KEYUP) {
                switch (event.key.keysym.sym) {
                    case SDLK_UP: vgsx.key.up = 0; break;
                    case SDLK_DOWN: vgsx.key.down = 0; break;
                    case SDLK_LEFT: vgsx.key.left = 0; break;
                    case SDLK_RIGHT: vgsx.key.right = 0; break;
                    case SDLK_z: vgsx.key.a = 0; break;
                    case SDLK_x: vgsx.key.b = 0; break;
                    case SDLK_a: vgsx.key.x = 0; break;
                    case SDLK_s: vgsx.key.y = 0; break;
                    case SDLK_SPACE: vgsx.key.start = 0; break;
                }
            }
        }
        if (!quit) {
            pthread_mutex_lock(&soundMutex);
            vgsx.tick();
            pthread_mutex_unlock(&soundMutex);
            totalClocks += vgsx.ctx.frameClocks;
            if (maxClocks < vgsx.ctx.frameClocks) {
                maxClocks = vgsx.ctx.frameClocks;
                printf("Update the peak CPU clock rate: %dHz per frame.\n", maxClocks);
            }
            if (!consoleMode) {
                auto vgsDisplay = vgsx.getDisplay();
                auto pcDisplay = (unsigned int*)windowSurface->pixels;
                auto pitch = windowSurface->pitch / windowSurface->format->BytesPerPixel;
                const int offsetX = 0;
                const int offsetY = 0;
                pcDisplay += offsetY;
                for (int y = 0; y < vgsx.getDisplayHeight(); y++) {
                    for (int x = 0; x < vgsx.getDisplayWidth(); x++) {
                        pcDisplay[offsetX + x] = vgsDisplay[x];
                    }
                    pcDisplay += pitch;
                    vgsDisplay += vgsx.getDisplayWidth();
                }
                SDL_UpdateWindowSurface(window);
            }
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

    if (print_dump) {
        printf("\n[RAM DUMP]\n");
        uint8_t prevbin[16];
        uint32_t ramUsage = 0;
        for (int i = 0; i < sizeof(vgsx.ctx.ram); i += 16) {
            if (i != 0) {
                if (0 == memcmp(prevbin, &vgsx.ctx.ram[i], 16)) {
                    continue; // skip same data
                }
            }
            memcpy(prevbin, &vgsx.ctx.ram[i], 16);
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

        file_dump("save.dat");
        for (int i = 0; i < 256; i++) {
            char fname[80];
            snprintf(fname, sizeof(fname), "save%03d.dat", i);
            file_dump(fname);
        }
        printf("RAM usage: %d/%d (%d%%)\n", ramUsage, 1024 * 1024, ramUsage * 100 / 1024 / 1024);
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
    double max = maxClocks;
    max *= 1000.0 / 60.0;
    if (max < 1000) {
        printf("Maximum MC68030 Clocks: %.1fHz per second.\n", max);
    } else if (max < 1000000) {
        printf("Maximum MC68030 Clocks: %.1fkHz per second.\n", max / 1000);
    } else {
        printf("Maximum MC68030 Clocks: %.1fMHz per second.\n", max / 1000000);
    }
    SDL_Quit();

    if (vgsx.isExit()) {
        auto exitCode = vgsx.getExitCode();
        printf("Detect exit: code=%d (0x%08X)\n", exitCode, exitCode);
        if (consoleMode) {
            if (expectedExitCode == exitCode) {
                puts("Excpected!");
                return 0;
            } else {
                puts("Unexpected!");
                return -1;
            }
        } else {
            return exitCode;
        }
    }
    return 0;
}
