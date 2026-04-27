
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
    std::array<bool, YM2612ChannelCount> ym2612_mute;
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

    struct Ym2612ResampleState {
        bool primed;
        emulated_time previousTime;
        emulated_time nextTime;
        std::array<int32_t, OutputChannelCount> previousSample;
        std::array<int32_t, OutputChannelCount> nextSample;

        void reset()
        {
            this->primed = false;
            this->previousTime = 0;
            this->nextTime = 0;
            this->previousSample.fill(0);
            this->nextSample.fill(0);
        }
    } ym2612ResampleState;

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
        float postLpAlpha;
        bool notchEnabled;
        float notchFrequencyHz;
        float notchQ;
        float notchMix;
        bool busSaturationEnabled;
        float saturatorDrive;
        float outputGain;
    };

  private:
    struct Ym2612AnalogNotchCoefficients {
        float b0;
        float b1;
        float b2;
        float a1;
        float a2;
    };

    struct Ym2612AnalogState {
        std::array<float, OutputChannelCount> hpLastInput;
        std::array<float, OutputChannelCount> hpLastOutput;
        std::array<float, OutputChannelCount> lpLastOutput;
        std::array<float, OutputChannelCount> postLpLastOutput;
        std::array<float, OutputChannelCount> notchInput1;
        std::array<float, OutputChannelCount> notchInput2;
        std::array<float, OutputChannelCount> notchOutput1;
        std::array<float, OutputChannelCount> notchOutput2;

        void reset()
        {
            this->hpLastInput.fill(0.0f);
            this->hpLastOutput.fill(0.0f);
            this->lpLastOutput.fill(0.0f);
            this->postLpLastOutput.fill(0.0f);
            this->notchInput1.fill(0.0f);
            this->notchInput2.fill(0.0f);
            this->notchOutput1.fill(0.0f);
            this->notchOutput2.fill(0.0f);
        }
    };

    Ym2612AnalogConfig ym2612AnalogConfig;
    Ym2612AnalogNotchCoefficients ym2612AnalogNotch;
    Ym2612AnalogState ym2612AnalogState;
    float outputSampleRate;

  public:
    VgmDriver(int samples, int channels) : ym2612(*this)
    {
        this->output_step = 0x100000000ull / samples;
        this->channels = channels;
        this->outputSampleRate = static_cast<float>(samples);
        this->postAmp = 1.55f;
        this->dcCutEnabled = true;
        this->dcCutAlpha = 0.995f;
        this->ym2612AnalogConfig = this->makeYm2612AnalogRealPreset();
        this->updateYm2612AnalogNotchCoefficients();
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
            1.0f,
            false,
            4900.0f,
            2.5f,
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
            1.0f,
            false,
            4900.0f,
            2.5f,
            0.0f,
            true,
            1.05f,
            0.985f,
        };
    }

    static Ym2612AnalogConfig makeYm2612AnalogRealPreset()
    {
        return {
            true,
            0.9971f,
            0.62f,
            1.000f,
            0.996f,
            0.006f,
            0.009f,
            0.94f,
            true,
            4900.0f,
            9.0f,
            0.020f,
            true,
            1.006f,
            0.940f,
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
            1.0f,
            false,
            4900.0f,
            2.5f,
            0.0f,
            true,
            1.10f,
            0.965f,
        };
    }

    void setYm2612AnalogConfig(const Ym2612AnalogConfig& config)
    {
        this->ym2612AnalogConfig = config;
        this->clampYm2612AnalogConfig();
        this->updateYm2612AnalogNotchCoefficients();
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

    void useYm2612AnalogRealPreset()
    {
        this->setYm2612AnalogConfig(this->makeYm2612AnalogRealPreset());
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
        this->ym2612_mute.fill(false);
        this->dcCutLastInput.fill(0.0f);
        this->dcCutLastOutput.fill(0.0f);
        this->ym2612AnalogState.reset();
        this->ym2612ResampleState.reset();
        for (int ch = 0; ch < YM2612ChannelCount; ch++) {
            this->ym2612.set_channel_mute(static_cast<uint32_t>(ch), false);
        }
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

                        this->sampleYm2612At(vgm.output_start, mixed);
                        vgm.output_start += output_step;
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
        if (this->ym2612_mute[ch]) {
            return 0;
        }
        return this->ym2612.channel_volume(static_cast<uint32_t>(ch));
    }

    bool getMute(ChipType type, int ch)
    {
        if (type != ChipType::YM2612 || ch < 0 || YM2612ChannelCount <= ch) {
            return false;
        }
        return this->ym2612_mute[ch];
    }

    void setMute(ChipType type, int ch, bool enabled)
    {
        if (type != ChipType::YM2612 || ch < 0 || YM2612ChannelCount <= ch) {
            return;
        }
        this->ym2612_mute[ch] = enabled;
        this->ym2612.set_channel_mute(static_cast<uint32_t>(ch), enabled);
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
        this->ym2612AnalogConfig.postLpAlpha = std::clamp(this->ym2612AnalogConfig.postLpAlpha, 0.0f, 1.0f);
        this->ym2612AnalogConfig.notchFrequencyHz = std::clamp(this->ym2612AnalogConfig.notchFrequencyHz, 1000.0f, 12000.0f);
        this->ym2612AnalogConfig.notchQ = std::clamp(this->ym2612AnalogConfig.notchQ, 0.2f, 12.0f);
        this->ym2612AnalogConfig.notchMix = std::clamp(this->ym2612AnalogConfig.notchMix, 0.0f, 1.0f);
        this->ym2612AnalogConfig.saturatorDrive = std::clamp(this->ym2612AnalogConfig.saturatorDrive, 1.0f, 2.0f);
        this->ym2612AnalogConfig.outputGain = std::clamp(this->ym2612AnalogConfig.outputGain, 0.0f, 2.0f);
    }

    void updateYm2612AnalogNotchCoefficients()
    {
        const float sampleRate = std::max(this->outputSampleRate, 1.0f);
        const float omega = 2.0f * static_cast<float>(M_PI) * this->ym2612AnalogConfig.notchFrequencyHz / sampleRate;
        const float alpha = std::sin(omega) / (2.0f * this->ym2612AnalogConfig.notchQ);
        const float cosOmega = std::cos(omega);
        const float a0 = 1.0f + alpha;

        this->ym2612AnalogNotch.b0 = 1.0f / a0;
        this->ym2612AnalogNotch.b1 = (-2.0f * cosOmega) / a0;
        this->ym2612AnalogNotch.b2 = 1.0f / a0;
        this->ym2612AnalogNotch.a1 = (-2.0f * cosOmega) / a0;
        this->ym2612AnalogNotch.a2 = (1.0f - alpha) / a0;
    }

    float applyYm2612AnalogNotch(size_t channel, float value)
    {
        if (!this->ym2612AnalogConfig.notchEnabled || this->ym2612AnalogConfig.notchMix <= 0.0f) {
            return value;
        }

        const Ym2612AnalogNotchCoefficients& notch = this->ym2612AnalogNotch;
        float filtered = (notch.b0 * value)
                       + (notch.b1 * this->ym2612AnalogState.notchInput1[channel])
                       + (notch.b2 * this->ym2612AnalogState.notchInput2[channel])
                       - (notch.a1 * this->ym2612AnalogState.notchOutput1[channel])
                       - (notch.a2 * this->ym2612AnalogState.notchOutput2[channel]);

        this->ym2612AnalogState.notchInput2[channel] = this->ym2612AnalogState.notchInput1[channel];
        this->ym2612AnalogState.notchInput1[channel] = value;
        this->ym2612AnalogState.notchOutput2[channel] = this->ym2612AnalogState.notchOutput1[channel];
        this->ym2612AnalogState.notchOutput1[channel] = filtered;

        return (value * (1.0f - this->ym2612AnalogConfig.notchMix)) + (filtered * this->ym2612AnalogConfig.notchMix);
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

        float& postLp = this->ym2612AnalogState.postLpLastOutput[channel];
        postLp += cfg.postLpAlpha * (value - postLp);
        value = postLp;

        value = this->applyYm2612AnalogNotch(channel, value);

        value *= cfg.outputGain;
        return clampUnit(value);
    }

    void generateYm2612Output(std::array<int32_t, OutputChannelCount>& sample)
    {
        ymfm::ym2612::output_data out;
        ym2612.generate(&out);
        sample[0] = out.data[0];
        sample[1] = out.data[1];
    }

    void primeYm2612Resampler()
    {
        this->generateYm2612Output(this->ym2612ResampleState.previousSample);
        this->ym2612ResampleState.previousTime = 0;
        vgm.pos = vgm.step;
        this->generateYm2612Output(this->ym2612ResampleState.nextSample);
        this->ym2612ResampleState.nextTime = vgm.pos;
        vgm.pos += vgm.step;
        this->ym2612ResampleState.primed = true;
    }

    void advanceYm2612Resampler(emulated_time targetTime)
    {
        if (!this->ym2612ResampleState.primed) {
            this->primeYm2612Resampler();
        }
        while (this->ym2612ResampleState.nextTime <= targetTime) {
            this->ym2612ResampleState.previousTime = this->ym2612ResampleState.nextTime;
            this->ym2612ResampleState.previousSample = this->ym2612ResampleState.nextSample;
            this->generateYm2612Output(this->ym2612ResampleState.nextSample);
            this->ym2612ResampleState.nextTime = vgm.pos;
            vgm.pos += vgm.step;
        }
    }

    void sampleYm2612At(emulated_time targetTime, int32_t mixed[OutputChannelCount])
    {
        this->advanceYm2612Resampler(targetTime);

        const emulated_time interval = this->ym2612ResampleState.nextTime - this->ym2612ResampleState.previousTime;
        double fraction = 0.0;
        if (interval > 0) {
            fraction = static_cast<double>(targetTime - this->ym2612ResampleState.previousTime) / static_cast<double>(interval);
        }
        fraction = std::clamp(fraction, 0.0, 1.0);

        for (size_t channel = 0; channel < OutputChannelCount; channel++) {
            const double previous = static_cast<double>(this->ym2612ResampleState.previousSample[channel]);
            const double next = static_cast<double>(this->ym2612ResampleState.nextSample[channel]);
            mixed[channel] += static_cast<int32_t>(std::lround(previous + ((next - previous) * fraction)));
        }
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
