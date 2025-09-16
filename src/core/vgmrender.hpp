//
// Simple vgm renderer.
//
// Leverages em_inflate tiny inflater from https://github.com/emmanuel-marty/em_inflate
//
// Compile with:
//
//   g++ --std=c++14 -I../../src vgmrender.cpp em_inflate.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o vgmrender.exe
//
// or:
//
//   clang++ --std=c++14 -I../../src vgmrender.cpp em_inflate.cpp ../../src/ymfm_misc.cpp ../../src/ymfm_opl.cpp ../../src/ymfm_opm.cpp ../../src/ymfm_opn.cpp ../../src/ymfm_adpcm.cpp ../../src/ymfm_pcm.cpp ../../src/ymfm_ssg.cpp -o vgmrender.exe
//
// or:
//
//   cl -I..\..\src vgmrender.cpp em_inflate.cpp ..\..\src\ymfm_misc.cpp ..\..\src\ymfm_opl.cpp ..\..\src\ymfm_opm.cpp ..\..\src\ymfm_opn.cpp ..\..\src\ymfm_adpcm.cpp ..\..\src\ymfm_pcm.cpp ..\..\src\ymfm_ssg.cpp /Od /Zi /std:c++14 /EHsc
//

#pragma once
#define _CRT_SECURE_NO_WARNINGS

#include <cmath>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <list>
#include <string>

// #include "em_inflate.h"
#include "ymfm_misc.h"
#include "ymfm_opm.h"
#include "ymfm_opn.h"

#define LOG_WRITES (0)

// run this many dummy clocks of each chip before generating
#define EXTRA_CLOCKS (0)

//*********************************************************
//  GLOBAL TYPES
//*********************************************************

// we use an int64_t as emulated time, as a 32.32 fixed point value
using emulated_time = int64_t;

// enumeration of the different types of chips we support
enum chip_type {
    CHIP_YM2149,
    CHIP_YM2151,
    CHIP_YM2203,
    CHIP_YM2608,
    CHIP_YM2610,
    CHIP_YM2612,
    CHIP_TYPES
};

//*********************************************************
//  CLASSES
//*********************************************************

// ======================> vgm_chip_base

// abstract base class for a Yamaha chip; we keep a list of these for processing
// as new commands come in
class vgm_chip_base
{
  public:
    // construction
    vgm_chip_base(uint32_t clock, chip_type type, char const* name) : m_type(type),
                                                                      m_name(name)
    {
    }

    // destruction
    virtual ~vgm_chip_base()
    {
    }

    // simple getters
    chip_type type() const { return m_type; }
    virtual uint32_t sample_rate() const = 0;

    // required methods for derived classes to implement
    virtual void write(uint32_t reg, uint8_t data) = 0;
    virtual void generate(int32_t* left, int32_t* right) = 0;

    // write data to the ADPCM-A buffer
    void write_data(ymfm::access_class type, uint32_t base, uint32_t length, uint8_t const* src)
    {
        uint32_t end = base + length;
        if (end > m_data[type].size())
            m_data[type].resize(end);
        memcpy(&m_data[type][base], src, length);
    }

    // seek within the PCM stream
    void seek_pcm(uint32_t pos) { m_pcm_offset = pos; }
    uint8_t read_pcm()
    {
        auto& pcm = m_data[ymfm::ACCESS_PCM];
        return (m_pcm_offset < pcm.size()) ? pcm[m_pcm_offset++] : 0;
    }

  protected:
    // internal state
    chip_type m_type;
    std::string m_name;
    std::vector<uint8_t> m_data[ymfm::ACCESS_CLASSES];
    uint32_t m_pcm_offset;
};

// ======================> vgm_chip

// actual chip-specific implementation class; includes implementatino of the
// ymfm_interface as needed for vgmplay purposes
template <typename ChipType>
class vgm_chip : public vgm_chip_base, public ymfm::ymfm_interface
{
  public:
    // construction
    vgm_chip(uint32_t clock, chip_type type, char const* name) : vgm_chip_base(clock, type, name),
                                                                 m_chip(*this),
                                                                 m_clock(clock),
                                                                 m_clocks(0),
                                                                 m_step(0x100000000ull / m_chip.sample_rate(clock)),
                                                                 m_pos(0)
    {
        m_chip.reset();
        for (int clock = 0; clock < EXTRA_CLOCKS; clock++) {
            m_chip.generate(&m_output);
        }
    }

    virtual uint32_t sample_rate() const override
    {
        return m_chip.sample_rate(m_clock);
    }

    // handle a register write: just queue for now
    virtual void write(uint32_t reg, uint8_t data) override
    {
        m_queue.push_back(std::make_pair(reg, data));
    }

    // generate one output sample of output
    virtual void generate(int32_t* left, int32_t* right) override
    {
        uint32_t addr1 = 0xffff, addr2 = 0xffff;
        uint8_t data1 = 0, data2 = 0;

        // see if there is data to be written; if so, extract it and dequeue
        if (!m_queue.empty()) {
            auto front = m_queue.front();
            addr1 = 0 + 2 * ((front.first >> 8) & 3);
            data1 = front.first & 0xff;
            addr2 = addr1 + ((m_type == CHIP_YM2149) ? 2 : 1);
            data2 = front.second;
            m_queue.erase(m_queue.begin());
        }

        // write to the chip
        if (addr1 != 0xffff) {
            m_chip.write(addr1, data1);
            m_chip.write(addr2, data2);
        }

        // generate at the appropriate sample rate
        m_chip.generate(&m_output);

        // add the final result to the buffer
        if (m_type == CHIP_YM2203) {
            int32_t out0 = m_output.data[0];
            int32_t out1 = m_output.data[1 % ChipType::OUTPUTS];
            int32_t out2 = m_output.data[2 % ChipType::OUTPUTS];
            int32_t out3 = m_output.data[3 % ChipType::OUTPUTS];
            *left += out0 + out1 + out2 + out3;
            *right += out0 + out1 + out2 + out3;
        } else if (m_type == CHIP_YM2608 || m_type == CHIP_YM2610) {
            int32_t out0 = m_output.data[0];
            int32_t out1 = m_output.data[1 % ChipType::OUTPUTS];
            int32_t out2 = m_output.data[2 % ChipType::OUTPUTS];
            *left += out0 + out2;
            *right += out1 + out2;
        } else if (ChipType::OUTPUTS == 1) {
            *left += m_output.data[0];
            *right += m_output.data[0];
        } else {
            *left += m_output.data[0];
            *right += m_output.data[1 % ChipType::OUTPUTS];
        }
        m_clocks++;
    }

  protected:
    // handle a read from the buffer
    virtual uint8_t ymfm_external_read(ymfm::access_class type, uint32_t offset) override
    {
        auto& data = m_data[type];
        return (offset < data.size()) ? data[offset] : 0;
    }

    // internal state
    ChipType m_chip;
    uint32_t m_clock;
    uint64_t m_clocks;
    typename ChipType::output_data m_output;
    emulated_time m_step;
    emulated_time m_pos;
    std::vector<std::pair<uint32_t, uint8_t>> m_queue;
};

class VgmHelper
{
  private:
    int32_t wait;
    bool done;

  public:
    VgmHelper(const uint8_t* vgm, size_t vgmSize)
    {
        this->buffer = vgm;
        this->data_start = parse_header();
        this->offset = this->data_start;
        this->wait = 0;
        this->done = false;
        parse_header();
    }

    void render(int16_t* buf, int samples)
    {
        int16_t l, r;
        int cursor = 0;
        while (cursor < samples) {
            while (this->wait < 1) {
                this->execute();
            }
            this->wait--;
            generate(&l, &r);
            buf[cursor++] = l;
            buf[cursor++] = r;
        }
    }

    std::vector<std::unique_ptr<vgm_chip_base>> active_chips;
    const uint8_t* buffer;
    uint32_t offset;
    uint32_t data_start;
    uint32_t extra_header;
    uint32_t loop_offset;

    uint32_t parse_uint32()
    {
        uint32_t result = buffer[offset++];
        result |= buffer[offset++] << 8;
        result |= buffer[offset++] << 16;
        result |= buffer[offset++] << 24;
        return result;
    }

    uint32_t parse_uint32(uint32_t localoffset)
    {
        uint32_t result = buffer[localoffset++];
        result |= buffer[localoffset++] << 8;
        result |= buffer[localoffset++] << 16;
        result |= buffer[localoffset] << 24;
        return result;
    }

    template <typename ChipType>
    void add_chips(uint32_t clock, chip_type type, char const* chipname)
    {
        for (auto& chip : active_chips) {
            if (chip->type() == type) {
                // Ignore if already exist
                return;
            }
        }
        uint32_t clockval = clock & 0x3fffffff;
        int numchips = (clock & 0x40000000) ? 2 : 1;
        printf("ymfm: Adding %s%s @ %dHz\n", (numchips == 2) ? "2 x " : "", chipname, clockval);
        for (int index = 0; index < numchips; index++) {
            char name[100];
            snprintf(name, sizeof(name), "%s #%d", chipname, index);
            active_chips.push_back(std::make_unique<vgm_chip<ChipType>>(clockval, type, (numchips == 2) ? name : chipname));
        }
    }

    void add_rom_data(chip_type type, ymfm::access_class access, uint32_t localoffset, uint32_t size)
    {
        uint32_t length = parse_uint32(localoffset);
        localoffset += 4;
        uint32_t start = parse_uint32(localoffset);
        localoffset += 4;
        for (int index = 0; index < 2; index++) {
            vgm_chip_base* chip = find_chip(type, index);
            if (chip != nullptr)
                chip->write_data(access, start, size, &buffer[localoffset]);
        }
    }

    uint32_t parse_header()
    {
        // +00: already checked the ID
        this->offset = 4;
        this->loop_offset = 0;

        // +04: parse the size
        uint32_t size = parse_uint32();

        // +08: parse the version
        uint32_t version = parse_uint32();
        if (version > 0x171) {
            printf("VGM warning: version > 1.71 detected, some things may not work\n");
        }

        // +0C: SN76489 clock
        uint32_t clock = parse_uint32();
        if (clock != 0) {
            printf("VGM warning: clock for SN76489 specified (%d), but not supported\n", clock);
        }

        // +10: YM2413 clock
        clock = parse_uint32();
        if (clock != 0) {
            printf("VGM warning: clock for YM2413 specified (%d), but not supported\n", clock);
        }

        // +14: GD3 offset
        offset += 4;

        // +18: Total # samples
        offset += 4;

        // +1C: Loop offset
        loop_offset = parse_uint32();
        if (loop_offset) {
            loop_offset += offset - 4;
        }

        // +20: Loop # samples
        offset += 4;

        // +24: Rate
        offset += 4;

        // +28: SN76489 feedback / SN76489 shift register width / SN76489 Flags
        offset += 4;

        // +2C: YM2612 clock
        clock = parse_uint32();
        if (version >= 0x110 && clock != 0) {
            add_chips<ymfm::ym2612>(clock, CHIP_YM2612, "YM2612");
        }

        // +30: YM2151 clock
        clock = parse_uint32();
        if (version >= 0x110 && clock != 0) {
            add_chips<ymfm::ym2151>(clock, CHIP_YM2151, "YM2151");
        }

        // +34: VGM data offset
        data_start = parse_uint32();
        data_start += offset - 4;
        if (version < 0x150) {
            data_start = 0x40;
        }

        // +38: Sega PCM clock
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for Sega PCM specified, but not supported\n");
        }

        // +3C: Sega PCM interface register
        offset += 4;

        // +40: RF5C68 clock
        if (offset + 4 > data_start) {
            return data_start;
        }

        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for RF5C68 specified, but not supported\n");
        }

        // +44: YM2203 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            add_chips<ymfm::ym2203>(clock, CHIP_YM2203, "YM2203");
        }

        // +48: YM2608 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            add_chips<ymfm::ym2608>(clock, CHIP_YM2608, "YM2608");
        }

        // +4C: YM2610/2610B clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            if (clock & 0x80000000) {
                add_chips<ymfm::ym2610b>(clock, CHIP_YM2610, "YM2610B");
            } else {
                add_chips<ymfm::ym2610>(clock, CHIP_YM2610, "YM2610");
            }
        }

        // +50: YM3812 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for YM3812 specified, but not supported\n");
        }

        // +54: YM3526 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for YM3526 specified, but not supported\n");
        }

        // +58: Y8950 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for Y8950 specified, but not supported\n");
        }

        // +5C: YMF262 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for YMF262 specified, but not supported\n");
        }

        // +60: YMF278B clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for YMF278B specified, but not supported\n");
        }

        // +64: YMF271 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for YMF271 specified, but not supported\n");
        }

        // +68: YMF280B clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for YMF280B specified, but not supported\n");
        }

        // +6C: RF5C164 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for RF5C164 specified, but not supported\n");
        }

        // +70: PWM clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for PWM specified, but not supported\n");
        }

        // +74: AY8910 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x151 && clock != 0) {
            printf("VGM warning: clock for AY8910 specified, substituting YM2149\n");
            add_chips<ymfm::ym2149>(clock, CHIP_YM2149, "YM2149");
        }

        // +78: AY8910 flags
        if (offset + 4 > data_start) {
            return data_start;
        }
        offset += 4;

        // +7C: volume / loop info
        if (offset + 4 > data_start) {
            return data_start;
        }
        offset += 4;

        // +80: GameBoy DMG clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for GameBoy DMG specified, but not supported\n");
        }

        // +84: NES APU clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for NES APU specified, but not supported\n");
        }

        // +88: MultiPCM clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for MultiPCM specified, but not supported\n");
        }

        // +8C: uPD7759 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for uPD7759 specified, but not supported\n");
        }

        // +90: OKIM6258 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for OKIM6258 specified, but not supported\n");
        }

        // +94: OKIM6258 Flags / K054539 Flags / C140 Chip Type / reserved
        if (offset + 4 > data_start) {
            return data_start;
        }
        offset += 4;

        // +98: OKIM6295 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for OKIM6295 specified, but not supported\n");
        }

        // +9C: K051649 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for K051649 specified, but not supported\n");
        }

        // +A0: K054539 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for K054539 specified, but not supported\n");
        }

        // +A4: HuC6280 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for HuC6280 specified, but not supported\n");
        }

        // +A8: C140 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for C140 specified, but not supported\n");
        }

        // +AC: K053260 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for K053260 specified, but not supported\n");
        }

        // +B0: Pokey clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for Pokey specified, but not supported\n");
        }

        // +B4: QSound clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x161 && clock != 0) {
            printf("VGM warning: clock for QSound specified, but not supported\n");
        }

        // +B8: SCSP clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for SCSP specified, but not supported\n");
        }

        // +BC: extra header offset
        if (offset + 4 > data_start) {
            return data_start;
        }
        extra_header = parse_uint32();

        // +C0: WonderSwan clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for WonderSwan specified, but not supported\n");
        }

        // +C4: VSU clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for VSU specified, but not supported\n");
        }

        // +C8: SAA1099 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for SAA1099 specified, but not supported\n");
        }

        // +CC: ES5503 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for ES5503 specified, but not supported\n");
        }

        // +D0: ES5505/ES5506 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for ES5505/ES5506 specified, but not supported\n");
        }

        // +D4: ES5503 output channels / ES5505/ES5506 amount of output channels / C352 clock divider
        if (offset + 4 > data_start) {
            return data_start;
        }
        offset += 4;

        // +D8: X1-010 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for X1-010 specified, but not supported\n");
        }

        // +DC: C352 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for C352 specified, but not supported\n");
        }

        // +E0: GA20 clock
        if (offset + 4 > data_start) {
            return data_start;
        }
        clock = parse_uint32();
        if (version >= 0x171 && clock != 0) {
            printf("VGM warning: clock for GA20 specified, but not supported\n");
        }
        return data_start;
    }

    //-------------------------------------------------
    //  find_chip - find the given chip and index
    //-------------------------------------------------
    vgm_chip_base* find_chip(chip_type type, uint8_t index)
    {
        for (auto& chip : active_chips) {
            if (chip->type() == type && index-- == 0) {
                return chip.get();
            }
        }
        return nullptr;
    }

    //-------------------------------------------------
    //  write_chip - handle a write to the given chip
    //  and index
    //-------------------------------------------------
    void write_chip(chip_type type, uint8_t index, uint32_t reg, uint8_t data)
    {
        vgm_chip_base* chip = find_chip(type, index);
        if (chip != nullptr) {
            chip->write(reg, data);
        }
    }

    inline void generate(int16_t* left, int16_t* right)
    {
        bool more_remaining = false;
        int32_t outputs[2];
        outputs[0] = 0;
        outputs[1] = 0;
        for (auto& chip : active_chips) {
            chip->generate(&outputs[0], &outputs[1]);
        }
        for (int i = 0; i < 2; i++) {
            if (32767 < outputs[i]) {
                outputs[i] = 32767;
            } else if (outputs[i] < -32768) {
                outputs[i] = -32768;
            }
        }
        *left = (int16_t)outputs[0];
        *right = (int16_t)outputs[1];
    }

    inline void execute()
    {
        while (this->wait < 1 && !this->done) {
            uint8_t cmd = buffer[offset++];
            switch (cmd) {
                // YM2612 port 0, write value dd to register aa
                case 0x52:
                case 0xa2:
                    write_chip(CHIP_YM2612, cmd >> 7, buffer[offset], buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2612 port 1, write value dd to register aa
                case 0x53:
                case 0xa3:
                    write_chip(CHIP_YM2612, cmd >> 7, buffer[offset] | 0x100, buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2151, write value dd to register aa
                case 0x54:
                case 0xa4:
                    write_chip(CHIP_YM2151, cmd >> 7, buffer[offset], buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2203, write value dd to register aa
                case 0x55:
                case 0xa5:
                    write_chip(CHIP_YM2203, cmd >> 7, buffer[offset], buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2608 port 0, write value dd to register aa
                case 0x56:
                case 0xa6:
                    write_chip(CHIP_YM2608, cmd >> 7, buffer[offset], buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2608 port 1, write value dd to register aa
                case 0x57:
                case 0xa7:
                    write_chip(CHIP_YM2608, cmd >> 7, buffer[offset] | 0x100, buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2610 port 0, write value dd to register aa
                case 0x58:
                case 0xa8:
                    write_chip(CHIP_YM2610, cmd >> 7, buffer[offset], buffer[offset + 1]);
                    offset += 2;
                    break;

                // YM2610 port 1, write value dd to register aa
                case 0x59:
                case 0xa9:
                    write_chip(CHIP_YM2610, cmd >> 7, buffer[offset] | 0x100, buffer[offset + 1]);
                    offset += 2;
                    break;

                // Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds)
                case 0x61:
                    wait = buffer[offset] | (buffer[offset + 1] << 8);
                    offset += 2;
                    break;

                // wait 735 samples (60th of a second)
                case 0x62:
                    wait = 735;
                    break;

                // wait 882 samples (50th of a second)
                case 0x63:
                    wait = 882;
                    break;

                // end of sound data
                case 0x66:
                    if (loop_offset) {
                        offset = loop_offset;
                    } else {
                        done = true;
                    }
                    break;

                // data block
                case 0x67: {
                    uint8_t dummy = buffer[offset++];
                    if (dummy != 0x66)
                        break;
                    uint8_t type = buffer[offset++];
                    uint32_t size = parse_uint32();
                    uint32_t localoffset = offset;

                    switch (type) {
                        case 0x01: // RF5C68 PCM data for use with associated commands
                        case 0x02: // RF5C164 PCM data for use with associated commands
                        case 0x03: // PWM PCM data for use with associated commands
                        case 0x04: // OKIM6258 ADPCM data for use with associated commands
                        case 0x05: // HuC6280 PCM data for use with associated commands
                        case 0x06: // SCSP PCM data for use with associated commands
                        case 0x07: // NES APU DPCM data for use with associated commands
                            break;

                        case 0x00: // YM2612 PCM data for use with associated commands
                        {
                            vgm_chip_base* chip = find_chip(CHIP_YM2612, 0);
                            if (chip != nullptr)
                                chip->write_data(ymfm::ACCESS_PCM, 0, size - 8, &buffer[localoffset]);
                            break;
                        }

                        case 0x82: // YM2610 ADPCM ROM data
                            add_rom_data(CHIP_YM2610, ymfm::ACCESS_ADPCM_A, localoffset, size - 8);
                            break;

                        case 0x81: // YM2608 DELTA-T ROM data
                            add_rom_data(CHIP_YM2608, ymfm::ACCESS_ADPCM_B, localoffset, size - 8);
                            break;

                        case 0x83: // YM2610 DELTA-T ROM data
                            add_rom_data(CHIP_YM2610, ymfm::ACCESS_ADPCM_B, localoffset, size - 8);
                            break;

                        case 0x80: // Sega PCM ROM data
                        case 0x85: // YMF271 ROM data
                        case 0x86: // YMZ280B ROM data
                        case 0x89: // MultiPCM ROM data
                        case 0x8A: // uPD7759 ROM data
                        case 0x8B: // OKIM6295 ROM data
                        case 0x8C: // K054539 ROM data
                        case 0x8D: // C140 ROM data
                        case 0x8E: // K053260 ROM data
                        case 0x8F: // Q-Sound ROM data
                        case 0x90: // ES5505/ES5506 ROM data
                        case 0x91: // X1-010 ROM data
                        case 0x92: // C352 ROM data
                        case 0x93: // GA20 ROM data
                            break;

                        case 0xC0: // RF5C68 RAM write
                        case 0xC1: // RF5C164 RAM write
                        case 0xC2: // NES APU RAM write
                        case 0xE0: // SCSP RAM write
                        case 0xE1: // ES5503 RAM write
                            break;

                        default:
                            if (type >= 0x40 && type < 0x7f) {
                                printf("Compressed data block not supported\n");
                            } else {
                                printf("Unknown data block type 0x%02X\n", type);
                            }
                            break;
                    }
                    offset += size;
                    break;
                }

                // PCM RAM write
                case 0x68:
                    printf("68: PCM RAM write\n");
                    break;

                // AY8910, write value dd to register aa
                case 0xa0:
                    write_chip(CHIP_YM2149, buffer[offset] >> 7, buffer[offset] & 0x7f, buffer[offset + 1]);
                    offset += 2;
                    break;

                case 0x70:
                case 0x71:
                case 0x72:
                case 0x73:
                case 0x74:
                case 0x75:
                case 0x76:
                case 0x77:
                case 0x78:
                case 0x79:
                case 0x7a:
                case 0x7b:
                case 0x7c:
                case 0x7d:
                case 0x7e:
                case 0x7f:
                    wait = (cmd & 15) + 1;
                    break;

                case 0x80:
                case 0x81:
                case 0x82:
                case 0x83:
                case 0x84:
                case 0x85:
                case 0x86:
                case 0x87:
                case 0x88:
                case 0x89:
                case 0x8a:
                case 0x8b:
                case 0x8c:
                case 0x8d:
                case 0x8e:
                case 0x8f: {
                    vgm_chip_base* chip = find_chip(CHIP_YM2612, 0);
                    if (chip != nullptr)
                        chip->write(0x2a, chip->read_pcm());
                    wait = cmd & 15;
                    break;
                }

                // ignored, consume one byte
                case 0x30:
                case 0x31:
                case 0x32:
                case 0x33:
                case 0x34:
                case 0x35:
                case 0x36:
                case 0x37:
                case 0x38:
                case 0x39:
                case 0x3a:
                case 0x3b:
                case 0x3c:
                case 0x3d:
                case 0x3e:
                case 0x3f:
                case 0x4f: // dd: Game Gear PSG stereo, write dd to port 0x06
                case 0x50: // dd: PSG (SN76489/SN76496) write value dd
                    offset++;
                    break;

                // ignored, consume two bytes
                case 0x40:
                case 0x41:
                case 0x42:
                case 0x43:
                case 0x44:
                case 0x45:
                case 0x46:
                case 0x47:
                case 0x48:
                case 0x49:
                case 0x4a:
                case 0x4b:
                case 0x4c:
                case 0x4d:
                case 0x4e:
                case 0x5d: // aa dd: YMZ280B, write value dd to register aa
                case 0xb0: // aa dd: RF5C68, write value dd to register aa
                case 0xb1: // aa dd: RF5C164, write value dd to register aa
                case 0xb2: // aa dd: PWM, write value ddd to register a (d is MSB, dd is LSB)
                case 0xb3: // aa dd: GameBoy DMG, write value dd to register aa
                case 0xb4: // aa dd: NES APU, write value dd to register aa
                case 0xb5: // aa dd: MultiPCM, write value dd to register aa
                case 0xb6: // aa dd: uPD7759, write value dd to register aa
                case 0xb7: // aa dd: OKIM6258, write value dd to register aa
                case 0xb8: // aa dd: OKIM6295, write value dd to register aa
                case 0xb9: // aa dd: HuC6280, write value dd to register aa
                case 0xba: // aa dd: K053260, write value dd to register aa
                case 0xbb: // aa dd: Pokey, write value dd to register aa
                case 0xbc: // aa dd: WonderSwan, write value dd to register aa
                case 0xbd: // aa dd: SAA1099, write value dd to register aa
                case 0xbe: // aa dd: ES5506, write value dd to register aa
                case 0xbf: // aa dd: GA20, write value dd to register aa
                    offset += 2;
                    break;

                // ignored, consume three bytes
                case 0xc9:
                case 0xca:
                case 0xcb:
                case 0xcc:
                case 0xcd:
                case 0xce:
                case 0xcf:
                case 0xd7:
                case 0xd8:
                case 0xd9:
                case 0xda:
                case 0xdb:
                case 0xdc:
                case 0xdd:
                case 0xde:
                case 0xdf:
                case 0xc0: // bbaa dd: Sega PCM, write value dd to memory offset aabb
                case 0xc1: // bbaa dd: RF5C68, write value dd to memory offset aabb
                case 0xc2: // bbaa dd: RF5C164, write value dd to memory offset aabb
                case 0xc3: // cc bbaa: MultiPCM, write set bank offset aabb to channel cc
                case 0xc4: // mmll rr: QSound, write value mmll to register rr (mm - data MSB, ll - data LSB)
                case 0xc5: // mmll dd: SCSP, write value dd to memory offset mmll (mm - offset MSB, ll - offset LSB)
                case 0xc6: // mmll dd: WonderSwan, write value dd to memory offset mmll (mm - offset MSB, ll - offset LSB)
                case 0xc7: // mmll dd: VSU, write value dd to memory offset mmll (mm - offset MSB, ll - offset LSB)
                case 0xc8: // mmll dd: X1-010, write value dd to memory offset mmll (mm - offset MSB, ll - offset LSB)
                case 0xd1: // pp aa dd: YMF271, port pp, write value dd to register aa
                case 0xd2: // pp aa dd: SCC1, port pp, write value dd to register aa
                case 0xd3: // pp aa dd: K054539, write value dd to register ppaa
                case 0xd4: // pp aa dd: C140, write value dd to register ppaa
                case 0xd5: // pp aa dd: ES5503, write value dd to register ppaa
                case 0xd6: // pp aa dd: ES5506, write value aadd to register pp
                    offset += 3;
                    break;

                // ignored, consume four bytes
                case 0xe0: // dddddddd: Seek to offset dddddddd (Intel byte order) in PCM data bank of data block type 0 (YM2612).
                {
                    vgm_chip_base* chip = find_chip(CHIP_YM2612, 0);
                    uint32_t pos = parse_uint32();
                    if (chip != nullptr) {
                        chip->seek_pcm(pos);
                    }
                    offset += 4;
                    break;
                }
                case 0xe1: // mmll aadd: C352, write value aadd to register mmll
                case 0xe2:
                case 0xe3:
                case 0xe4:
                case 0xe5:
                case 0xe6:
                case 0xe7:
                case 0xe8:
                case 0xe9:
                case 0xea:
                case 0xeb:
                case 0xec:
                case 0xed:
                case 0xee:
                case 0xef:
                case 0xf0:
                case 0xf1:
                case 0xf2:
                case 0xf3:
                case 0xf4:
                case 0xf5:
                case 0xf6:
                case 0xf7:
                case 0xf8:
                case 0xf9:
                case 0xfa:
                case 0xfb:
                case 0xfc:
                case 0xfd:
                case 0xfe:
                case 0xff:
                    offset += 4;
                    break;
            }
        }
        if (done) {
            wait = 1;
        }
    }
};

#if 0
//-------------------------------------------------
//  parse_header - parse the vgm header, adding
//  chips for anything we encounter that we can
//  support
//-------------------------------------------------


//-------------------------------------------------
//  add_rom_data - add data to the given chip
//  type in the given access class
//-------------------------------------------------

//-------------------------------------------------
//  generate_all - generate everything described
//  in the vgmplay file
//-------------------------------------------------

void generate_all(std::vector<uint8_t>& buffer, uint32_t data_start, uint32_t output_rate, std::vector<int32_t>& wav_buffer)
{
    // set the offset to the data start and go
    uint32_t offset = data_start;
    bool done = false;
    emulated_time output_step = 0x100000000ull / output_rate;
    emulated_time output_pos = 0;
}

//-------------------------------------------------
//  write_wav - write a WAV file from the provided
//  stereo data
//-------------------------------------------------

int write_wav(char const* filename, uint32_t output_rate, std::vector<int32_t>& wav_buffer_src)
{
    // determine normalization parameters
    int32_t max_scale = 0;
    for (size_t index = 0; index < wav_buffer_src.size(); index++) {
        int32_t absval = std::abs(wav_buffer_src[index]);
        max_scale = std::max(max_scale, absval);
    }

    // warn if only silence was detected (and also avoid divide by zero)
    if (max_scale == 0) {
        printf("The WAV file data will only contain silence.\n");
        max_scale = 1;
    }

    // now convert
    std::vector<int16_t> wav_buffer(wav_buffer_src.size());
    for (size_t index = 0; index < wav_buffer_src.size(); index++)
        wav_buffer[index] = wav_buffer_src[index] * 26000 / max_scale;

    // write the WAV file
    FILE* out = fopen(filename, "wb");
    if (out == nullptr) {
        printf("Error creating output file '%s'\n", filename);
        return 6;
    }

    // write the 'RIFF' header
    if (fwrite("RIFF", 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the total size
    uint32_t total_size = 48 + wav_buffer.size() * 2 - 8;
    uint8_t wavdata[4];
    wavdata[0] = total_size >> 0;
    wavdata[1] = total_size >> 8;
    wavdata[2] = total_size >> 16;
    wavdata[3] = total_size >> 24;
    if (fwrite(wavdata, 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the 'WAVE' type
    if (fwrite("WAVE", 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the 'fmt ' tag
    if (fwrite("fmt ", 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the format length
    wavdata[0] = 16;
    wavdata[1] = 0;
    wavdata[2] = 0;
    wavdata[3] = 0;
    if (fwrite(wavdata, 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the format (PCM)
    wavdata[0] = 1;
    wavdata[1] = 0;
    if (fwrite(wavdata, 1, 2, out) != 2) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the channels
    wavdata[0] = 2;
    wavdata[1] = 0;
    if (fwrite(wavdata, 1, 2, out) != 2) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the sample rate
    wavdata[0] = output_rate >> 0;
    wavdata[1] = output_rate >> 8;
    wavdata[2] = output_rate >> 16;
    wavdata[3] = output_rate >> 24;
    if (fwrite(wavdata, 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the bytes/second
    uint32_t bps = output_rate * 2 * 2;
    wavdata[0] = bps >> 0;
    wavdata[1] = bps >> 8;
    wavdata[2] = bps >> 16;
    wavdata[3] = bps >> 24;
    if (fwrite(wavdata, 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the block align
    wavdata[0] = 4;
    wavdata[1] = 0;
    if (fwrite(wavdata, 1, 2, out) != 2) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the bits/sample
    wavdata[0] = 16;
    wavdata[1] = 0;
    if (fwrite(wavdata, 1, 2, out) != 2) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the 'data' tag
    if (fwrite("data", 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the data length
    uint32_t datalen = wav_buffer.size() * 2;
    wavdata[0] = datalen >> 0;
    wavdata[1] = datalen >> 8;
    wavdata[2] = datalen >> 16;
    wavdata[3] = datalen >> 24;
    if (fwrite(wavdata, 1, 4, out) != 4) {
        printf("Error writing to output file\n");
        return 7;
    }

    // write the data
    if (fwrite(&wav_buffer[0], 1, datalen, out) != datalen) {
        printf("Error writing to output file\n");
        return 7;
    }
    fclose(out);
    return 0;
}
#endif
