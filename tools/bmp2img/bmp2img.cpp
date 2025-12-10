#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"

#include <cstdio>
#include <cstdint>
#include <vector>
#include <string>

static bool is_valid_size(int w, int h)
{
    if (w <= 0 || h <= 0) return false;
    if (w > 256 || h > 256) return false;
    if ((w % 8) != 0 || (h % 8) != 0) return false;
    return true;
}

int main(int argc, char** argv)
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s input.(png|bmp) output.img\n", argv[0]);
        return 1;
    }

    int w, h, ch;
    uint8_t* img = stbi_load(argv[1], &w, &h, &ch, 4); // force RGBA
    if (!img) {
        fprintf(stderr, "Failed to load image: %s\n", argv[1]);
        return 1;
    }

    // --- サイズ制約チェック ---
    if (!is_valid_size(w, h)) {
        fprintf(stderr,
                "ERROR: invalid size %dx%d (must be <=256 and multiples of 8)\n",
                w, h);
        stbi_image_free(img);
        return 1;
    }

    // --- 変換処理 ---
    std::vector<uint32_t> out;
    out.reserve(w * h);

    for (int i = 0; i < w * h; ++i) {
        uint8_t r = img[i * 4];
        uint8_t g = img[i * 4 + 1];
        uint8_t b = img[i * 4 + 2];
        uint32_t rgb = r;
        rgb <<= 8;
        rgb |= g;
        rgb <<= 8;
        rgb |= b;
        rgb <<= 8;
        out.push_back(rgb);
    }

    stbi_image_free(img);

    FILE* fp = fopen(argv[2], "wb");
    if (!fp) {
        fprintf(stderr, "Failed to open output file: %s\n", argv[2]);
        return 1;
    }

    fwrite(out.data(), 4, out.size(), fp);
    fclose(fp);

    printf("OK: %dx%d pixels -> %s\n", w, h, argv[2]);
    return 0;
}