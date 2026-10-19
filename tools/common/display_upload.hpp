#pragma once
#include <stdint.h>

namespace display_upload {

inline void copyOpaque(const uint32_t* source, uint32_t* destination,
                       int width, int height, int pitchPixels)
{
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            destination[y * pitchPixels + x] = 0xFF000000u | source[y * width + x];
        }
    }
}

} // namespace display_upload
