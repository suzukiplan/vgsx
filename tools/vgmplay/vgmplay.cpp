#include <pthread.h>
#include <SDL.h>
#include <unistd.h>
#include <ctype.h>
#include <iostream>
#include <fstream>
#include <chrono>
#include "vgsx.h"

static pthread_mutex_t soundMutex = PTHREAD_MUTEX_INITIALIZER;

extern "C" {
extern const unsigned char vgmplay_elf[18852];
};

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

int main(int argc, char* argv[])
{
    if (argc < 2) {
        puts("usage: vgmplay /path/to/bgm.vgm");
        exit(1);
    }

    vgsx.loadProgram(vgmplay_elf, sizeof(vgmplay_elf));
    int vgmSize;
    const void* vgm = loadBinary(argv[1], &vgmSize);
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
