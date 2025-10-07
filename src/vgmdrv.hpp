
// VGM Driver for ymfm/OPN2
// BSD 3-Clause License
//
// Copyright (c) 2025, Yoji Suzuki
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
//    contributors may be used to endorse or promote products derived from
//    this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#pragma once

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>
#include "ymfm_opn2.hpp"

// we use an int64_t as emulated time, as a 32.32 fixed point value
using emulated_time = int64_t;

std::map<int, std::string> chips = {
    {0x2C, "YM2612"},
    {0x30, "YM2151"},
    {0x38, "Sega_PCM"},
    {0x40, "RF5C68"},
    {0x44, "YM2203"},
    {0x48, "YM2608"},
    {0x4C, "YM2610/B"},
    {0x50, "YM3812"},
    {0x54, "YM3526"},
    {0x58, "Y8950"},
    {0x5C, "YMF262"},
    {0x60, "YMF278B"},
    {0x64, "YMF271"},
    {0x68, "YMZ280B"},
    {0x6C, "RF5C164"},
    {0x70, "PWM"},
    {0x74, "AY8910"},
    {0x80, "GB_DMG"},
    {0x84, "NES_APU"},
    {0x88, "MultiPCM"},
    {0x8C, "uPD7759"},
    {0x90, "OKIM6258"},
    {0x98, "OKIM6295"},
    {0x9C, "uPD7759"},
    {0xA0, "K054539"},
    {0xA4, "HuC6280"},
    {0xA8, "C140"},
    {0xAC, "K053260"},
    {0xB0, "Pokey"},
    {0xB4, "QSound"},
    {0xB8, "SCSP"},
    {0xC0, "WonderSwan"},
    {0xC4, "VSU"},
    {0xC8, "SAA1099"},
    {0xCC, "ES5503"},
    {0xD0, "ES5506"},
    {0xD8, "X1-010"},
    {0xDC, "C352"},
    {0xE0, "GA20"},
    {0xE4, "Mikey"},
};

enum class ChipType {
    YM2612,
    Unsupported
};

static ChipType getChipType(std::string chipName)
{
    if (chipName == "YM2612") {
        return ChipType::YM2612;
    }
    return ChipType::Unsupported;
}

class VgmDriver : public ymfm::ymfm_interface
{
  private:
    ymfm::ym2612 ym2612;
    std::vector<std::pair<uint32_t, uint8_t>> ym2612_queue;
    bool subscribedLog;
    std::function<void(bool isError, const char* msg)> logCallback;

    struct Context {
        uint32_t version;
        const uint8_t* data;
        size_t size;
        int32_t cursor;
        int32_t loopOffset;
        int32_t wait;
        uint32_t loopCount;
        bool end;
        emulated_time output_start;
        emulated_time pos;
        emulated_time step;
    } vgm;

    emulated_time output_step;
    std::map<ChipType, uint32_t> clocks;
    int channels;

  public:
    VgmDriver(int samples, int channels) : ym2612(*this)
    {
        this->output_step = 0x100000000ull / samples;
        this->channels = channels;
        this->reset();
    }

    ~VgmDriver()
    {
    }

    void subscribeLog(std::function<void(bool, const char*)> callback)
    {
        this->subscribedLog = true;
        this->logCallback = callback;
    }

    void reset()
    {
        memset(&this->vgm, 0, sizeof(this->vgm));
        this->clocks.clear();
        this->ym2612.reset();
        this->ym2612_queue.clear();
    }

    bool load(const uint8_t* data, size_t size)
    {
        this->reset();
        if (size < 0x100) {
            return false;
        }
        if (0 != memcmp("Vgm ", data, 4)) {
            return false;
        }

        memcpy(&vgm.version, &data[0x08], 4);
        if (vgm.version < 0x161) {
            return false;
        }

        vgm.data = data;
        vgm.size = size;

        bool detect_unsupported = false;
        bool detect_supported = false;
        for (int i = 0x2C; i < 0xE8; i += 4) {
            auto it = chips.find(i);
            if (it != chips.end()) {
                uint32_t clocks;
                memcpy(&clocks, &data[it->first], 4);
                if (clocks) {
                    char msg[1024];
                    snprintf(msg, sizeof(msg), "Detected %s: clocks=%uHz ", it->second.c_str(), clocks);
                    auto type = getChipType(it->second);
                    if (type != ChipType::Unsupported) {
                        strcat(msg, "<supported>\n");
                        this->clocks[type] = clocks;
                        switch (type) {
                            case ChipType::YM2612:
                                vgm.step = 0x100000000ull / this->ym2612.sample_rate(clocks);
                                break;
                            case ChipType::Unsupported: break;
                        }
                        detect_supported = true;
                        if (subscribedLog) {
                            logCallback(false, msg);
                        }
                    } else {
                        strcat(msg, "<unsupported!>\n");
                        detect_unsupported = true;
                        if (subscribedLog) {
                            logCallback(true, msg);
                        }
                    }
                }
            }
        }
        if (detect_unsupported || !detect_supported) {
            return false;
        }

        memcpy(&vgm.cursor, &data[0x34], 4);
        vgm.cursor += 0x34;
        memcpy(&vgm.loopOffset, &data[0x1C], 4);
        vgm.loopOffset += vgm.loopOffset ? 0x1C : 0;
        return true;
    }

    void render(int16_t* buf, int samples)
    {
        if (!vgm.data) {
            memset(buf, 0, samples * 2);
            return;
        }
        int cursor = 0;
        while (cursor < samples) {
            if (vgm.wait < 1) {
                this->execute();
            }
            vgm.wait--;
            buf[cursor] = 0;
            if (2 <= this->channels) {
                buf[cursor + 1] = 0;
            }
            for (auto it = this->clocks.begin(); it != this->clocks.end(); it++) {
                switch (it->first) {
                    case ChipType::YM2612: {
                        uint32_t addr1 = 0xffff, addr2 = 0xffff;
                        uint8_t data1 = 0, data2 = 0;

                        // see if there is data to be written; if so, extract it and dequeue
                        if (!ym2612_queue.empty()) {
                            auto front = ym2612_queue.front();
                            addr1 = 0 + 2 * ((front.first >> 8) & 3);
                            data1 = front.first & 0xff;
                            addr2 = addr1 + 1;
                            data2 = front.second;
                            ym2612_queue.erase(ym2612_queue.begin());
                        }

                        // write to the chip
                        if (addr1 != 0xffff) {
                            ym2612.write(addr1, data1);
                            ym2612.write(addr2, data2);
                        }

                        ymfm::ym2612::output_data out;
                        for (; vgm.pos <= vgm.output_start; vgm.pos += vgm.step) {
                            ym2612.generate(&out);
                        }
                        vgm.output_start += output_step;
                        if (this->channels < 2) {
                            buf[cursor++] += out.data[0]; // output left channel only (mono)
                        } else {
                            buf[cursor++] += out.data[0];
                            buf[cursor++] += out.data[1];
                        }
                        break;
                    }
                    case ChipType::Unsupported:
                        break;
                }
            }
        }
    }

    inline uint32_t getLoopCount() { return this->vgm.loopCount; }
    inline bool isEnded() { return this->vgm.end; }
    inline void setEnded() { this->vgm.end = true; }

  private:
    void execute()
    {
        if (!vgm.data || vgm.end) {
            return;
        }
        static uint32_t seq;
        while (vgm.wait < 1) {
            uint8_t cmd = vgm.data[vgm.cursor++];
            switch (cmd) {
                case 0x52:
                case 0xA2: {
                    // YM2612 port 0, write value dd to register aa
                    uint32_t reg = vgm.data[vgm.cursor++];
                    uint8_t data = vgm.data[vgm.cursor++];
                    ym2612_queue.push_back(std::make_pair(reg, data));
                    break;
                }
                case 0x53:
                case 0xA3: {
                    // YM2612 port 1, write value dd to register aa
                    uint32_t reg = vgm.data[vgm.cursor++];
                    uint8_t data = vgm.data[vgm.cursor++];
                    ym2612_queue.push_back(std::make_pair(reg | 0x100, data));
                    break;
                }

                case 0x61: {
                    // Wait nn samples
                    unsigned short nn;
                    memcpy(&nn, &vgm.data[vgm.cursor], 2);
                    vgm.cursor += 2;
                    vgm.wait += nn;
                    break;
                }
                case 0x62: vgm.wait += 735; break;
                case 0x63: vgm.wait += 882; break;
                case 0x66: {
                    // End of sound data
                    if (vgm.loopOffset) {
                        vgm.cursor = vgm.loopOffset;
                        vgm.loopCount++;
                        break;
                    } else {
                        vgm.end = true;
                        return;
                    }
                }

                case 0x90: // Setup Stream Control: 0x90 ss tt pp cc (ignore)
                case 0x91: // Set Stream Data: 0x91 ss dd ll bb (ignore)
                case 0x95: // Start Stream (fast call): 0x95 ss bb bb ff
                    // printf("[DAC]0x%02X: %02X %02X %02X %02X\n", cmd, vgm.data[0], vgm.data[1], vgm.data[2], vgm.data[3]);
                    vgm.cursor += 4;
                    break;

                case 0x92: // Set Stream Frequency: 0x92 ss ff ff ff ff (ignore)
                    // printf("[DAC]0x%02X: %02X %02X %02X %02X %02X\n", cmd, vgm.data[0], vgm.data[1], vgm.data[2], vgm.data[3], vgm.data[4]);
                    vgm.cursor += 5;
                    break;

                case 0x93: // Start Stream: 0x93 ss aa aa aa aa mm ll ll ll ll (ignored)
                    // printf("[DAC]0x%02X: %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\n", cmd, vgm.data[0], vgm.data[1], vgm.data[2], vgm.data[3], vgm.data[4], vgm.data[5], vgm.data[6], vgm.data[7], vgm.data[8], vgm.data[9]);
                    vgm.cursor += 10;
                    break;

                case 0x94: // Stop Stream: 0x94 ss
                    // printf("[DAC]0x%02X: %02X\n", cmd, vgm.data[0]);
                    vgm.cursor++;
                    break;

                default:
                    // unsupported command
                    printf("Detected an unknown VGM command: %02X\n", cmd);
                    vgm.end = true;
                    return;
            }
        }
    }
};
