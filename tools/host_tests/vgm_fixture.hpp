#pragma once
#include <cstdint>
#include <vector>
#include <initializer_list>

// VGM 1.61, YM2612, command stream at 0x100. No external music assets.
static std::vector<uint8_t> makeVgm(std::initializer_list<uint8_t> commands, int loop = -1)
{
    std::vector<uint8_t> data(0x100, 0);
    auto le32 = [&](int offset, uint32_t value) {
        for (int i = 0; i < 4; i++) data[offset + i] = value >> (8 * i);
    };
    data[0] = 'V'; data[1] = 'g'; data[2] = 'm'; data[3] = ' ';
    le32(0x08, 0x161);
    le32(0x2c, 7670454);
    le32(0x34, 0x100 - 0x34);
    if (loop >= 0) le32(0x1c, 0x100 + loop - 0x1c);
    data.insert(data.end(), commands);
    le32(0x04, data.size() - 4);
    return data;
}
