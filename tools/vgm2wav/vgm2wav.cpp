#include <array>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <limits>
#include <stdexcept>
#include <string>
#include <vector>

#include "vgmdrv.hpp"

static constexpr int OutputSampleRate = 44100;
static constexpr int OutputChannels = 2;
static constexpr int FadeSeconds = 3;
static constexpr int FadeSamples = OutputSampleRate * FadeSeconds;

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

static int16_t applyGain(int16_t sample, double gain)
{
    const double value = static_cast<double>(sample) * gain;
    if (value < std::numeric_limits<int16_t>::min()) {
        return std::numeric_limits<int16_t>::min();
    }
    if (value > std::numeric_limits<int16_t>::max()) {
        return std::numeric_limits<int16_t>::max();
    }
    return static_cast<int16_t>(value);
}

static std::string defaultOutputPath(const std::string& input)
{
    const size_t slash = input.find_last_of("/\\");
    const size_t dot = input.find_last_of('.');
    if (dot != std::string::npos && (slash == std::string::npos || slash < dot)) {
        return input.substr(0, dot) + ".wav";
    }
    return input + ".wav";
}

struct Options {
    std::string inputPath;
    std::string outputPath;
    uint32_t loopCount = 1;
};

static void printUsage()
{
    std::puts("usage: vgm2wav /path/to/file.vgm [-l loop_count] [-o file.wav]");
}

static uint32_t parseLoopCount(const char* value)
{
    char* end = nullptr;
    const unsigned long parsed = std::strtoul(value, &end, 10);
    if (!value[0] || (end && *end) || parsed < 1 || parsed > std::numeric_limits<uint32_t>::max()) {
        throw std::runtime_error("-l requires an integer greater than or equal to 1");
    }
    return static_cast<uint32_t>(parsed);
}

static Options parseOptions(int argc, char** argv)
{
    Options options;
    for (int i = 1; i < argc; i++) {
        if (std::strcmp(argv[i], "-l") == 0) {
            if (++i >= argc) {
                throw std::runtime_error("-l requires a value");
            }
            options.loopCount = parseLoopCount(argv[i]);
        } else if (std::strcmp(argv[i], "-o") == 0) {
            if (++i >= argc) {
                throw std::runtime_error("-o requires a value");
            }
            options.outputPath = argv[i];
        } else if (!argv[i][0] || argv[i][0] == '-') {
            throw std::runtime_error("unknown option: " + std::string(argv[i]));
        } else if (options.inputPath.empty()) {
            options.inputPath = argv[i];
        } else {
            throw std::runtime_error("multiple input files specified");
        }
    }

    if (options.inputPath.empty()) {
        throw std::runtime_error("input VGM file is required");
    }
    if (options.outputPath.empty()) {
        options.outputPath = defaultOutputPath(options.inputPath);
    }
    return options;
}

static std::vector<int16_t> renderVgm(const std::vector<uint8_t>& data, uint32_t loopCount)
{
    VgmDriver driver(OutputSampleRate, OutputChannels);
    if (!driver.load(data.data(), data.size())) {
        throw std::runtime_error("failed to load VGM");
    }

    std::vector<int16_t> pcm;
    std::array<int16_t, OutputChannels> frame;
    bool fading = false;
    int fadeCursor = 0;

    while (!driver.isEnded()) {
        driver.render(frame.data(), OutputChannels);

        if (!fading && driver.getLoopCount() >= loopCount) {
            fading = true;
        }

        if (fading) {
            if (fadeCursor >= FadeSamples) {
                break;
            }
            const double gain = 1.0 - (static_cast<double>(fadeCursor) / static_cast<double>(FadeSamples));
            frame[0] = applyGain(frame[0], gain);
            frame[1] = applyGain(frame[1], gain);
            fadeCursor++;
        }

        pcm.push_back(frame[0]);
        pcm.push_back(frame[1]);
    }
    return pcm;
}

int main(int argc, char** argv)
{
    try {
        if (argc < 2) {
            printUsage();
            return 1;
        }

        const Options options = parseOptions(argc, argv);
        const std::vector<uint8_t> data = loadFile(options.inputPath);
        const std::vector<int16_t> pcm = renderVgm(data, options.loopCount);
        writeWav(options.outputPath, pcm);
        std::printf("wrote %s\n", options.outputPath.c_str());
    } catch (const std::exception& e) {
        std::fprintf(stderr, "error: %s\n", e.what());
        return 1;
    }
    return 0;
}
