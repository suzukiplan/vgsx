#pragma once

#include <cmath>
#include <cstdint>

namespace crt_helper
{

struct Filter {
    static constexpr int kGammaCompressLutSize = 4096;
    static constexpr float kMaskWeights2x[3][3] = {
        {1.00f, 0.99f, 0.99f},
        {0.99f, 1.00f, 0.99f},
        {0.99f, 0.99f, 1.00f},
    };

    bool initialized = false;
    float gammaExpandLut[256];
    uint8_t gammaCompressLut[kGammaCompressLutSize];

    static inline int clampIndex(int value, int min, int max)
    {
        if (value < min) {
            return min;
        }
        if (max < value) {
            return max;
        }
        return value;
    }

    static inline float clampLinear(float value)
    {
        if (value < 0.0f) {
            return 0.0f;
        }
        if (1.0f < value) {
            return 1.0f;
        }
        return value;
    }

    void init()
    {
        if (initialized) {
            return;
        }
        for (int i = 0; i < 256; i++) {
            gammaExpandLut[i] = powf((float)i / 255.0f, 2.2f);
        }
        for (int i = 0; i < kGammaCompressLutSize; i++) {
            const float linear = (float)i / (float)(kGammaCompressLutSize - 1);
            gammaCompressLut[i] = (uint8_t)(powf(linear, 1.0f / 2.2f) * 255.0f + 0.5f);
        }
        initialized = true;
    }

    inline float linearFromSrgb8(uint8_t value) const
    {
        return gammaExpandLut[value];
    }

    inline uint8_t srgb8FromLinear(float value) const
    {
        const float linear = clampLinear(value);
        const int index = (int)(linear * (float)(kGammaCompressLutSize - 1) + 0.5f);
        return gammaCompressLut[index];
    }

    void apply(const uint32_t* src, uint32_t* dst, int width, int height) const
    {
        static const float blurWeights[5] = {0.05f, 0.20f, 0.50f, 0.20f, 0.05f};

        for (int y = 0; y < height; y++) {
            const float scanline = (y & 1) ? 0.82f : 1.00f;
            const int row = y * width;
            for (int x = 0; x < width; x++) {
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;

                // Keep the filter local and cheap: horizontal blur only, then a scanline brightness step.
                for (int i = 0; i < 5; i++) {
                    const int sampleX = clampIndex(x + i - 2, 0, width - 1);
                    const uint32_t pixel = src[row + sampleX];
                    const float weight = blurWeights[i];
                    r += linearFromSrgb8((pixel >> 16) & 0xFF) * weight;
                    g += linearFromSrgb8((pixel >> 8) & 0xFF) * weight;
                    b += linearFromSrgb8(pixel & 0xFF) * weight;
                }

                r *= scanline;
                g *= scanline;
                b *= scanline;
                dst[row + x] = ((uint32_t)srgb8FromLinear(r) << 16) | ((uint32_t)srgb8FromLinear(g) << 8) | srgb8FromLinear(b);
            }
        }
    }

    void apply2x(const uint32_t* src, uint32_t* dst, int width, int height) const
    {
        static const float blurWeights[3] = {0.125f, 0.75f, 0.125f};
        const int dstWidth = width * 2;
        const int dstHeight = height * 2;
        const float vignetteXScale = 2.0f / (float)dstWidth;
        const float vignetteYScale = 2.0f / (float)dstHeight;

        for (int y = 0; y < height; y++) {
            const int srcRow = y * width;
            const int dstRowTop = (y * 2) * dstWidth;
            const int dstRowBottom = dstRowTop + dstWidth;
            const float nyTop = ((float)(y * 2) + 0.5f) * vignetteYScale - 1.0f;
            const float nyBottom = ((float)(y * 2 + 1) + 0.5f) * vignetteYScale - 1.0f;
            int maskPhase = 0;
            for (int x = 0; x < width; x++) {
                float r = 0.0f;
                float g = 0.0f;
                float b = 0.0f;

                // Keep horizontal glow subtle in 2x mode so the scanline contrast stays dominant.
                for (int i = 0; i < 3; i++) {
                    const int sampleX = clampIndex(x + i - 1, 0, width - 1);
                    const uint32_t pixel = src[srcRow + sampleX];
                    const float weight = blurWeights[i];
                    r += linearFromSrgb8((pixel >> 16) & 0xFF) * weight;
                    g += linearFromSrgb8((pixel >> 8) & 0xFF) * weight;
                    b += linearFromSrgb8(pixel & 0xFF) * weight;
                }

                const float nxLeft = ((float)(x * 2) + 0.5f) * vignetteXScale - 1.0f;
                const float nxRight = ((float)(x * 2 + 1) + 0.5f) * vignetteXScale - 1.0f;
                float vignetteLeftTop = 1.0f - (nxLeft * nxLeft + nyTop * nyTop) * 0.012f;
                float vignetteRightTop = 1.0f - (nxRight * nxRight + nyTop * nyTop) * 0.012f;
                float vignetteLeftBottom = 1.0f - (nxLeft * nxLeft + nyBottom * nyBottom) * 0.012f;
                float vignetteRightBottom = 1.0f - (nxRight * nxRight + nyBottom * nyBottom) * 0.012f;
                if (vignetteLeftTop < 0.96f) vignetteLeftTop = 0.96f;
                if (vignetteRightTop < 0.96f) vignetteRightTop = 0.96f;
                if (vignetteLeftBottom < 0.96f) vignetteLeftBottom = 0.96f;
                if (vignetteRightBottom < 0.96f) vignetteRightBottom = 0.96f;

                const float scanlineTop = 1.00f;
                const float scanlineBottom = 0.92f;
                const float* mask = kMaskWeights2x[maskPhase];

                const float topR = r * scanlineTop * mask[0];
                const float topG = g * scanlineTop * mask[1];
                const float topB = b * scanlineTop * mask[2];
                const float bottomR = r * scanlineBottom * mask[0];
                const float bottomG = g * scanlineBottom * mask[1];
                const float bottomB = b * scanlineBottom * mask[2];

                const uint32_t topLeft =
                    0xFF000000u |
                    ((uint32_t)srgb8FromLinear(topR * vignetteLeftTop) << 16) |
                    ((uint32_t)srgb8FromLinear(topG * vignetteLeftTop) << 8) |
                    srgb8FromLinear(topB * vignetteLeftTop);
                const uint32_t topRight =
                    0xFF000000u |
                    ((uint32_t)srgb8FromLinear(topR * vignetteRightTop) << 16) |
                    ((uint32_t)srgb8FromLinear(topG * vignetteRightTop) << 8) |
                    srgb8FromLinear(topB * vignetteRightTop);
                const uint32_t bottomLeft =
                    0xFF000000u |
                    ((uint32_t)srgb8FromLinear(bottomR * vignetteLeftBottom) << 16) |
                    ((uint32_t)srgb8FromLinear(bottomG * vignetteLeftBottom) << 8) |
                    srgb8FromLinear(bottomB * vignetteLeftBottom);
                const uint32_t bottomRight =
                    0xFF000000u |
                    ((uint32_t)srgb8FromLinear(bottomR * vignetteRightBottom) << 16) |
                    ((uint32_t)srgb8FromLinear(bottomG * vignetteRightBottom) << 8) |
                    srgb8FromLinear(bottomB * vignetteRightBottom);

                const int dstX = x * 2;
                dst[dstRowTop + dstX] = topLeft;
                dst[dstRowTop + dstX + 1] = topRight;
                dst[dstRowBottom + dstX] = bottomLeft;
                dst[dstRowBottom + dstX + 1] = bottomRight;

                maskPhase++;
                if (maskPhase == 3) {
                    maskPhase = 0;
                }
            }
        }
    }
};

} // namespace crt_helper
