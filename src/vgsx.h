/**
 * VGS-X
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Yoji Suzuki.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#pragma once
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <functional>
#include "vdp.hpp"

class VGSX
{
  public:
    enum class GamepadType {
        Unknown,
        Keyboard,
        XBOX,
        NintendoSwitch,
        PlayStation,
    };

    enum class ButtonId {
        Unknown = 0, // (Error case)
        A,           // XBOX, Nintendo Switch, Keyboard
        B,           // XBOX, Nintendo Switch
        X,           // XBOX, Nintendo Switch, Keyboard
        Y,           // XBOX, Nintendo Switch
        Z,           // Keyboard
        S,           // Keyboard
        Cross,       // PlayStation
        Circle,      // PlayStation
        Triangle,    // PlayStation
        Square,      // PlayStation
        Start,       // XBOX
        Space,       // Keyboard
        Plus,        // Nintendo Switch
        Options,     // PlayStation
    };

    typedef struct {
        const uint8_t* data;
        size_t size;
    } Binary;

    typedef struct {
        const int16_t* data;
        size_t count;
        bool play;
        size_t index;
    } SfxData;

    typedef struct {
        uint32_t source;
        uint32_t destination;
        uint32_t argument;
    } DMA;

    typedef struct {
        int32_t x1;
        int32_t y1;
        int32_t x2;
        int32_t y2;
        int32_t degree;
    } Angle;

    typedef struct {
        uint32_t address;
        uint32_t size;
    } SaveData;

    typedef struct {
        uint8_t buffer[1024 * 1024];
        uint32_t size;
        uint32_t readOffset;
        uint8_t index;
        uint8_t reserved[3];
    } SequencialData;

    struct Context {
        uint8_t ram[0x100000]; // WRAM (1MB)
        Binary vgmData[0x10000];
        SfxData sfxData[0x100];
        const uint8_t* elf;
        size_t elfSize;
        const uint8_t* program;
        size_t programSize;
        int randomIndex;
        uint32_t frameClocks;
        DMA dma;
        Angle angle;
        SaveData save;
        SequencialData sqw;
        SequencialData sqr;
        ButtonId getNameId;
        bool vgmPause;
        uint32_t vgmFadeout;
        uint32_t vgmMasterVolume;
        uint32_t sfxMasterVolume;
        uint32_t fmChip;
        uint32_t fmOffset;
    } ctx;

    struct KeyStatus {
        uint8_t up;
        uint8_t down;
        uint8_t left;
        uint8_t right;
        uint8_t a;
        uint8_t b;
        uint8_t x;
        uint8_t y;
        uint8_t start;
    } key;

    enum class LogLevel {
        I, // Information
        N, // Notice
        W, // Warning
        E, // Error
    };

    VDP vdp;
    void* vgmdrv;

    VGSX();
    ~VGSX();
    void disableBootBios() { this->bootBios = false; }
    void enableBootBios() { this->bootBios = true; }
    bool loadRom(const void* data, size_t size);
    bool loadProgram(const void* data, size_t size);
    bool loadPattern(uint16_t index, const void* data, size_t size);
    bool loadPalette(const void* data, size_t size);
    bool loadVgm(uint16_t index, const void* data, size_t size);
    bool loadWav(uint8_t index, const void* data, size_t size);
    const char* getLastError() { return this->lastError; }
    void reset();
    void tick();
    void tickSound(int16_t* buf, int samples);
    inline uint32_t* getDisplay() { return this->vdp.ctx.display; }
    inline int getDisplayWidth() { return VDP_WIDTH; }
    inline int getDisplayHeight() { return VDP_HEIGHT; }
    uint32_t inPort(uint32_t address);
    void outPort(uint32_t address, uint32_t value);
    inline bool isExit() { return this->exitFlag; }
    int32_t getExitCode() { return this->exitCode; }
    void putlog(LogLevel level, const char* format, ...);
    void setLogCallback(void (*callback)(LogLevel level, const char* msg)) { this->logCallback = callback; }
    inline void setGamepadType(GamepadType type) { this->gamepadType = type; }
    ButtonId getButtonIdA();
    ButtonId getButtonIdB();
    ButtonId getButtonIdX();
    ButtonId getButtonIdY();
    ButtonId getButtonIdStart();
    void subscribeInput(std::function<uint32_t(uint32_t port)> callback);
    void subscribeOutput(std::function<void(uint32_t port, uint32_t value)> callback);

  private:
    static struct tm* now()
    {
        time_t now = time(nullptr);
        return gmtime(&now);
    }

    bool subscribedInput;
    bool subscribedOutput;
    std::function<uint32_t(uint32_t port)> inputCallback;
    std::function<void(uint32_t port, uint32_t value)> outputCallback;

    bool ignoreReset;
    struct PendingRomData {
        const uint8_t* data;
        int size;
    } pendingRomData;
    bool bootBios;
    bool extractRom(const uint8_t* program, int programSize);
    void (*logCallback)(LogLevel level, const char* msg);
    GamepadType gamepadType;
    bool exitFlag;
    int32_t exitCode;
    char lastError[256];
    void setLastError(const char* format, ...);
    volatile bool detectReferVSync;
    void dmaMemcpy();
    void dmaMemset();
    uint32_t dmaSearch();
    void dmaU2S();
    void u2s(uint8_t* dest, const uint8_t* src);
};

extern VGSX vgsx;
