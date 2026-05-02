#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "../src/vgmdrv.hpp"

extern "C" {
#include "Nuked-OPN2/ym3438.h"
#include "YMF276-LLE/fmopn2.h"
}

static constexpr int OutputSampleRate = 44100;
static constexpr int OutputChannels = 2;
static constexpr int16_t Int16Min = std::numeric_limits<int16_t>::min();
static constexpr int16_t Int16Max = std::numeric_limits<int16_t>::max();

static uint16_t readLe16(const std::vector<uint8_t>& data, size_t offset)
{
    if (offset + 2 > data.size()) {
        throw std::runtime_error("unexpected end of VGM header");
    }
    return static_cast<uint16_t>(data[offset] | (data[offset + 1] << 8));
}

static uint32_t readLe32(const std::vector<uint8_t>& data, size_t offset)
{
    if (offset + 4 > data.size()) {
        throw std::runtime_error("unexpected end of VGM header");
    }
    return static_cast<uint32_t>(data[offset])
         | (static_cast<uint32_t>(data[offset + 1]) << 8)
         | (static_cast<uint32_t>(data[offset + 2]) << 16)
         | (static_cast<uint32_t>(data[offset + 3]) << 24);
}

static int16_t clampInt16(int32_t value)
{
    if (value < Int16Min) {
        return Int16Min;
    }
    if (value > Int16Max) {
        return Int16Max;
    }
    return static_cast<int16_t>(value);
}

static std::vector<uint8_t> loadFile(const std::string& path)
{
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) {
        throw std::runtime_error("failed to open input file: " + path);
    }
    ifs.seekg(0, std::ios::end);
    const std::streamoff size = ifs.tellg();
    if (size < 0) {
        throw std::runtime_error("failed to get input file size: " + path);
    }
    ifs.seekg(0);
    std::vector<uint8_t> data(static_cast<size_t>(size));
    if (!data.empty()) {
        ifs.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(data.size()));
    }
    if (!ifs) {
        throw std::runtime_error("failed to read input file: " + path);
    }
    return data;
}

static void writeLe16(std::ofstream& ofs, uint16_t value)
{
    const char bytes[2] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
    };
    ofs.write(bytes, sizeof(bytes));
}

static void writeLe32(std::ofstream& ofs, uint32_t value)
{
    const char bytes[4] = {
        static_cast<char>(value & 0xff),
        static_cast<char>((value >> 8) & 0xff),
        static_cast<char>((value >> 16) & 0xff),
        static_cast<char>((value >> 24) & 0xff),
    };
    ofs.write(bytes, sizeof(bytes));
}

static void writeWav(const std::string& path, const std::vector<int16_t>& pcm)
{
    std::ofstream ofs(path, std::ios::binary);
    if (!ofs) {
        throw std::runtime_error("failed to open output file: " + path);
    }

    const uint16_t blockAlign = OutputChannels * sizeof(int16_t);
    const uint32_t dataBytes = static_cast<uint32_t>(pcm.size() * sizeof(int16_t));
    const uint32_t riffBytes = 36 + dataBytes;

    ofs.write("RIFF", 4);
    writeLe32(ofs, riffBytes);
    ofs.write("WAVE", 4);
    ofs.write("fmt ", 4);
    writeLe32(ofs, 16);
    writeLe16(ofs, 1);
    writeLe16(ofs, OutputChannels);
    writeLe32(ofs, OutputSampleRate);
    writeLe32(ofs, OutputSampleRate * blockAlign);
    writeLe16(ofs, blockAlign);
    writeLe16(ofs, 16);
    ofs.write("data", 4);
    writeLe32(ofs, dataBytes);
    if (!pcm.empty()) {
        ofs.write(reinterpret_cast<const char*>(pcm.data()), static_cast<std::streamsize>(dataBytes));
    }
    if (!ofs) {
        throw std::runtime_error("failed to write output file: " + path);
    }
}

struct VgmInfo {
    uint32_t version;
    uint32_t ym2612Clock;
    size_t dataOffset;
    size_t loopOffset;
};

static VgmInfo parseVgmInfo(const std::vector<uint8_t>& data)
{
    if (data.size() < 0x40 || std::memcmp(data.data(), "Vgm ", 4) != 0) {
        throw std::runtime_error("input is not a VGM file");
    }

    VgmInfo info;
    info.version = readLe32(data, 0x08);
    if (info.version < 0x150) {
        throw std::runtime_error("VGM version 1.50 or later is required");
    }

    info.ym2612Clock = readLe32(data, 0x2c) & 0x3fffffff;
    if (info.ym2612Clock == 0) {
        throw std::runtime_error("YM2612 clock is not set");
    }

    const size_t chipOffsets[] = {
        0x30, 0x38, 0x40, 0x44, 0x48, 0x4c, 0x50, 0x54, 0x58, 0x5c,
        0x60, 0x64, 0x68, 0x6c, 0x70, 0x74, 0x80, 0x84, 0x88, 0x8c,
        0x90, 0x98, 0x9c, 0xa0, 0xa4, 0xa8, 0xac, 0xb0, 0xb4, 0xb8,
        0xc0, 0xc4, 0xc8, 0xcc, 0xd0, 0xd8, 0xdc, 0xe0, 0xe4,
    };
    for (size_t offset : chipOffsets) {
        if (offset + 4 > data.size()) {
            continue;
        }
        if (readLe32(data, offset) != 0) {
            throw std::runtime_error("only YM2612-only VGM files are supported");
        }
    }

    const uint32_t dataOffset = readLe32(data, 0x34);
    info.dataOffset = dataOffset == 0 ? 0x40 : static_cast<size_t>(0x34 + dataOffset);
    if (info.dataOffset >= data.size()) {
        throw std::runtime_error("VGM data offset is outside the file");
    }

    const uint32_t loopOffset = readLe32(data, 0x1c);
    info.loopOffset = loopOffset == 0 ? 0 : static_cast<size_t>(0x1c + loopOffset);
    if (info.loopOffset != 0 && info.loopOffset >= data.size()) {
        throw std::runtime_error("VGM loop offset is outside the file");
    }
    return info;
}

class VgmCommandStream {
  public:
    explicit VgmCommandStream(const std::vector<uint8_t>& data) : data(data), info(parseVgmInfo(data)), cursor(info.dataOffset)
    {
    }

    const VgmInfo& getInfo() const
    {
        return info;
    }

    template <typename Callback>
    bool runUntilWait(Callback write)
    {
        while (waitSamples == 0 && !ended) {
            require(1);
            const uint8_t cmd = data[cursor++];
            switch (cmd) {
                case 0x52:
                case 0x53: {
                    require(2);
                    const uint8_t reg = data[cursor++];
                    const uint8_t value = data[cursor++];
                    if (reg == 0x2a || reg == 0x2b) {
                        break;
                    }
                    write(cmd == 0x53 ? 1 : 0, reg, value);
                    break;
                }
                case 0x61:
                    require(2);
                    waitSamples = readLe16(data, cursor);
                    cursor += 2;
                    break;
                case 0x62:
                    waitSamples = 735;
                    break;
                case 0x63:
                    waitSamples = 882;
                    break;
                case 0x66:
                    if (info.loopOffset != 0) {
                        cursor = info.loopOffset;
                        loopCount++;
                    } else {
                        ended = true;
                    }
                    break;
                case 0x67: {
                    require(6);
                    if (data[cursor++] != 0x66) {
                        throw std::runtime_error("invalid VGM data block");
                    }
                    cursor++;
                    const uint32_t size = readLe32(data, cursor);
                    cursor += 4;
                    require(size);
                    cursor += size;
                    break;
                }
                case 0x4f:
                case 0x50:
                    throw std::runtime_error("PSG commands are not supported by this test command");
                default:
                    if (0x70 <= cmd && cmd <= 0x7f) {
                        waitSamples = (cmd & 0x0f) + 1;
                    } else if (0x80 <= cmd && cmd <= 0x8f) {
                        waitSamples = cmd & 0x0f;
                    } else if (cmd == 0x90 || cmd == 0x91 || cmd == 0x95) {
                        require(4);
                        cursor += 4;
                    } else if (cmd == 0x92) {
                        require(5);
                        cursor += 5;
                    } else if (cmd == 0x93) {
                        require(10);
                        cursor += 10;
                    } else if (cmd == 0x94) {
                        require(1);
                        cursor++;
                    } else {
                        char msg[64];
                        std::snprintf(msg, sizeof(msg), "unsupported VGM command: 0x%02X", cmd);
                        throw std::runtime_error(msg);
                    }
                    break;
            }
        }

        if (waitSamples > 0) {
            waitSamples--;
        }
        return !shouldStop();
    }

    bool shouldStop() const
    {
        return ended || loopCount > 0;
    }

  private:
    const std::vector<uint8_t>& data;
    VgmInfo info;
    size_t cursor;
    uint32_t waitSamples = 0;
    uint32_t loopCount = 0;
    bool ended = false;

    void require(size_t size) const
    {
        if (cursor + size > data.size()) {
            throw std::runtime_error("unexpected end of VGM data");
        }
    }
};

class NativeResampler {
  public:
    explicit NativeResampler(double nativeSampleRate) : nativeStep(nativeSampleRate / OutputSampleRate)
    {
    }

    template <typename Generator>
    std::array<int32_t, 2> sample(Generator generate)
    {
        if (!primed) {
            previous = generate();
            next = generate();
            primed = true;
        }
        phase += nativeStep;
        while (phase >= 1.0) {
            phase -= 1.0;
            previous = next;
            next = generate();
        }

        std::array<int32_t, 2> out;
        for (size_t i = 0; i < out.size(); i++) {
            out[i] = static_cast<int32_t>(std::lround(previous[i] + ((next[i] - previous[i]) * phase)));
        }
        return out;
    }

  private:
    double nativeStep;
    double phase = 0.0;
    bool primed = false;
    std::array<int32_t, 2> previous = {0, 0};
    std::array<int32_t, 2> next = {0, 0};
};

class NukedOpn2 {
  public:
    explicit NukedOpn2(uint32_t clock) : resampler(static_cast<double>(clock) / (6.0 * 24.0))
    {
        std::memset(&chip, 0, sizeof(chip));
        OPN2_SetChipType(ym3438_mode_ym2612);
        OPN2_Reset(&chip);
    }

    void write(uint8_t port, uint8_t reg, uint8_t value)
    {
        Bit16s pins[2] = {0, 0};
        OPN2_Write(&chip, port ? 2 : 0, reg);
        OPN2_Clock(&chip, pins);
        OPN2_Write(&chip, port ? 3 : 1, value);
        OPN2_Clock(&chip, pins);
        OPN2_Clock(&chip, pins);
    }

    std::array<int32_t, 2> sample()
    {
        return resampler.sample([this]() { return nativeSample(); });
    }

  private:
    ym3438_t chip;
    NativeResampler resampler;

    std::array<int32_t, 2> nativeSample()
    {
        int32_t left = 0;
        int32_t right = 0;
        for (int i = 0; i < 24; i++) {
            Bit16s pins[2] = {0, 0};
            OPN2_Clock(&chip, pins);
            left += pins[0] * 128;
            right += pins[1] * 128;
        }
        return {left / 24, right / 24};
    }
};

class LleOpn2 {
  public:
    explicit LleOpn2(uint32_t clock) : resampler(static_cast<double>(clock) / (24.0 * 24.0))
    {
        std::memset(&chip, 0, sizeof(chip));
        reset();
    }

    void write(uint8_t port, uint8_t reg, uint8_t value)
    {
        writeBus(port ? 2 : 0, reg);
        writeBus(port ? 3 : 1, value);
    }

    std::array<int32_t, 2> sample()
    {
        return resampler.sample([this]() { return nativeSample(); });
    }

  private:
    fmopn2_t chip;
    NativeResampler resampler;
    int phi = 0;

    void clock()
    {
        phi ^= 1;
        FMOPN2_Clock(&chip, phi);
    }

    void clockCycles(int cycles)
    {
        for (int i = 0; i < cycles; i++) {
            clock();
        }
    }

    void idle(int cycles)
    {
        chip.input.cs = 0;
        chip.input.wr = 0;
        chip.input.rd = 0;
        clockCycles(cycles);
    }

    void reset()
    {
        chip.input.ic = 1;
        idle(1024);
        chip.input.ic = 0;
        idle(1024);
    }

    void writeBus(uint8_t address, uint8_t value)
    {
        chip.input.address = address;
        chip.input.data = value;
        chip.input.cs = 1;
        chip.input.wr = 1;
        chip.input.rd = 0;
        clockCycles(24);
        chip.input.cs = 0;
        chip.input.wr = 0;
        idle(96);
    }

    std::array<int32_t, 2> nativeSample()
    {
        int32_t left = 0;
        int32_t right = 0;
        for (int i = 0; i < 144; i++) {
            clock();
            left += signExtend18(chip.ch_accm_l[1]) / 16;
            right += signExtend18(chip.ch_accm_r[1]) / 16;
        }
        return {left / 144, right / 144};
    }

    static int32_t signExtend18(int value)
    {
        value &= 0x3ffff;
        if (value & 0x20000) {
            value |= ~0x3ffff;
        }
        return value;
    }
};

class YmfmInterface : public ymfm::ymfm_interface {
};

static std::vector<int16_t> renderVgs(const std::vector<uint8_t>& data)
{
    VgmDriver driver(OutputSampleRate, OutputChannels);
    if (!driver.load(data.data(), data.size())) {
        throw std::runtime_error("failed to load VGM with VGS-X VgmDriver");
    }

    std::vector<int16_t> pcm;
    std::array<int16_t, OutputChannels> frame;
    while (!driver.isEnded() && driver.getLoopCount() == 0) {
        driver.render(frame.data(), OutputChannels);
        if (driver.isEnded() || driver.getLoopCount() != 0) {
            break;
        }
        pcm.push_back(frame[0]);
        pcm.push_back(frame[1]);
    }
    return pcm;
}

template <typename Chip>
static std::vector<int16_t> renderExternal(const std::vector<uint8_t>& data)
{
    VgmCommandStream stream(data);
    Chip chip(stream.getInfo().ym2612Clock);
    std::vector<int16_t> pcm;

    while (stream.runUntilWait([&chip](uint8_t port, uint8_t reg, uint8_t value) {
        chip.write(port, reg, value);
    })) {
        const auto sample = chip.sample();
        pcm.push_back(clampInt16(sample[0]));
        pcm.push_back(clampInt16(sample[1]));
    }
    return pcm;
}

static std::string outputPath(const std::string& input, const char* suffix)
{
    const size_t slash = input.find_last_of("/\\");
    const size_t dot = input.find_last_of('.');
    if (dot != std::string::npos && (slash == std::string::npos || slash < dot)) {
        return input.substr(0, dot) + suffix;
    }
    return input + suffix;
}

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::puts("usage: test file.vgm");
        return 1;
    }

    try {
        const std::string input = argv[1];
        const std::vector<uint8_t> data = loadFile(input);
        parseVgmInfo(data);

        const std::string vgsPath = outputPath(input, "_vgs.wav");
        const std::string nukedPath = outputPath(input, "_nuked.wav");
        const std::string llePath = outputPath(input, "_lle.wav");

        writeWav(vgsPath, renderVgs(data));
        writeWav(nukedPath, renderExternal<NukedOpn2>(data));
        writeWav(llePath, renderExternal<LleOpn2>(data));

        std::printf("wrote %s\n", vgsPath.c_str());
        std::printf("wrote %s\n", nukedPath.c_str());
        std::printf("wrote %s\n", llePath.c_str());
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    return 0;
}
