#include <cmath>
#include <cstdint>
#include <cstring>
#include <cstdio>
#include <limits>
#include <memory>
#include <vector>
#include <cstdlib>
#include <string>
#include <unistd.h>

#include "vdp.hpp"
#include "vgsx.h"
#include "vgs_io.h"
#include "../common/frame_pacer.hpp"
#include "../common/display_upload.hpp"

static int fail(const char* msg)
{
    std::fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
}

static int test_readme_vdp_register_doc()
{
    const struct {
        const char* readme;
        const char* vdpHeader;
    } candidates[] = {
        {"README.md", "src/vdp.hpp"},
        {"../../README.md", "../../src/vdp.hpp"},
    };
    const char* readmePath = nullptr;
    for (const auto& candidate : candidates) {
        // Identify the repository root so the host-test README is never
        // mistaken for the register specification. A missing root README
        // must still fail validation rather than falling back to another file.
        if (FILE* fp = std::fopen(candidate.vdpHeader, "rb")) {
            std::fclose(fp);
            readmePath = candidate.readme;
            break;
        }
    }
    if (!readmePath) {
        return fail("repository root not found (tried . and ../..)");
    }

    char msg[512];
    if (!VDP::checkReadmeVdpRegisterSectionComplete(readmePath, msg, sizeof(msg))) {
        return fail(msg[0] ? msg : "VDP register doc check failed");
    }
    return 0;
}

static int test_frame_pacer()
{
    using Pacer = frame_timing::FramePacer;
    using namespace std::chrono;
    const Pacer::TimePoint origin{};
    Pacer pacer(origin);
    if (pacer.nextFrame(origin + milliseconds(4)) != origin + Pacer::period)
        return fail("frame pacing did not wait for the first deadline");
    if (pacer.nextFrame(origin + Pacer::period + milliseconds(4)) != origin + 2 * Pacer::period)
        return fail("frame pacing accumulated processing or sleep time");

    Pacer synchronized(origin);
    if (synchronized.nextFrame(origin + Pacer::period) != origin + Pacer::period)
        return fail("frame pacing added a delay after V-SYNC");
    const auto stalled = origin + milliseconds(80);
    if (synchronized.nextFrame(stalled) != stalled ||
        synchronized.nextFrame(stalled + milliseconds(4)) != stalled + Pacer::period)
        return fail("frame pacing did not recover from a stall without catch-up frames");
    if (Pacer::isLate(microseconds(16891)) || !Pacer::isLate(milliseconds(20)) ||
        !Pacer::isLate(milliseconds(80)))
        return fail("lag detection confused scheduling jitter with an overrun");

    // Simulate 60/120 Hz presentation and 100us sleep overshoot without using
    // real timers. An absolute schedule must not drift into missed V-SYNCs.
    for (int refreshMultiple : {1, 2}) {
        auto now = origin + milliseconds(2);
        Pacer displayPacer(now);
        const auto refresh = Pacer::period / refreshMultiple;
        for (int frame = 0; frame < 600; frame++) {
            const auto ready = now + milliseconds(8);
            const auto elapsed = ready - origin;
            const auto refreshCount = elapsed / refresh + (elapsed % refresh != Pacer::Clock::duration::zero());
            const auto presented = origin + refresh * refreshCount;
            const auto expected = origin + Pacer::period * (frame + 1);
            if (presented < expected - microseconds(10) || presented > expected + microseconds(10))
                return fail("frame pacing drifted and missed a display refresh");
            now = displayPacer.nextFrame(presented);
            if (now > presented) now += microseconds(100);
        }
    }
    return 0;
}

static int test_native_display_upload()
{
    constexpr int width = 17, height = 3, pitch = 24;
    constexpr uint32_t sentinel = 0x11223344;
    uint32_t source[width * height];
    uint32_t destination[pitch * height];
    for (int i = 0; i < width * height; i++) source[i] = i * 0x10203;
    for (auto& pixel : destination) pixel = sentinel;
    display_upload::copyOpaque(source, destination, width, height, pitch);
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < pitch; x++) {
            // A nearest-neighbour 2x copy shrunk back to the window gives
            // exactly these colors. Row padding must remain untouched.
            uint32_t expected = x < width ? 0xFF000000u | source[y * width + x] : sentinel;
            if (destination[y * pitch + x] != expected)
                return fail("native display upload changed colors, opacity or row padding");
        }
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

static int test_sprite_pivot_zoom_does_not_move_focus()
{
    auto ownedVdp = std::make_unique<VDP>();
    VDP& vdp = *ownedVdp;
    std::vector<uint8_t> ram(1024 * 1024, 0);
    constexpr int focusX = 286;
    constexpr int focusY = 125;
    constexpr uint32_t color = 0x123456;
    int address = (focusY * 320 + focusX) * 4;
    ram[address + 1] = 0x12;
    ram[address + 2] = 0x34;
    ram[address + 3] = 0x56;
    vdp.setCpuRam(ram.data());
    vdp.reset();
    vdp.ctx.reg.skip0 = 1;
    vdp.ctx.reg.skip1 = 1;
    vdp.ctx.reg.skip2 = 1;
    vdp.ctx.reg.skip3 = 1;
    auto& sprite = vdp.ctx.oam[0];
    sprite.visible = 1;
    sprite.size = 39;
    sprite.alpha = 0xFFFFFF;
    sprite.ram_ptr = 1;
    sprite.pivot_enabled = 1;
    sprite.pivot_x = focusX * 65536 + 32768;
    sprite.pivot_y = focusY * 65536 + 32768;
    sprite.x = 160 * 65536;
    sprite.y = 100 * 65536;
    int previousLeft = 320;
    int previousRight = 319;
    int previousTop = 200;
    int previousBottom = 199;
    for (int scale = 100; scale <= 800; scale++) {
        sprite.scale = scale;
        vdp.render();
        int left = 640, right = -1, top = 400, bottom = -1;
        for (int x = 300; x < 340; x++) {
            if (vdp.ctx.display[199 * VDP_DISPLAY_WIDTH + x] == color) {
                if (left == 640) left = x;
                right = x;
            }
        }
        for (int y = 180; y < 220; y++) {
            if (vdp.ctx.display[y * VDP_DISPLAY_WIDTH + 319] == color) {
                if (top == 400) top = y;
                bottom = y;
            }
        }
        // An odd raster width can put its center half a display pixel from the
        // pivot. Its edges must still only expand, never oscillate with scale.
        if (left + right < 638 || left + right > 640 || top + bottom < 398 || top + bottom > 400) {
            std::fprintf(stderr, "scale=%d bounds=(%d,%d)-(%d,%d)\n", scale, left, top, right, bottom);
            return fail("zoom moved the rasterized focus away from its pivot");
        }
        if (left > previousLeft || right < previousRight || top > previousTop || bottom < previousBottom)
            return fail("zoom made a focus edge move backwards");
        previousLeft = left;
        previousRight = right;
        previousTop = top;
        previousBottom = bottom;
    }
    sprite.scale = 0;
    vdp.render();
    if (vdp.ctx.display[199 * VDP_DISPLAY_WIDTH + 319] == color)
        return fail("zero-scale pivot sprite was drawn");
    sprite.scale = 800;
    sprite.alpha = 0;
    vdp.render();
    if (vdp.ctx.display[199 * VDP_DISPLAY_WIDTH + 319] == color)
        return fail("transparent pivot sprite was drawn");

    // Both features must compose in the same frame after the merge.
    vdp.ctx.reg.skip0 = 0;
    vdp.ctx.reg.m7_en[0] = 1;
    vdp.ctx.reg.m7_a[0] = vdp.ctx.reg.m7_d[0] = 0;
    vdp.ctx.reg.m7_b[0] = 0xFF00;
    vdp.ctx.reg.m7_c[0] = 0x100;
    vdp.ctx.reg.m7_tx[0] = vdp.ctx.reg.m7_ty[0] = 1024;
    std::memset(vdp.ctx.ptn[0], 0x11, 32);
    vdp.ctx.palette[0][1] = 0x00A700;
    sprite.alpha = 0xFFFFFF;
    vdp.render();
    if (vdp.ctx.display[0] != 0x00A700 ||
        vdp.ctx.display[199 * VDP_DISPLAY_WIDTH + 319] != color)
        return fail("Mode 7 BG and pivot-scaled sprite must compose together");
    return 0;
}

static int test_sprite_pivot_flip_keeps_source_anchor()
{
    auto vdp = std::make_unique<VDP>();
    std::vector<uint8_t> ram(1024 * 1024, 0);
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int address = (y * 8 + x) * 4;
            ram[address + 1] = x + 1;
            ram[address + 2] = y + 1;
            ram[address + 3] = 0x56;
        }
    }
    vdp->setCpuRam(ram.data());
    vdp->reset();
    vdp->ctx.reg.skip0 = vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    auto& sprite = vdp->ctx.oam[0];
    sprite.visible = 1;
    sprite.alpha = 0xFFFFFF;
    sprite.ram_ptr = 1;
    sprite.pivot_enabled = 1;
    sprite.pivot_x = 1 * 65536 + 16384;
    sprite.pivot_y = 5 * 65536 + 49152;
    // Place the destination pivot exactly at a display pixel center so its
    // source sample is invariant under both flipping and scaling.
    sprite.x = 80 * 65536 + 16384;
    sprite.y = 60 * 65536 + 16384;
    constexpr int anchorX = 160;
    constexpr int anchorY = 120;
    constexpr uint32_t anchorColor = 0x020656;
    const int scales[] = {50, 100, 175, 400};
    for (int scale : scales) {
        sprite.scale = scale;
        for (int locks = 0; locks < 4; locks++) {
            sprite.slx = locks & 1;
            sprite.sly = (locks >> 1) & 1;
            sprite.attr = 0;
            vdp->render();
            std::vector<uint32_t> original(vdp->ctx.display,
                vdp->ctx.display + VDP_DISPLAY_WIDTH * VDP_DISPLAY_HEIGHT);
            for (uint32_t flips = 0; flips < 4; flips++) {
                sprite.attr = flips << 30;
                vdp->render();
                if (vdp->ctx.display[anchorY * VDP_DISPLAY_WIDTH + anchorX] != anchorColor)
                    return fail("flipped pivot sprite moved the specified source anchor");
                for (int y = 0; y < VDP_DISPLAY_HEIGHT; y++) {
                    for (int x = 0; x < VDP_DISPLAY_WIDTH; x++) {
                        int originalX = (flips & 2) ? 2 * anchorX - x : x;
                        int originalY = (flips & 1) ? 2 * anchorY - y : y;
                        uint32_t expected = 0;
                        if (0 <= originalX && originalX < VDP_DISPLAY_WIDTH &&
                            0 <= originalY && originalY < VDP_DISPLAY_HEIGHT)
                            expected = original[originalY * VDP_DISPLAY_WIDTH + originalX];
                        if (vdp->ctx.display[y * VDP_DISPLAY_WIDTH + x] != expected)
                            return fail("pivot flip did not reflect the sprite around its destination anchor");
                    }
                }
            }
        }
    }
    return 0;
}

static int test_sprite_pivot_clipping_matches_full_frame_sampling()
{
    auto vdp = std::make_unique<VDP>();
    std::vector<uint8_t> ram(1024 * 1024, 0);
    for (int y = 0; y < 8; y++) {
        for (int x = 0; x < 8; x++) {
            int address = (y * 8 + x) * 4;
            ram[address + 1] = x + 1;
            ram[address + 2] = y + 1;
            ram[address + 3] = 0x56;
        }
    }
    vdp->setCpuRam(ram.data());
    vdp->reset();
    vdp->ctx.reg.skip0 = vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    auto& sprite = vdp->ctx.oam[0];
    sprite.visible = 1;
    sprite.alpha = 0xFFFFFF;
    sprite.ram_ptr = 1;
    sprite.pivot_enabled = 1;
    struct TestCase {
        int32_t x, y, pivotX, pivotY;
    } cases[] = {
        {16384, 16384, 0, 8 * 65536},
        {-32768, -16384, 81920, 376832},
        {319 * 65536 + 49152, 199 * 65536 + 16384, 376832, 81920},
        {80 * 65536 + 16384, 60 * 65536 + 49152, -32768, 9 * 65536},
        {-1000 * 65536, 100 * 65536, 81920, 376832},
        {100 * 65536, 1000 * 65536, 81920, 376832},
        {std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max(),
         std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min()},
        {std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::min(),
         std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::max()},
    };
    const uint32_t scales[] = {7, 50, 175, 3200, std::numeric_limits<uint32_t>::max()};
    for (const auto& test : cases) {
        sprite.x = test.x;
        sprite.y = test.y;
        sprite.pivot_x = test.pivotX;
        sprite.pivot_y = test.pivotY;
        for (uint32_t scale : scales) {
            sprite.scale = scale;
            for (uint32_t flags = 0; flags < 16; flags++) {
                sprite.attr = (flags & 3) << 30;
                sprite.slx = (flags >> 2) & 1;
                sprite.sly = (flags >> 3) & 1;
                vdp->render();
                // Reference the unbounded inverse mapping, independently of
                // how the renderer selects its destination scan rectangle.
                auto sample = [&](int pixel, int32_t destination, int32_t pivot, bool lock, bool flip) {
                    int axisScale = lock ? 200 : (int)(scale > 3200 ? 3200 : scale) * VDP_DISPLAY_SCALE;
                    int64_t displacement = ((int64_t)pixel * 65536 + 32768 -
                        (int64_t)destination * VDP_DISPLAY_SCALE) * 100;
                    int64_t numerator = (int64_t)pivot * axisScale + (flip ? -displacement : displacement);
                    int64_t denominator = (int64_t)axisScale * 65536;
                    return numerator < 0 || numerator >= 8 * denominator ? -1 : (int)(numerator / denominator);
                };
                int sourceX[VDP_DISPLAY_WIDTH];
                for (int x = 0; x < VDP_DISPLAY_WIDTH; x++)
                    sourceX[x] = sample(x, sprite.x, sprite.pivot_x, sprite.slx, flags & 2);
                for (int y = 0; y < VDP_DISPLAY_HEIGHT; y++) {
                    int sy = sample(y, sprite.y, sprite.pivot_y, sprite.sly, flags & 1);
                    for (int x = 0; x < VDP_DISPLAY_WIDTH; x++) {
                        int sx = sourceX[x];
                        uint32_t expected = sx < 0 || sy < 0 ? 0 :
                            (uint32_t)((sx + 1) << 16 | (sy + 1) << 8 | 0x56);
                        if (vdp->ctx.display[y * VDP_DISPLAY_WIDTH + x] != expected)
                            return fail("clipped pivot sprite differs from full-frame sampling");
                    }
                }
            }
        }
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

static int test_sprite_sampling_matches_reference()
{
    auto vdp = std::make_unique<VDP>();
    setup_sprite_only_vdp(*vdp);
    constexpr int size = 40;
    constexpr uint32_t background = 0x204060;
    const uint32_t colors[] = {0, 0, 0xABCDEF, 0x123456};
    std::vector<uint8_t> ram(1024 * 1024, 0);
    vdp->setCpuRam(ram.data());
    vdp->ctx.palette[0][0] = background;
    vdp->ctx.reg.skip0 = 0;
    vdp->ctx.reg.bmp[0] = 1;
    for (int y = 0; y < VDP_HEIGHT; y++) {
        for (int x = 0; x < VDP_WIDTH; x++)
            vdp->ctx.nametbl[0][y * VDP_WIDTH + x] = background + (x + y) % 16;
    }
    for (int i = 0; i < 4; i++) vdp->ctx.palette[1][i] = colors[i];
    std::memset(vdp->ctx.ptn, 0, 25 * 32);
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            int col = (x * 3 + y * 5) % 4;
            int tile = (y / 8) * 5 + x / 8;
            vdp->ctx.ptn[tile][(y % 8) * 4 + (x % 8) / 2] |= col << (x % 2 ? 0 : 4);
            int address = (y * size + x) * 4;
            ram[address + 1] = colors[col] >> 16;
            ram[address + 2] = colors[col] >> 8;
            ram[address + 3] = colors[col];
        }
    }
    struct TestCase {
        int x, y, rotate, scale, slx, sly;
    } cases[] = {
        {13, 27, 0, 7, 0, 1},
        {-20, -10, 0, 175, 0, 0},
        {300, 190, 0, 399, 1, 0},
        {30, 40, 37, 99, 0, 1},
        {-15, -10, -47, 175, 1, 0},
        {310, 190, 211, 399, 0, 0},
        {0, 0, 90, 100, 0, 0},
        {0, 0, 37, 3200, 0, 0},
    };
    auto& sprite = vdp->ctx.oam[0];
    sprite.visible = 1;
    sprite.size = 4;
    for (const auto& test : cases) {
        sprite.x = test.x;
        sprite.y = test.y;
        sprite.rotate = test.rotate;
        sprite.scale = test.scale;
        sprite.slx = test.slx;
        sprite.sly = test.sly;
        int width = size * (test.slx ? 200 : test.scale * 2) / 100;
        int height = size * (test.sly ? 200 : test.scale * 2) / 100;
        int ox = test.x * 2 + (size * 2 - width) / 2;
        int oy = test.y * 2 + (size * 2 - height) / 2;
        int angle = ((90 - test.rotate) % 360 + 360) % 360;
        int sine = vgsx_sin[angle], cosine = vgsx_cos[angle];
        int cx = ox + width / 2, cy = oy + height / 2;
        int left = VDP_DISPLAY_WIDTH, right = -1, top = VDP_DISPLAY_HEIGHT, bottom = -1;
        for (int by : {0, height - 1}) {
            for (int bx : {0, width - 1}) {
                int x = ((bx - width / 2) * sine - (by - height / 2) * cosine) / 256 + cx;
                int y = ((by - height / 2) * sine + (bx - width / 2) * cosine) / 256 + cy;
                if (x < left) left = x;
                if (x > right) right = x;
                if (y < top) top = y;
                if (y > bottom) bottom = y;
            }
        }
        for (int flags = 0; flags < 32; flags++) {
            sprite.attr = 0x10000 | ((uint32_t)(flags & 3) << 30);
            sprite.ram_ptr = flags & 4 ? 1 : 0;
            sprite.alpha = flags & 8 ? 0xD08040 : 0xFFFFFFFF;
            sprite.mask = flags & 16 ? 0xF09030 : 0;
            vdp->render();
            for (int y = 0; y < VDP_DISPLAY_HEIGHT; y++) {
                for (int x = 0; x < VDP_DISPLAY_WIDTH; x++) {
                    const uint32_t backdrop = background + (x / 2 + y / 2) % 16;
                    uint32_t expected = backdrop;
                    // Direct per-pixel inverse mapping is independent of row
                    // caching and incremental rotation in the renderer.
                    int bx = ((x - cx) * sine + (y - cy) * cosine) / 256 + width / 2;
                    int by = (-(x - cx) * cosine + (y - cy) * sine) / 256 + height / 2;
                    if (left <= x && x <= right && top <= y && y <= bottom &&
                        0 <= bx && bx < width && 0 <= by && by < height) {
                        int sx = bx * size / width;
                        int sy = by * size / height;
                        if (flags & 2) sx = size - sx - 1;
                        if (flags & 1) sy = size - sy - 1;
                        int col = (sx * 3 + sy * 5) % 4;
                        // Palette index 1 is opaque black; RAM black is transparent.
                        bool visible = sprite.ram_ptr ? colors[col] != 0 : col != 0;
                        if (visible) {
                            uint32_t color = sprite.mask ? sprite.mask : colors[col];
                            expected = 0;
                            for (int shift : {0, 8, 16}) {
                                uint32_t alpha = (sprite.alpha >> shift) & 255;
                                uint32_t value = (((color >> shift) & 255) * alpha +
                                    ((backdrop >> shift) & 255) * (255 - alpha)) >> 8;
                                if (sprite.alpha == 0xFFFFFFFF) value = (color >> shift) & 255;
                                expected |= value << shift;
                            }
                        }
                    }
                    if (vdp->ctx.display[y * VDP_DISPLAY_WIDTH + x] != expected)
                        return fail("sprite sampling changed with scaling, rotation, flips, alpha or masks");
                }
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

static int test_mode7_depth()
{
    auto vdp = std::make_unique<VDP>();
    vdp->reset();
    for (int bg = 0; bg < 4; bg++) {
        const uint32_t address = 0xD20134 + bg * 4;
        if (vdp->read(address)) return fail("Mode 7 perspective reset");
        vdp->write(address, 0xFFFFFFFF);
        if (vdp->read(address) != 75) return fail("Mode 7 perspective angle clamp");
        vdp->write(address, 0);
    }
    vdp->ctx.reg.skip1 = vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    vdp->write(0xD20028, 1);
    vdp->write(0xD200A4, 1);
    for (int i = 0; i < 320 * 200; i++) vdp->ctx.nametbl[0][i] = i + 1;
    int previousTop = -1;
    int previousWidth = 321;
    for (int angle : {0, 15, 30, 60, 75}) {
        vdp->write(0xD20134, angle);
        vdp->render();
        int top = 200;
        for (int y = 0; y < 200; y++) {
            for (int x = 0; x < 320; x++) {
                const uint32_t pixel = vdp->ctx.display[y * 1280 + x * 2];
                if (pixel && y < top) top = y;
                if (pixel) {
                    // The destination must lie within the forward projection of
                    // the sampled pixel's unit square (including steep angles).
                    const double sourceX = (pixel - 1) % 320;
                    const double sourceY = (pixel - 1) / 320;
                    const double sine = vgsx_sin[angle] / 256.0;
                    const double cosine = vgsx_cos[angle] / 256.0;
                    const double nearDen = 1 - 99 * sine / 200;
                    double px[4], py[4];
                    const int ox[4] = {0, 1, 1, 0}, oy[4] = {0, 0, 1, 1};
                    for (int corner = 0; corner < 4; corner++) {
                        const double v = sourceY + oy[corner] - 100;
                        const double factor = 1 - v * sine / 200;
                        px[corner] = 160 + (sourceX + ox[corner] - 160) * nearDen / factor;
                        py[corner] = 199 - 99 * cosine / nearDen + v * cosine / factor;
                    }
                    for (int corner = 0; corner < 4; corner++) {
                        const int next = (corner + 1) % 4;
                        const double cross = (px[next] - px[corner]) * (y - py[corner]) -
                                             (py[next] - py[corner]) * (x - px[corner]);
                        if (cross < -1e-7) return fail("Mode 7 perspective forward projection");
                    }
                }
            }
        }
        if (top < previousTop || top == 200) return fail("Mode 7 perspective top margin");
        previousTop = top;
        int topWidth = 0;
        for (int x = 0; x < 320; x++) {
            if (vdp->ctx.display[top * 1280 + x * 2]) topWidth++;
            if (vdp->ctx.display[199 * 1280 + x * 2] != (uint32_t)(199 * 320 + x + 1))
                return fail("Mode 7 perspective bottom must remain aligned and full-width");
        }
        if (topWidth >= previousWidth) return fail("Mode 7 far edge must narrow as depth increases");
        previousWidth = topWidth;
    }
    vdp->write(0xD20134, 0);
    vdp->render();
    for (int y = 0; y < 200; y++) {
        for (int x = 0; x < 320; x++) {
            if (vdp->ctx.display[y * 1280 + x * 2] != (uint32_t)(y * 320 + x + 1))
                return fail("Disabling perspective must restore exact affine output");
        }
    }
    vdp->write(0xD20134, 30);
    vdp->ctx.reg.m7_cx[0] = INT32_MIN;
    vdp->ctx.reg.m7_cy[0] = INT32_MAX;
    vdp->ctx.reg.m7_a[0] = vdp->ctx.reg.m7_b[0] = 0x8000;
    vdp->ctx.reg.m7_c[0] = vdp->ctx.reg.m7_d[0] = 0x7FFF;
    vdp->render(); // Extreme affine inputs must remain safe under perspective.
    for (int bg = 0; bg < 4; bg++) {
        vdp->reset();
        std::memset(vdp->ctx.ptn[0], 0x11, 32);
        vdp->ctx.palette[0][0] = 0x112233;
        vdp->ctx.palette[0][1] = 0xABCDEF;
        for (int n = 0; n < 4; n++) vdp->write(0xD2006C + n * 4, n != bg);
        vdp->write(0xD200A4 + bg * 4, 1);
        vdp->write(0xD20134 + bg * 4, 30);
        if (bg & 1) {
            vdp->write(0xD20028 + bg * 4, 1);
            for (int i = 0; i < 320 * 200; i++) vdp->ctx.nametbl[bg][i] = 0xABCDEF;
        }
        vdp->render();
        if (vdp->ctx.display[0] != 0x112233 || vdp->ctx.display[199 * 1280] != 0xABCDEF)
            return fail("Perspective must work independently on all BGs and source modes");
        if (bg & 1) {
            vdp->ctx.wy2[bg] = 198;
            vdp->render();
            if (vdp->ctx.display[199 * 1280] != 0x112233)
                return fail("Perspective must respect bitmap destination clipping");
        }
    }
    vdp->reset();
    if (vdp->read(0xD20134)) return fail("Mode 7 perspective reset after use");
    return 0;
}

static int test_mode7_perspective_horizontal_coverage()
{
    auto vdp = std::make_unique<VDP>();
    for (bool bitmap : {false, true}) {
        vdp->reset();
        vdp->ctx.reg.skip1 = vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
        vdp->write(0xD200A4, 1);
        vdp->write(0xD20028, bitmap);
        vdp->write(0xD200F4, 160);
        if (bitmap) {
            // A zoomed bitmap also has valid samples beyond the unprojected
            // viewport. Encode X in the color to detect stretched edge pixels.
            vdp->write(0xD200B4, 64);
            for (int y = 0; y < 200; y++) {
                for (int x = 0; x < 320; x++) vdp->ctx.nametbl[0][y * 320 + x] = x + 1;
            }
        } else {
            vdp->write(0xD20114, 800);
            std::memset(vdp->ctx.ptn[0], 0x11, 32);
            vdp->ctx.palette[0][1] = 0xABCDEF;
        }
        for (int depth : {30, 65, 75}) {
            vdp->write(0xD20134, depth);
            vdp->render();
            int top = 0;
            while (top < 200 && !vdp->ctx.display[top * 1280 + 320]) top++;
            if (top == 0 || top == 200) return fail("Perspective must preserve the upper margin");
            for (int y = 0; y < 200; y++) {
                for (int x = 0; x < 320; x++) {
                    const uint32_t color = vdp->ctx.display[y * 1280 + x * 2];
                    if ((color != 0) != (y >= top))
                        return fail("Perspective must cover both sides of the reference trapezoid");
                }
            }
            if (bitmap) {
                const auto* line = &vdp->ctx.display[top * 1280];
                if (!(line[0] < line[32] && line[32] < line[320] && line[320] < line[606] && line[606] < line[638]))
                    return fail("Perspective sides must sample the map instead of repeating borders");
            }
        }
        // Keep genuine source bounds transparent; do not wrap or clamp them.
        vdp->write(0xD20114, 0);
        vdp->write(0xD200B4, 256);
        vdp->render();
        if (vdp->ctx.display[150 * 1280] != 0)
            return fail("Perspective must still reject samples outside the source BG");
    }
    return 0;
}

static int test_mode7_fractional_translation()
{
    auto vdp = std::make_unique<VDP>();
    for (int bg = 0; bg < 4; bg++) {
        vdp->reset();
        const uint32_t fractionAddress = 0xD20144 + 4 * bg;
        if (vdp->read(fractionAddress)) return fail("Fractional translation reset");
        vdp->write(fractionAddress, 0xFFFFFFFF);
        if (vdp->read(fractionAddress) != 65535) return fail("Fractional translation reserved bits");
        for (int n = 0; n < 4; n++) vdp->write(0xD2006C + n * 4, n != bg);
        vdp->write(0xD20028 + bg * 4, 1);
        vdp->write(0xD200A4 + bg * 4, 1);
        for (int i = 0; i < 320 * 200; i++) vdp->ctx.nametbl[bg][i] = i + 1;
        vdp->write(0xD200E4 + bg * 4, 128);
        for (int depth : {0, 30, 75}) {
            vdp->write(0xD20134 + bg * 4, depth);
            for (bool negative : {false, true}) {
                vdp->write(0xD200B4 + bg * 4, negative ? 0xFF80 : 128);
                vdp->write(0xD20114 + bg * 4, negative ? 1 : 0);
                for (int fx : {0, 127, 128, 255}) {
                    for (int fy : {0, 127, 128, 255}) {
                        vdp->write(fractionAddress, fx | (fy << 8));
                        vdp->render();
                        // The bottom edge's inverse perspective is exact. At
                        // x=1,y=199 the affine parts end in half-pixel fractions.
                        const uint32_t expected = (99 + (fy >= 128)) * 320 + (fx >= 128) + 1;
                        if (vdp->ctx.display[199 * 1280 + 2] != expected)
                            return fail("Fractional translation must precede source rounding");
                    }
                }
            }
        }
    }
    vdp->reset();
    for (int bg = 0; bg < 4; bg++) {
        if (vdp->read(0xD20144 + bg * 4)) return fail("Fractional translation reset after use");
    }
    return 0;
}

static int test_mode7_backdrop()
{
    auto vdp = std::make_unique<VDP>();
    for (int bg = 0; bg < 4; bg++) {
        for (bool bitmap : {false, true}) {
            vdp->reset();
            const uint32_t address = 0xD20164 + bg * 4;
            if (vdp->read(address)) return fail("Backdrop reset");
            vdp->write(address, 0xFF00A700);
            if (vdp->read(address) != 0x00A700) return fail("Backdrop RGB mask/readback");
            for (int n = 0; n < 4; n++) vdp->write(0xD2006C + n * 4, n != bg);
            vdp->ctx.palette[0][0] = 0x112233;
            vdp->write(0xD20028 + bg * 4, bitmap);
            vdp->write(0xD200A4 + bg * 4, 1);
            // A quarter turn maps (x,y) to (-y,x): only row zero is in bounds.
            vdp->write(0xD200B4 + bg * 4, 0);
            vdp->write(0xD200C4 + bg * 4, 0xFF00);
            vdp->write(0xD200D4 + bg * 4, 256);
            vdp->write(0xD200E4 + bg * 4, 0);
            vdp->render();
            if (vdp->ctx.display[0] != 0x112233)
                return fail("Backdrop must not fill transparent source pixels");
            for (int y = 2; y < 400; y++) {
                for (int x = 0; x < 640; x++) {
                    if (vdp->ctx.display[y * 640 + x] != 0x00A700)
                        return fail("Rotated source margin backdrop and display scaling");
                }
            }
            vdp->write(address, 0);
            vdp->render();
            if (vdp->ctx.display[1280] != 0x112233) return fail("Zero backdrop preserves transparency");
            vdp->write(address, 0x00A700);
            vdp->write(0xD20134 + bg * 4, 30);
            vdp->render();
            if (vdp->ctx.display[0] != 0x112233 || vdp->ctx.display[199 * 1280] != 0x00A700)
                return fail("Perspective backdrop must preserve sky margin");
            if (bitmap) {
                vdp->ctx.wx1[bg] = 1;
                vdp->render();
                if (vdp->ctx.display[199 * 1280] != 0x112233 || vdp->ctx.display[199 * 1280 + 2] != 0x00A700)
                    return fail("Backdrop must respect bitmap window");
            }
            vdp->write(0xD200A4 + bg * 4, 0);
            vdp->render();
            if (vdp->ctx.display[199 * 1280 + 2] != 0x112233)
                return fail("Backdrop must be ignored without Mode 7");
            vdp->write(0xD200A4 + bg * 4, 1);
            vdp->write(0xD2006C + bg * 4, 1);
            vdp->render();
            if (vdp->ctx.display[199 * 1280 + 2] != 0x112233)
                return fail("Backdrop must respect BG skipping");
            vdp->reset();
            if (vdp->read(address)) return fail("Backdrop reset after use");
        }
    }
    return 0;
}

static int test_mode7_focal_length()
{
    auto vdp = std::make_unique<VDP>();
    vdp->reset();
    for (int bg = 0; bg < 4; bg++) {
        uint32_t address = 0xD20154 + 4 * bg;
        if (vdp->read(address) != 200) return fail("Perspective focal default must preserve existing projection");
        vdp->write(address, 0);
        if (vdp->read(address) != 100) return fail("Perspective focal lower bound");
        vdp->write(address, 0xFFFFFFFF);
        if (vdp->read(address) != 4096) return fail("Perspective focal upper bound");
    }
    vdp->ctx.reg.skip1 = vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    vdp->write(0xD20028, 1);
    vdp->write(0xD200A4, 1);
    for (int i = 0; i < 320 * 200; i++) vdp->ctx.nametbl[0][i] = i + 1;
    uint32_t previousSample = 0;
    for (int focal : {100, 128, 200, 4096}) {
        vdp->write(0xD20154, focal);
        vdp->write(0xD20134, 65);
        vdp->render();
        for (int x = 0; x < 320; x++) {
            if (vdp->ctx.display[199 * 1280 + x * 2] != (uint32_t)(199 * 320 + x + 1))
                return fail("Focal adjustment must preserve bottom alignment");
        }
        uint32_t sample = vdp->ctx.display[150 * 1280 + 320];
        if (!sample || sample == previousSample) return fail("Focal adjustment must change perspective depth");
        previousSample = sample;
        vdp->write(0xD20134, 0);
        vdp->render();
        for (int i = 0; i < 320 * 200; i++) {
            if (vdp->ctx.display[(i / 320) * 1280 + (i % 320) * 2] != (uint32_t)(i + 1))
                return fail("Focal length must not affect affine-only rendering");
        }
    }
    vdp->reset();
    if (vdp->read(0xD20154) != 200) return fail("Focal reset after use");
    return 0;
}

static int test_implicit_save()
{
    struct SaveDirectory {
        char path[64] = "/tmp/vgsx-save-XXXXXX";
        std::string file;
        ~SaveDirectory()
        {
            if (!file.empty()) {
                std::remove(file.c_str());
                rmdir(path);
            }
        }
    } directory;
    if (!mkdtemp(directory.path)) return fail("create save test directory");
    directory.file = std::string(directory.path) + "/save.dat";
    auto vgs = std::make_unique<VGSX>();
    vgs->setSaveDataDirectory(directory.path);
    auto select = [&](uint32_t address, uint32_t size) {
        vgs->outPort(VGS_ADDR_SAVE_ADDRESS, address);
        vgs->outPort(VGS_ADDR_SAVE_LOAD_SIZE, size);
    };
    auto expectFile = [&](std::initializer_list<uint8_t> expected) {
        FILE* fp = std::fopen(directory.file.c_str(), "rb");
        if (!fp) return false;
        bool matches = true;
        for (auto byte : expected) {
            if (std::fgetc(fp) != byte) matches = false;
        }
        if (std::fgetc(fp) != EOF) matches = false;
        std::fclose(fp);
        return matches;
    };
    select(0xF00010, 4);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE) != 0) return fail("missing save should return zero");
    std::memcpy(vgs->ctx.ram + 0x10, "ABCD", 4);
    vgs->saveImplicit();
    if (FILE* fp = std::fopen(directory.file.c_str(), "rb")) {
        std::fclose(fp);
        return fail("implicit save must default to off");
    }
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 2);
    vgs->outPort(VGS_ADDR_EXIT, 0);
    if (!expectFile({'A', 'B', 'C', 'D'})) return fail("exit should save full missing-file load request");

    vgs->reset();
    select(0xF00020, 6);
    std::memset(vgs->ctx.ram + 0x20, 'Z', 6);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE) != 4) return fail("short load should return actual size");
    vgs->ctx.ram[0x20] = 'X';
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 1);
    // Merely changing the address and size registers must not replace the last request.
    select(0xF00040, 2);
    vgs->saveImplicit();
    if (!expectFile({'X', 'B', 'C', 'D', 'Z', 'Z'})) return fail("short load should remember requested size and address");

    select(0xF00030, 2);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE) != 2) return fail("sized load should cap a larger file");
    vgs->ctx.ram[0x30] = 'Y';
    vgs->saveImplicit();
    if (!expectFile({'Y', 'B'})) return fail("latest load must replace save region");

    std::memcpy(vgs->ctx.ram + 0x40, "123", 3);
    vgs->outPort(VGS_ADDR_SAVE_ADDRESS, 0xF00040);
    vgs->outPort(VGS_ADDR_SAVE_EXECUTE, 3);
    if (!expectFile({'1', '2', '3'})) return fail("explicit save must work with implicit save enabled");
    vgs->ctx.ram[0x40] = '4';
    select(0xEFFFFF, 2);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE)) return fail("reject load outside WRAM");
    select(0xFFFFFF, 2);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE)) return fail("reject load extending beyond WRAM");
    vgs->outPort(VGS_ADDR_SAVE_EXECUTE, 2);
    select(0xF00000, 0xFFFFFFFF);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE)) return fail("reject oversized load");
    vgs->outPort(VGS_ADDR_SAVE_EXECUTE, 0);
    vgs->saveImplicit();
    if (!expectFile({'4', '2', '3'})) return fail("invalid requests must preserve last valid save region");
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 0);
    vgs->ctx.ram[0x40] = '5';
    vgs->saveImplicit();
    if (!expectFile({'4', '2', '3'})) return fail("disabling implicit save should suppress writes");
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 0xFFFFFFFF);
    vgs.reset();
    if (!expectFile({'5', '2', '3'})) return fail("runtime destruction should save latest RAM");

    vgs = std::make_unique<VGSX>();
    vgs->setSaveDataDirectory(directory.path);
    select(0xFFFFFF, 1);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE) != 1) return fail("load last WRAM byte");
    vgs->ctx.ram[0xFFFFF] = '6';
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 1);
    vgs->reset();
    if (!expectFile({'6'})) return fail("reset should save before discarding region");
    vgs->ctx.ram[0xFFFFF] = '7';
    vgs->saveImplicit();
    if (!expectFile({'6'})) return fail("reset should disable implicit saving");
    select(0xF00050, 0);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE) != 1) return fail("legacy load should use file size");
    vgs->ctx.ram[0x50] = '8';
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 1);
    vgs->saveImplicit();
    if (!expectFile({'8'})) return fail("legacy load should remember actual file size");
    vgs->outPort(VGS_ADDR_SAVE_IMPLICIT, 0);
    vgs->setSaveDataDirectory((directory.file + "/missing").c_str());
    select(0xF00000, 1);
    vgs->outPort(VGS_ADDR_SAVE_EXECUTE, 1);
    if (vgs->inPort(VGS_ADDR_SAVE_EXECUTE)) return fail("failed file access should return zero");
    return 0;
}

int main()
{
    vgsx.disableBootBios();

    if (int rc = test_readme_vdp_register_doc(); rc) return rc;
    if (int rc = test_frame_pacer(); rc) return rc;
    if (int rc = test_native_display_upload(); rc) return rc;
    if (int rc = test_mode7(); rc) return rc;
    if (int rc = test_mode7_depth(); rc) return rc;
    if (int rc = test_mode7_backdrop(); rc) return rc;
    if (int rc = test_mode7_focal_length(); rc) return rc;
    if (int rc = test_mode7_perspective_horizontal_coverage(); rc) return rc;
    if (int rc = test_mode7_fractional_translation(); rc) return rc;
    if (int rc = test_random_full_cycle(vgsx); rc) return rc;
    if (int rc = test_random_seed_io(vgsx); rc) return rc;
    if (int rc = test_dma_memset_last_byte(vgsx); rc) return rc;
    if (int rc = test_seq_write_clamps_to_1mb(vgsx); rc) return rc;
    if (int rc = test_sprite_size_63_renders_512_pixels(); rc) return rc;
    if (int rc = test_sprite_large_scale_is_clipped_to_display(); rc) return rc;
    if (int rc = test_sprite_pivot_zoom_does_not_move_focus(); rc) return rc;
    if (int rc = test_sprite_pivot_flip_keeps_source_anchor(); rc) return rc;
    if (int rc = test_sprite_pivot_clipping_matches_full_frame_sampling(); rc) return rc;
    if (int rc = test_sprite_vertical_flip_and_bitmap_origin(); rc) return rc;
    if (int rc = test_sprite_alpha_uses_low_24_bits(); rc) return rc;
    if (int rc = test_sprite_battle_hanafuda_title_oam(); rc) return rc;
    if (int rc = test_sprite_sampling_matches_reference(); rc) return rc;
    if (int rc = test_sprite_boundaries_and_extreme_oam_values(); rc) return rc;
    if (int rc = test_sprite_ram_flip_scale_rotate_and_alpha(); rc) return rc;
    if (int rc = test_palette_1024_addressing_and_rendering(vgsx); rc) return rc;

    if (int rc = test_implicit_save(); rc) return rc;

    std::fprintf(stderr, "OK\n");
    return 0;
}
