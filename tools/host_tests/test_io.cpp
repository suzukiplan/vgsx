#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>

#include "vdp.hpp"
#include "vgsx.h"
#include "vgs_io.h"

static int fail(const char* msg)
{
    std::fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
}

static int test_readme_vdp_register_doc()
{
    const char* candidates[] = {"README.md", "../../README.md"};
    const char* readmePath = nullptr;
    for (const char* candidate : candidates) {
        if (FILE* fp = std::fopen(candidate, "rb")) {
            std::fclose(fp);
            readmePath = candidate;
            break;
        }
    }
    if (!readmePath) {
        return fail("README.md not found (tried README.md and ../../README.md)");
    }

    char msg[512];
    if (!VDP::checkReadmeVdpRegisterSectionComplete(readmePath, msg, sizeof(msg))) {
        return fail(msg[0] ? msg : "VDP register doc check failed");
    }
    return 0;
}

static int test_random_full_cycle(VGSX& vgs)
{
    constexpr int kSample = 1024;
    std::vector<uint32_t> seq1;
    std::vector<uint32_t> seq2;
    seq1.reserve(kSample);
    seq2.reserve(kSample);

    vgs.outPort(VGS_ADDR_RANDOM, 0);
    for (int i = 0; i < kSample; i++) {
        uint32_t r = vgs.inPort(VGS_ADDR_RANDOM);
        if (r > 0xFFFF) {
            return fail("random out of range");
        }
        seq1.push_back(r);
    }

    vgs.outPort(VGS_ADDR_RANDOM, 0);
    for (int i = 0; i < kSample; i++) {
        uint32_t r = vgs.inPort(VGS_ADDR_RANDOM);
        if (r > 0xFFFF) {
            return fail("random out of range");
        }
        seq2.push_back(r);
    }

    if (seq1 != seq2) {
        return fail("random sequence not deterministic for same seed");
    }
    return 0;
}

static int test_random_seed_io(VGSX& vgs)
{
    vgs.outPort(VGS_ADDR_RANDOM, 0);
    if (vgs.inPort(VGS_ADDR_RANDOM_SEED) != 0) {
        return fail("random seed was not set to zero");
    }
    if (vgs.inPort(VGS_ADDR_RANDOM) != 0xCC5D) {
        return fail("random value did not use the current seed");
    }
    if (vgs.inPort(VGS_ADDR_RANDOM_SEED) != 1) {
        return fail("random seed was not incremented after reading random");
    }
    if (vgs.inPort(VGS_ADDR_RANDOM_SEED) != 1) {
        return fail("reading random seed changed the seed");
    }

    vgs.outPort(VGS_ADDR_RANDOM, 0xFFFFFFFF);
    if (vgs.inPort(VGS_ADDR_RANDOM_SEED) != 0xFFFF) {
        return fail("random seed was not normalized to 16 bits");
    }
    vgs.inPort(VGS_ADDR_RANDOM);
    if (vgs.inPort(VGS_ADDR_RANDOM_SEED) != 0) {
        return fail("random seed did not wrap after 65535");
    }
    return 0;
}

static int test_dma_memset_last_byte(VGSX& vgs)
{
    vgs.outPort(VGS_ADDR_DMA_DESTINATION, 0x00FFFFFF);
    vgs.outPort(VGS_ADDR_DMA_SOURCE, 0x000000AB);
    vgs.outPort(VGS_ADDR_DMA_ARGUMENT, 1);
    vgs.outPort(VGS_ADDR_DMA_EXECUTE, VGS_DMA_MEMSET);
    if (vgs.ctx.ram[0x0FFFFF] != 0xAB) {
        return fail("DMA memset did not write last RAM byte");
    }
    return 0;
}

static int test_seq_write_clamps_to_1mb(VGSX& vgs)
{
    vgs.outPort(VGS_ADDR_SEQ_OPEN_W, 0);
    for (int i = 0; i < (1024 * 1024 + 10); i++) {
        vgs.outPort(VGS_ADDR_SEQ_WRITE, static_cast<uint32_t>(i));
    }
    if (vgs.ctx.sqw.size != 1024U * 1024U) {
        return fail("sequencial write size is not clamped to 1MB");
    }
    return 0;
}

static int test_sprite_size_63_renders_512_pixels()
{
    auto vdp = std::make_unique<VDP>();
    std::vector<uint8_t> ram(1024 * 1024, 0);
    const int spriteWidth = 512;
    const int sourceX = 300;
    const uint32_t color = 0x123456;
    const int ramOffset = sourceX * 4;
    ram[ramOffset + 1] = (color >> 16) & 0xFF;
    ram[ramOffset + 2] = (color >> 8) & 0xFF;
    ram[ramOffset + 3] = color & 0xFF;

    vdp->setCpuRam(ram.data());
    vdp->reset();
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].size = 63;
    vdp->ctx.oam[0].scale = 50;
    vdp->ctx.oam[0].alpha = 0xFFFFFF;
    vdp->ctx.oam[0].ram_ptr = 1;
    vdp->render();

    const int displayX = ((spriteWidth * 2 - spriteWidth) / 2) + sourceX;
    const int displayY = (spriteWidth * 2 - spriteWidth) / 2;
    if (vdp->ctx.display[displayY * VDP_DISPLAY_WIDTH + displayX] != color) {
        return fail("sprite size 63 did not render a source pixel beyond the old 256px limit");
    }
    return 0;
}

static int test_sprite_large_scale_is_clipped_to_display()
{
    auto vdp = std::make_unique<VDP>();
    std::vector<uint8_t> ram(1024 * 1024, 0);
    constexpr uint32_t kColor = 0x345678;
    for (int i = 0; i < 320 * 320; i++) {
        ram[i * 4 + 1] = (kColor >> 16) & 0xFF;
        ram[i * 4 + 2] = (kColor >> 8) & 0xFF;
        ram[i * 4 + 3] = kColor & 0xFF;
    }

    vdp->setCpuRam(ram.data());
    vdp->reset();
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].size = 39; // 320x320
    vdp->ctx.oam[0].scale = 1600;
    vdp->ctx.oam[0].alpha = 0x808080;
    vdp->ctx.oam[0].ram_ptr = 1;
    vdp->ctx.oam[0].x = 2400;
    vdp->ctx.oam[0].y = 2400;
    vdp->render();

    if (vdp->ctx.display[VDP_DISPLAY_WIDTH * 200 + 320] == 0) {
        return fail("large scaled translucent sprite was not rendered inside the clipped display");
    }
    return 0;
}

static int test_sprite_vertical_flip_and_bitmap_origin()
{
    constexpr uint32_t kColor = 0x123456;

    auto vdp = std::make_unique<VDP>();
    vdp->reset();
    vdp->ctx.ptn[0][0] = 0x10;
    vdp->ctx.palette[0][1] = kColor;
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].attr = 0x40000000;
    vdp->ctx.oam[0].scale = 100;
    vdp->ctx.oam[0].alpha = 0xFFFFFF;
    vdp->ctx.reg.skip0 = 1;
    vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = 1;
    vdp->ctx.reg.skip3 = 1;
    vdp->render();
    if (vdp->ctx.display[14 * VDP_DISPLAY_WIDTH] != kColor || vdp->ctx.display[0] == kColor) {
        return fail("sprite vertical flip did not use the vertical flip attribute");
    }

    vdp->reset();
    vdp->ctx.ptn[0][0] = 0x10;
    vdp->ctx.palette[0][1] = kColor;
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].attr = 0x80000000;
    vdp->ctx.oam[0].scale = 100;
    vdp->ctx.oam[0].alpha = 0xFFFFFF;
    vdp->ctx.reg.skip0 = 1;
    vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = 1;
    vdp->ctx.reg.skip3 = 1;
    vdp->render();
    if (vdp->ctx.display[14] != kColor || vdp->ctx.display[0] == kColor) {
        return fail("sprite horizontal flip did not use the horizontal flip attribute");
    }

    std::vector<uint8_t> ram(1024 * 1024, 0);
    ram[1] = (kColor >> 16) & 0xFF;
    ram[2] = (kColor >> 8) & 0xFF;
    ram[3] = kColor & 0xFF;
    vdp->setCpuRam(ram.data());
    vdp->reset();
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].scale = 100;
    vdp->ctx.oam[0].alpha = 0xFFFFFF;
    vdp->ctx.oam[0].ram_ptr = 1;
    vdp->ctx.reg.skip0 = 1;
    vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = 1;
    vdp->ctx.reg.skip3 = 1;
    vdp->render();
    if (vdp->ctx.display[0] != kColor) {
        return fail("bitmap sprite origin was shifted by one pixel");
    }
    return 0;
}

static int test_sprite_alpha_uses_low_24_bits()
{
    auto vdp = std::make_unique<VDP>();
    vdp->reset();
    vdp->ctx.palette[0][0] = 0x112233;
    vdp->ctx.ptn[0][0] = 0x10;
    vdp->ctx.palette[0][1] = 0xFFFFFF;
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].scale = 3200;
    vdp->ctx.oam[0].alpha = 0xFF000000;
    vdp->ctx.reg.skip0 = 1;
    vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = 1;
    vdp->ctx.reg.skip3 = 1;
    vdp->render();
    if (vdp->ctx.display[0] != 0x112233) {
        return fail("sprite alpha ignored the low-24-bit fully transparent value");
    }
    return 0;
}

static void setup_sprite_only_vdp(VDP& vdp)
{
    vdp.reset();
    vdp.ctx.reg.skip0 = 1;
    vdp.ctx.reg.skip1 = 1;
    vdp.ctx.reg.skip2 = 1;
    vdp.ctx.reg.skip3 = 1;
    for (int i = 0; i < 25; i++) {
        std::memset(vdp.ctx.ptn[i], 0x11, sizeof(vdp.ctx.ptn[i]));
    }
    vdp.ctx.palette[1][1] = 0xD06020;
}

static int test_sprite_battle_hanafuda_title_oam()
{
    auto vdp = std::make_unique<VDP>();
    setup_sprite_only_vdp(*vdp);

    // Battle Hanafuda's title effect uses up to 256 40x40 cards. Cards start
    // outside the top edge, lock Y scaling, shrink X to odd percentages, and
    // rotate while falling. Exercise the whole input envelope in one frame.
    for (int i = 0; i < 256; i++) {
        VDP::OAM& oam = vdp->ctx.oam[i];
        oam.visible = 1;
        oam.x = -48 + (i * 97) % 377;
        oam.y = -40 + (i * 53) % 249;
        oam.attr = 0x10000;
        oam.size = 4;
        oam.rotate = -720 + i * 37;
        oam.scale = (i % 50) * 2 + 1;
        oam.alpha = 0xFFFFFFFF;
        oam.slx = 0;
        oam.sly = 1;
    }
    vdp->render();

    bool rendered = false;
    for (uint32_t pixel : vdp->ctx.display) {
        if (pixel) {
            rendered = true;
            break;
        }
    }
    if (!rendered) {
        return fail("Battle Hanafuda title OAM did not render any card pixels");
    }
    for (int i = 0; i < 25; i++) {
        for (uint8_t byte : vdp->ctx.ptn[i]) {
            if (byte != 0x11) {
                return fail("sprite rendering wrote past the display into pattern memory");
            }
        }
    }
    return 0;
}

static int test_sprite_boundaries_and_extreme_oam_values()
{
    auto vdp = std::make_unique<VDP>();
    setup_sprite_only_vdp(*vdp);

    struct TestCase {
        int32_t x;
        int32_t y;
        int32_t rotate;
        uint32_t scale;
        uint32_t attr;
        uint32_t slx;
        uint32_t sly;
        uint32_t alpha;
    } cases[] = {
        {0, 0, 0, 100, 0x10000, 0, 0, 0xFFFFFF},
        {0, 0, 37, 100, 0x10000, 0, 0, 0xFFFFFF},
        {0, 0, 211, 1, 0x10000, 0, 1, 0xFFFFFF},
        {319, 199, -47, 3200, 0x10000, 0, 0, 0x808080},
        {-40, -40, 90, 99, 0xC0010000, 0, 1, 0xFFFFFF},
        {-1000, -1000, 0, 100, 0x10000, 0, 0, 0xFFFFFF},
        {1000, 1000, 0, 100, 0x10000, 0, 0, 0xFFFFFF},
        {std::numeric_limits<int32_t>::min(), 0, 0, 100,
         0x10000, 0, 0, 0xFFFFFF},
        {std::numeric_limits<int32_t>::max(), 0, 0, 100,
         0x10000, 0, 0, 0xFFFFFF},
        {std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min(),
         std::numeric_limits<int32_t>::min(), std::numeric_limits<uint32_t>::max(),
         0x10000, 0, 0, 0xFFFFFF},
        {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max(),
         std::numeric_limits<int32_t>::max(), std::numeric_limits<uint32_t>::max(),
         0x10000, 1, 1, 0xFFFFFF},
    };

    for (const TestCase& test : cases) {
        VDP::OAM& oam = vdp->ctx.oam[0];
        std::memset(&oam, 0, sizeof(oam));
        oam.visible = 1;
        oam.x = test.x;
        oam.y = test.y;
        oam.attr = test.attr;
        oam.size = 4;
        oam.rotate = test.rotate;
        oam.scale = test.scale;
        oam.alpha = test.alpha;
        oam.slx = test.slx;
        oam.sly = test.sly;
        vdp->render();
    }
    return 0;
}

static int test_sprite_ram_flip_scale_rotate_and_alpha()
{
    auto vdp = std::make_unique<VDP>();
    std::vector<uint8_t> ram(1024 * 1024, 0);
    constexpr uint32_t kColor = 0x804020;
    for (int i = 0; i < 40 * 40; i++) {
        ram[i * 4 + 1] = (kColor >> 16) & 0xFF;
        ram[i * 4 + 2] = (kColor >> 8) & 0xFF;
        ram[i * 4 + 3] = kColor & 0xFF;
    }
    vdp->setCpuRam(ram.data());
    setup_sprite_only_vdp(*vdp);

    VDP::OAM& oam = vdp->ctx.oam[0];
    oam.visible = 1;
    oam.x = -20;
    oam.y = -20;
    oam.attr = 0xC0000000;
    oam.size = 4;
    oam.rotate = 180;
    oam.scale = 3200;
    oam.alpha = 0x808080;
    oam.ram_ptr = 1;
    vdp->render();
    for (uint32_t pixel : vdp->ctx.display) {
        if (pixel == 0x402010) {
            return 0;
        }
    }
    return fail("rotated, flipped, scaled RAM sprite alpha result was incorrect");
}

static int test_palette_1024_addressing_and_rendering(VGSX& vgs)
{
    std::vector<uint32_t> palette(VDP_PALETTE_NUM * VDP_PALETTE_COLOR_NUM, 0);
    if (!vgs.loadPalette(palette.data(), palette.size() * sizeof(palette[0]))) {
        return fail("64KB palette data was rejected");
    }
    if (vgs.loadPalette(palette.data(), (palette.size() + 1) * sizeof(palette[0]))) {
        return fail("palette data larger than 64KB was accepted");
    }

    auto vdp = std::make_unique<VDP>();
    vdp->reset();

    constexpr uint32_t kColor = 0x123456;
    constexpr uint32_t kAttr = (1023U << 16);
    vdp->write(0xD1FFFC, kColor);
    if (vdp->read(0xD1FFFC) != kColor) {
        return fail("palette 1023 color 15 read/write failed");
    }

    vdp->ctx.ptn[0][0] = 0x10;
    vdp->ctx.palette[1023][1] = kColor;
    vdp->ctx.nametbl[0][0] = kAttr;
    vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = 1;
    vdp->ctx.reg.skip3 = 1;
    vdp->render();
    if (vdp->ctx.display[0] != kColor) {
        return fail("BG did not render with palette 1023");
    }

    vdp->reset();
    vdp->ctx.ptn[0][0] = 0x10;
    vdp->ctx.palette[1023][1] = kColor;
    vdp->ctx.oam[0].visible = 1;
    vdp->ctx.oam[0].attr = kAttr;
    vdp->ctx.oam[0].scale = 100;
    vdp->ctx.oam[0].alpha = 0xFFFFFF;
    vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = 1;
    vdp->ctx.reg.skip3 = 1;
    vdp->render();
    if (vdp->ctx.display[0] != kColor) {
        return fail("sprite did not render with palette 1023");
    }

    return 0;
}

static int test_mode7()
{
    auto vdp = std::make_unique<VDP>();
    vdp->reset();
    std::memset(vdp->ctx.ptn, 0, sizeof(vdp->ctx.ptn));
    for (int n = 0; n < 4; n++) {
        for (int group = 0; group < 9; group++) {
            const uint32_t address = 0xD200A4 + group * 16 + n * 4;
            const uint32_t initial = group == 1 || group == 4 ? 256 : 0;
            if (vdp->read(address) != initial) return fail("Mode 7 reset registers");
            vdp->write(address, 0xFFFFFFFF);
            const uint32_t expected = group == 0 ? 1 : group <= 4 ? 65535 : 0xFFFFFFFF;
            if (vdp->read(address) != expected) return fail("Mode 7 register masks/readback");
        }
    }
    vdp->reset();
    vdp->ctx.reg.skip1 = vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    vdp->write(0xD20028, 1);
    vdp->write(0xD200A4, 1);
    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 320; x++) {
            vdp->ctx.nametbl[0][y * 320 + x] = 1 + y * 320 + x;
        }
    }
    const int matrices[][4] = {
        {256, 0, 0, 256}, {128, 0, 0, 128}, {128, 128, -128, 128}, {512, 0, 0, 512},
        {0, 256, -256, 0}, {256, -128, 0, 256}, {0, 0, 0, 0},
        {-1, 0, 0, -1}, {-32768, 32767, 32767, -32768},
    };
    for (const auto& m : matrices) {
        for (int variant = 0; variant < 3; variant++) {
            const int32_t cx = variant == 0 ? 0 : variant == 1 ? 160 : INT32_MIN;
            const int32_t cy = variant == 0 ? 0 : variant == 1 ? 100 : INT32_MAX;
            const int32_t tx = variant == 2 ? INT32_MAX : 0;
            const int32_t ty = variant == 2 ? INT32_MIN : 0;
            for (int i = 0; i < 4; i++) vdp->write(0xD200B4 + i * 16, (uint32_t)m[i]);
            vdp->write(0xD200F4, cx);
            vdp->write(0xD20104, cy);
            vdp->write(0xD20114, tx);
            vdp->write(0xD20124, ty);
            vdp->render();
            for (int y = 0; y < 200; y++) {
                for (int x = 0; x < 320; x++) {
                    // Independent floating-point reference, exact for these inputs.
                    const long double u = (long double)x - cx, v = (long double)y - cy;
                    const long double sx = std::floor((m[0] * u + m[1] * v) / 256) + cx + (long double)tx;
                    const long double sy = std::floor((m[2] * u + m[3] * v) / 256) + cy + (long double)ty;
                    const uint32_t expected = sx < 0 || sx >= 320 || sy < 0 || sy >= 200
                                                  ? 0 : 1 + (int)sy * 320 + (int)sx;
                    for (int dy = 0; dy < 2; dy++) {
                        for (int dx = 0; dx < 2; dx++) {
                            if (vdp->ctx.display[(y * 2 + dy) * 640 + x * 2 + dx] != expected)
                                return fail("Mode 7 bitmap transform/reference mismatch");
                        }
                    }
                }
            }
        }
    }
    // Four independently translated layers, mixing both source formats.
    vdp->reset();
    std::memset(vdp->ctx.ptn, 0, sizeof(vdp->ctx.ptn));
    vdp->ctx.ptn[0][31] = 0x01;
    vdp->ctx.palette[1023][1] = 0x123456;
    for (int n = 0; n < 4; n++) {
        vdp->write(0xD200A4 + n * 4, 1);
        vdp->write(0xD20114 + n * 4, (uint32_t)-n);
        if (n % 2) {
            vdp->write(0xD20028 + n * 4, 1);
            vdp->ctx.nametbl[n][0] = 0x100 + n;
        } else {
            vdp->ctx.nametbl[n][0] = 0xC3FF0000; // Both tile flips, palette 1023.
        }
    }
    vdp->render();
    for (int n = 0; n < 4; n++) {
        const uint32_t expected = n % 2 ? 0x100 + n : 0x123456;
        if (vdp->ctx.display[n * 2] != expected) return fail("Mode 7 four-layer composition/flips/palette");
    }
    vdp->ctx.ptn[1][0] = 0x10;
    vdp->ctx.palette[0][1] = 0x654321;
    auto& sprite = vdp->ctx.oam[0];
    sprite.visible = 1;
    sprite.attr = 1;
    sprite.x = 3;
    sprite.scale = 100;
    sprite.alpha = 0xFFFFFF;
    vdp->ctx.reg.spos = 2;
    vdp->render();
    if (vdp->ctx.display[6] != 0x103) return fail("Mode 7 BG above sprite");
    vdp->ctx.reg.spos = 3;
    vdp->render();
    if (vdp->ctx.display[6] != 0x654321) return fail("Mode 7 sprite above BG");
    sprite.visible = 0;
    vdp->ctx.reg.skip0 = vdp->ctx.reg.skip1 = vdp->ctx.reg.skip2 = 1;
    vdp->write(0xD20014, 1); // Bitmap BG3 scrolls its stored pixel right once.
    vdp->render();
    if (vdp->ctx.display[8] != 0x103 || vdp->ctx.display[10] != 0)
        return fail("Mode 7 bitmap scroll was applied twice");
    vdp->ctx.wx1[3] = 5;
    vdp->render();
    if (vdp->ctx.display[8]) return fail("Mode 7 bitmap window is not destination clipping");
    // Character source edge must become transparent, then wrap again when disabled.
    vdp->reset();
    std::memset(vdp->ctx.ptn, 0x11, sizeof(vdp->ctx.ptn));
    vdp->ctx.palette[0][1] = 0xABCDEF;
    vdp->ctx.reg.skip1 = vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    vdp->write(0xD20008, 2047);
    vdp->write(0xD20018, 2047);
    vdp->write(0xD200A4, 1);
    vdp->render();
    if (vdp->ctx.display[0] != 0xABCDEF || vdp->ctx.display[2] || vdp->ctx.display[1280])
        return fail("Mode 7 character bounds/scroll");
    vdp->write(0xD200A4, 0);
    vdp->render();
    if (vdp->ctx.display[2] != 0xABCDEF) return fail("Mode 7 disable restores wrapping");
    vdp->write(0xD200A4, 1);
    vdp->render();
    if (vdp->ctx.display[2]) return fail("Mode 7 re-enable preserves parameters");
    vdp->ctx.reg.skip = 1;
    vdp->ctx.palette[0][1] = 0;
    vdp->render();
    if (vdp->ctx.display[0] != 0xABCDEF) return fail("Mode 7 screen skip");
    uint8_t pattern[32];
    std::memset(pattern, 0x22, sizeof(pattern));
    vdp->addPattern(1, pattern, sizeof(pattern));
    vdp->reset();
    if (vdp->ctx.ptn[0][0] != 0 || vdp->ctx.ptn[1][0] != 0x22 || vdp->ctx.ptn[65535][31] != 0)
        return fail("Reset must clear unspecified patterns and restore ROM patterns");
    if (vdp->read(0xD200A4) || vdp->read(0xD200B4) != 256 || vdp->read(0xD20114))
        return fail("Mode 7 reset after use");
    return 0;
}

int main()
{
    vgsx.disableBootBios();

    if (int rc = test_readme_vdp_register_doc(); rc) return rc;
    if (int rc = test_mode7(); rc) return rc;
    if (int rc = test_random_full_cycle(vgsx); rc) return rc;
    if (int rc = test_random_seed_io(vgsx); rc) return rc;
    if (int rc = test_dma_memset_last_byte(vgsx); rc) return rc;
    if (int rc = test_seq_write_clamps_to_1mb(vgsx); rc) return rc;
    if (int rc = test_sprite_size_63_renders_512_pixels(); rc) return rc;
    if (int rc = test_sprite_large_scale_is_clipped_to_display(); rc) return rc;
    if (int rc = test_sprite_vertical_flip_and_bitmap_origin(); rc) return rc;
    if (int rc = test_sprite_alpha_uses_low_24_bits(); rc) return rc;
    if (int rc = test_sprite_battle_hanafuda_title_oam(); rc) return rc;
    if (int rc = test_sprite_boundaries_and_extreme_oam_values(); rc) return rc;
    if (int rc = test_sprite_ram_flip_scale_rotate_and_alpha(); rc) return rc;
    if (int rc = test_palette_1024_addressing_and_rendering(vgsx); rc) return rc;

    std::fprintf(stderr, "OK\n");
    return 0;
}
