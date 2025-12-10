#include <pthread.h>
#include <SDL.h>
#include <unistd.h>
#include <ctype.h>
#include <cstdio>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include "vgsx.h"

static pthread_mutex_t soundMutex = PTHREAD_MUTEX_INITIALIZER;

static uint32_t pngCrc32Update(uint32_t crc, const uint8_t* data, size_t length)
{
    static bool initialized = false;
    static uint32_t table[256];
    if (!initialized) {
        for (uint32_t i = 0; i < 256; i++) {
            uint32_t c = i;
            for (int k = 0; k < 8; k++) {
                c = (c & 1) ? (0xEDB88320u ^ (c >> 1)) : (c >> 1);
            }
            table[i] = c;
        }
        initialized = true;
    }
    for (size_t i = 0; i < length; i++) {
        crc = table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }
    return crc;
}

static uint32_t pngCrc32(const char type[4], const uint8_t* data, uint32_t length)
{
    uint32_t crc = pngCrc32Update(0xFFFFFFFFu, reinterpret_cast<const uint8_t*>(type), 4);
    if (length) crc = pngCrc32Update(crc, data, length);
    return crc ^ 0xFFFFFFFFu;
}

static uint32_t pngAdler32(const uint8_t* data, size_t length)
{
    const uint32_t mod = 65521u;
    uint32_t a = 1;
    uint32_t b = 0;
    while (length) {
        size_t block = length > 5552 ? 5552 : length;
        length -= block;
        for (size_t i = 0; i < block; i++) {
            a += data[i];
            b += a;
        }
        a %= mod;
        b %= mod;
        data += block;
    }
    return (b << 16) | a;
}

static bool writePngChunk(FILE* fp, const char type[4], const uint8_t* data, uint32_t length)
{
    uint8_t header[4];
    header[0] = (length >> 24) & 0xFF;
    header[1] = (length >> 16) & 0xFF;
    header[2] = (length >> 8) & 0xFF;
    header[3] = length & 0xFF;
    if (4 != fwrite(header, 1, 4, fp)) return false;
    if (4 != fwrite(type, 1, 4, fp)) return false;
    if (length && length != fwrite(data, 1, length, fp)) return false;
    uint32_t crc = pngCrc32(type, data, length);
    uint8_t crcbuf[4];
    crcbuf[0] = (crc >> 24) & 0xFF;
    crcbuf[1] = (crc >> 16) & 0xFF;
    crcbuf[2] = (crc >> 8) & 0xFF;
    crcbuf[3] = crc & 0xFF;
    return 4 == fwrite(crcbuf, 1, 4, fp);
}

static bool writePng(const char* path, const uint32_t* display, int width, int height)
{
    const size_t stride = (size_t)width * 4;
    std::vector<uint8_t> raw;
    raw.reserve((stride + 1) * height);
    for (int y = 0; y < height; ++y) {
        raw.push_back(0);
        const uint32_t* row = display + y * width;
        for (int x = 0; x < width; ++x) {
            uint32_t pixel = row[x];
            raw.push_back((pixel >> 16) & 0xFF);
            raw.push_back((pixel >> 8) & 0xFF);
            raw.push_back(pixel & 0xFF);
            raw.push_back(0xFF);
        }
    }

    uint32_t adler = pngAdler32(raw.data(), raw.size());
    std::vector<uint8_t> idat;
    size_t blockCount = (raw.size() + 65534) / 65535;
    idat.reserve(raw.size() + blockCount * 5 + 6);
    idat.push_back(0x78);
    idat.push_back(0x01);
    size_t offset = 0;
    while (offset < raw.size()) {
        size_t block = raw.size() - offset;
        if (block > 65535) block = 65535;
        bool finalBlock = (offset + block) == raw.size();
        idat.push_back(finalBlock ? 0x01 : 0x00);
        uint16_t len = (uint16_t)block;
        uint16_t nlen = ~len;
        idat.push_back(len & 0xFF);
        idat.push_back((len >> 8) & 0xFF);
        idat.push_back(nlen & 0xFF);
        idat.push_back((nlen >> 8) & 0xFF);
        idat.insert(idat.end(), raw.begin() + offset, raw.begin() + offset + block);
        offset += block;
    }
    idat.push_back((adler >> 24) & 0xFF);
    idat.push_back((adler >> 16) & 0xFF);
    idat.push_back((adler >> 8) & 0xFF);
    idat.push_back(adler & 0xFF);

    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return false;
    }
    bool result = true;
    const uint8_t signature[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    result = result && 8 == fwrite(signature, 1, 8, fp);
    uint8_t ihdr[13];
    ihdr[0] = (width >> 24) & 0xFF;
    ihdr[1] = (width >> 16) & 0xFF;
    ihdr[2] = (width >> 8) & 0xFF;
    ihdr[3] = width & 0xFF;
    ihdr[4] = (height >> 24) & 0xFF;
    ihdr[5] = (height >> 16) & 0xFF;
    ihdr[6] = (height >> 8) & 0xFF;
    ihdr[7] = height & 0xFF;
    ihdr[8] = 8; // bit depth
    ihdr[9] = 6; // RGBA
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    result = result && writePngChunk(fp, "IHDR", ihdr, sizeof(ihdr));
    result = result && writePngChunk(fp, "IDAT", idat.data(), (uint32_t)idat.size());
    result = result && writePngChunk(fp, "IEND", nullptr, 0);
    fclose(fp);
    return result;
}

static void screenShot()
{
    const char* filename = "screen.png";
    if (writePng(filename, vgsx.getDisplay(), vgsx.getDisplayWidth(), vgsx.getDisplayHeight())) {
        puts("Capture screen.png");
    } else {
        puts("Failed to capture screen.png");
    }
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
