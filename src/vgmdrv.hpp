
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
#include <algorithm>
#include <array>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <functional>
#include <cmath>
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
    static constexpr int YM2612ChannelCount = 6;
    static constexpr size_t OutputChannelCount = 2;
    ymfm::ym2612 ym2612;
    std::vector<std::pair<uint32_t, uint8_t>> ym2612_queue;
    std::array<uint8_t, YM2612ChannelCount> ym2612_frequency_low;
    std::array<uint8_t, YM2612ChannelCount> ym2612_frequency_high;
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
    float postAmp;
    bool dcCutEnabled;
    float dcCutAlpha;
    std::array<float, OutputChannelCount> dcCutLastInput;
    std::array<float, OutputChannelCount> dcCutLastOutput;

  public:
    struct Ym2612AnalogConfig {
        bool enabled;
        float hpAlpha;
        float lpAlpha;
        float asymPosGain;
        float asymNegGain;
        float asymPosCurve;
        float asymNegCurve;
        bool busSaturationEnabled;
        float saturatorDrive;
        float outputGain;
    };

  private:
    struct Ym2612AnalogState {
        std::array<float, OutputChannelCount> hpLastInput;
        std::array<float, OutputChannelCount> hpLastOutput;
        std::array<float, OutputChannelCount> lpLastOutput;

        void reset()
        {
            this->hpLastInput.fill(0.0f);
            this->hpLastOutput.fill(0.0f);
            this->lpLastOutput.fill(0.0f);
        }
    };

    Ym2612AnalogConfig ym2612AnalogConfig;
    Ym2612AnalogState ym2612AnalogState;

  public:
    VgmDriver(int samples, int channels) : ym2612(*this)
    {
        this->output_step = 0x100000000ull / samples;
        this->channels = channels;
        this->postAmp = 1.55f;
        this->dcCutEnabled = true;
        this->dcCutAlpha = 0.995f;
        this->ym2612AnalogConfig = this->makeYm2612AnalogSubtlePreset();
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

    void setYm2612ChipType(ymfm::ym2612::chip_type type)
    {
        this->ym2612.set_chip_type(type);
    }

    void setPostAmp(float value)
    {
        this->postAmp = value;
    }

    void setDcCutEnabled(bool enabled)
    {
        this->dcCutEnabled = enabled;
        this->dcCutLastInput.fill(0.0f);
        this->dcCutLastOutput.fill(0.0f);
    }

    void setDcCutAlpha(float alpha)
    {
        if (alpha < 0.0f) {
            alpha = 0.0f;
        } else if (alpha > 1.0f) {
            alpha = 1.0f;
        }
        this->dcCutAlpha = alpha;
    }

    static Ym2612AnalogConfig makeYm2612AnalogCleanPreset()
    {
        return {
            false,
            0.9965f,
            0.40f,
            1.0f,
            1.0f,
            0.0f,
            0.0f,
            false,
            1.0f,
            1.0f,
        };
    }

    static Ym2612AnalogConfig makeYm2612AnalogSubtlePreset()
    {
        return {
            true,
            0.9965f,
            0.34f,
            1.000f,
            0.994f,
            0.010f,
            0.016f,
            true,
            1.05f,
            0.985f,
        };
    }

    static Ym2612AnalogConfig makeYm2612AnalogWarmPreset()
    {
        return {
            true,
            0.9955f,
            0.27f,
            1.002f,
            0.990f,
            0.016f,
            0.024f,
            true,
            1.10f,
            0.965f,
        };
    }

    void setYm2612AnalogConfig(const Ym2612AnalogConfig& config)
    {
        this->ym2612AnalogConfig = config;
        this->clampYm2612AnalogConfig();
        this->ym2612AnalogState.reset();
    }

    const Ym2612AnalogConfig& getYm2612AnalogConfig() const
    {
        return this->ym2612AnalogConfig;
    }

    void setYm2612AnalogEnabled(bool enabled)
    {
        this->ym2612AnalogConfig.enabled = enabled;
        this->ym2612AnalogState.reset();
    }

    void useYm2612AnalogCleanPreset()
    {
        this->setYm2612AnalogConfig(this->makeYm2612AnalogCleanPreset());
    }

    void useYm2612AnalogSubtlePreset()
    {
        this->setYm2612AnalogConfig(this->makeYm2612AnalogSubtlePreset());
    }

    void useYm2612AnalogWarmPreset()
    {
        this->setYm2612AnalogConfig(this->makeYm2612AnalogWarmPreset());
    }

    void reset()
    {
        memset(&this->vgm, 0, sizeof(this->vgm));
        this->clocks.clear();
        this->ym2612.reset();
        this->ym2612_queue.clear();
        this->ym2612_frequency_low.fill(0);
        this->ym2612_frequency_high.fill(0);
        this->dcCutLastInput.fill(0.0f);
        this->dcCutLastOutput.fill(0.0f);
        this->ym2612AnalogState.reset();
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
                        strcat(msg, "<supported>");
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
                        strcat(msg, "<unsupported!>");
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
            int32_t mixed[2] = {0, 0};
            for (auto it = this->clocks.begin(); it != this->clocks.end(); it++) {
                switch (it->first) {
                    case ChipType::YM2612: {
                        uint32_t addr1 = 0xffff, addr2 = 0xffff;
                        uint32_t reg = 0;
                        uint8_t data1 = 0, data2 = 0;

                        // see if there is data to be written; if so, extract it and dequeue
                        if (!ym2612_queue.empty()) {
                            auto front = ym2612_queue.front();
                            reg = front.first;
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
                            this->updateYm2612Frequency(reg, data2);
                        }

                        ymfm::ym2612::output_data out;
                        for (; vgm.pos <= vgm.output_start; vgm.pos += vgm.step) {
                            ym2612.generate(&out);
                        }
                        vgm.output_start += output_step;
                        mixed[0] += out.data[0];
                        mixed[1] += out.data[1];
                        break;
                    }
                    case ChipType::Unsupported:
                        break;
                }
            }

            int16_t left = this->postProcessSample(0, mixed[0]);
            int16_t right = this->postProcessSample(1, mixed[1]);
            if (this->channels < 2) {
                buf[cursor++] = left;
            } else {
                buf[cursor++] = left;
                buf[cursor++] = right;
            }
        }
    }

    inline uint32_t getLoopCount() { return this->vgm.loopCount; }
    inline bool isEnded() { return this->vgm.end; }
    inline void setEnded() { this->vgm.end = true; }

    int getFrequency(ChipType type, int ch)
    {
        if (type != ChipType::YM2612 || ch < 0 || YM2612ChannelCount <= ch) {
            return -1;
        }
        return ((this->ym2612_frequency_high[ch] & 0x3f) << 8) | this->ym2612_frequency_low[ch];
    }
    uint32_t getChannelVolume(ChipType type, int ch)
    {
        if (type != ChipType::YM2612 || ch < 0 || YM2612ChannelCount <= ch) {
            return 0;
        }
        return this->ym2612.channel_volume(static_cast<uint32_t>(ch));
    }

  private:
    static float clampUnit(float value)
    {
        if (value < -1.0f) {
            return -1.0f;
        }
        if (value > 1.0f) {
            return 1.0f;
        }
        return value;
    }

    void clampYm2612AnalogConfig()
    {
        this->ym2612AnalogConfig.hpAlpha = std::clamp(this->ym2612AnalogConfig.hpAlpha, 0.0f, 1.0f);
        this->ym2612AnalogConfig.lpAlpha = std::clamp(this->ym2612AnalogConfig.lpAlpha, 0.0f, 1.0f);
        this->ym2612AnalogConfig.asymPosGain = std::clamp(this->ym2612AnalogConfig.asymPosGain, 0.5f, 1.5f);
        this->ym2612AnalogConfig.asymNegGain = std::clamp(this->ym2612AnalogConfig.asymNegGain, 0.5f, 1.5f);
        this->ym2612AnalogConfig.asymPosCurve = std::clamp(this->ym2612AnalogConfig.asymPosCurve, 0.0f, 0.25f);
        this->ym2612AnalogConfig.asymNegCurve = std::clamp(this->ym2612AnalogConfig.asymNegCurve, 0.0f, 0.25f);
        this->ym2612AnalogConfig.saturatorDrive = std::clamp(this->ym2612AnalogConfig.saturatorDrive, 1.0f, 2.0f);
        this->ym2612AnalogConfig.outputGain = std::clamp(this->ym2612AnalogConfig.outputGain, 0.0f, 2.0f);
    }

    float applyLegacyDcCut(size_t channel, float value)
    {
        if (!this->dcCutEnabled) {
            return value;
        }
        float filtered = value - this->dcCutLastInput[channel] + (this->dcCutAlpha * this->dcCutLastOutput[channel]);
        this->dcCutLastInput[channel] = value;
        this->dcCutLastOutput[channel] = filtered;
        return filtered;
    }

    float applyYm2612AnalogPipeline(size_t channel, float sample)
    {
        const Ym2612AnalogConfig& cfg = this->ym2612AnalogConfig;

        // Gentle DC cut keeps the downstream nonlinearity from biasing.
        float value = sample - this->ym2612AnalogState.hpLastInput[channel] + (cfg.hpAlpha * this->ym2612AnalogState.hpLastOutput[channel]);
        this->ym2612AnalogState.hpLastInput[channel] = sample;
        this->ym2612AnalogState.hpLastOutput[channel] = value;

        // YM2612 character is approximated with a tiny asymmetric curve.
        if (value >= 0.0f) {
            value = (value * cfg.asymPosGain) + (cfg.asymPosCurve * value * value);
        } else {
            value = (value * cfg.asymNegGain) - (cfg.asymNegCurve * value * value);
        }
        value = clampUnit(value);

        // A one-pole low-pass lightly rounds the sharpest digital edge.
        float& lp = this->ym2612AnalogState.lpLastOutput[channel];
        lp += cfg.lpAlpha * (value - lp);
        value = lp;

        if (cfg.busSaturationEnabled) {
            float driven = value * cfg.saturatorDrive;
            value = driven / (1.0f + ((cfg.saturatorDrive - 1.0f) * std::fabs(driven)));
        }

        value *= cfg.outputGain;
        return clampUnit(value);
    }

    int16_t postProcessSample(size_t channel, int32_t sample)
    {
        float value = static_cast<float>(sample) * this->postAmp;
        if (this->ym2612AnalogConfig.enabled) {
            value = this->applyYm2612AnalogPipeline(channel, value / 32768.0f) * 32768.0f;
        } else {
            value = this->applyLegacyDcCut(channel, value);
        }
        if (value < -32768.0f) {
            value = -32768.0f;
        } else if (value > 32767.0f) {
            value = 32767.0f;
        }
        return static_cast<int16_t>(std::lround(value));
    }

    void updateYm2612Frequency(uint32_t reg, uint8_t data)
    {
        const int port = (reg >> 8) & 1;
        const uint8_t addr = reg & 0xff;

        if (0xa0 <= addr && addr <= 0xa2) {
            const int ch = port * 3 + (addr - 0xa0);
            this->ym2612_frequency_low[ch] = data;
            return;
        }
        if (0xa4 <= addr && addr <= 0xa6) {
            const int ch = port * 3 + (addr - 0xa4);
            this->ym2612_frequency_high[ch] = data;
        }
    }

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
