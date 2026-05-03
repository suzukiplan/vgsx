#include <pthread.h>
#include <SDL.h>
#include <unistd.h>
#include <ctype.h>
#include <cstring>
#include <iostream>
#include <fstream>
#include <chrono>
#include "vgsx.h"

static pthread_mutex_t soundMutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {
extern const unsigned char vgmplay_elf[18852];
};

enum class YmAnalogOption {
    Off,
    Clean,
    Subtle,
    Real,
    Re1e,
    Warm,
};

struct Options {
    const char* inputPath = nullptr;
    YmAnalogOption ymAnalogOption = YmAnalogOption::Real;
};

static void putUsage()
{
    puts("usage: vgmplay [--ym-analog=off|clean|subtle|real|re1e|warm] /path/to/bgm.vgm");
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

static void audioCallback(void* userdata, Uint8* stream, int len)
{
    memset(stream, 0, len);
    pthread_mutex_lock(&soundMutex);
    vgsx.tickSound((int16_t*)stream, len / 2);
    pthread_mutex_unlock(&soundMutex);
}

static bool parseYmAnalogOption(const char* value, YmAnalogOption* option)
{
    if (0 == strcmp(value, "off")) {
        *option = YmAnalogOption::Off;
    } else if (0 == strcmp(value, "clean")) {
        *option = YmAnalogOption::Clean;
    } else if (0 == strcmp(value, "subtle")) {
        *option = YmAnalogOption::Subtle;
    } else if (0 == strcmp(value, "real")) {
        *option = YmAnalogOption::Real;
    } else if (0 == strcmp(value, "re1e")) {
        *option = YmAnalogOption::Re1e;
    } else if (0 == strcmp(value, "warm")) {
        *option = YmAnalogOption::Warm;
    } else {
        return false;
    }
    return true;
}

static bool parseOptions(int argc, char* argv[], Options* options)
{
    for (int i = 1; i < argc; i++) {
        if (0 == strncmp(argv[i], "--ym-analog=", 12)) {
            if (!parseYmAnalogOption(argv[i] + 12, &options->ymAnalogOption)) {
                return false;
            }
        } else if (!argv[i][0] || argv[i][0] == '-') {
            return false;
        } else if (!options->inputPath) {
            options->inputPath = argv[i];
        } else {
            return false;
        }
    }
    return nullptr != options->inputPath;
}

static void applyYmAnalogOption(YmAnalogOption option)
{
    switch (option) {
        case YmAnalogOption::Off:
            vgsx.setYm2612AnalogEnabled(false);
            break;
        case YmAnalogOption::Clean:
            vgsx.useYm2612AnalogCleanPreset();
            break;
        case YmAnalogOption::Subtle:
            vgsx.useYm2612AnalogSubtlePreset();
            break;
        case YmAnalogOption::Real:
            vgsx.useYm2612AnalogRealPreset();
            break;
        case YmAnalogOption::Re1e:
            vgsx.useYm2612AnalogRe1ePreset();
            break;
        case YmAnalogOption::Warm:
            vgsx.useYm2612AnalogWarmPreset();
            break;
    }
}

int main(int argc, char* argv[])
{
    Options options;
    if (!parseOptions(argc, argv, &options)) {
        putUsage();
        exit(1);
    }

    vgsx.loadProgram(vgmplay_elf, sizeof(vgmplay_elf));
    applyYmAnalogOption(options.ymAnalogOption);
    int vgmSize;
    const void* vgm = loadBinary(options.inputPath, &vgmSize);
    if (!vgsx.loadVgm(0, vgm, vgmSize)) {
        printf("%s\n", vgsx.getLastError());
        return -1;
    }

    SDL_AudioDeviceID audioDeviceId = 0;
    SDL_Window* window = nullptr;
    SDL_Surface* windowSurface = nullptr;

    SDL_version sdlVersion;
    SDL_GetVersion(&sdlVersion);
    if (SDL_Init(SDL_INIT_AUDIO)) {
        printf("SDL_Init failed: %s\n", SDL_GetError());
        exit(-1);
    }

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
    SDL_PauseAudioDevice(audioDeviceId, 0);

    // execute 1 frame (program_elf calls vgs_bgm_play in the fast frame)
    pthread_mutex_lock(&soundMutex);
    vgsx.tick();
    pthread_mutex_unlock(&soundMutex);

    // wait enter
    puts("Push enter to exit.");
    fgetc(stdin);

    SDL_Quit();
    return 0;
}
