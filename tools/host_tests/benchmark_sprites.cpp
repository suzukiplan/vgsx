// Run with: make -C tools/host_tests benchmark
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <memory>
#include <vector>

#include "vdp.hpp"

int main()
{
    auto vdp = std::make_unique<VDP>();
    vdp->reset();
    vdp->ctx.reg.skip0 = vdp->ctx.reg.skip1 = 1;
    vdp->ctx.reg.skip2 = vdp->ctx.reg.skip3 = 1;
    std::memset(vdp->ctx.ptn, 0x11, sizeof(vdp->ctx.ptn));
    vdp->ctx.palette[1][1] = 0xD06020;
    // Match the title cards' 40px size, locked Y scale, odd X scales and angles.
    for (int rotate : {0, 37}) {
        for (int i = 0; i < 256; i++) {
            auto& oam = vdp->ctx.oam[i];
            oam.visible = 1;
            oam.x = -48 + (i * 97) % 377;
            oam.y = -40 + (i * 53) % 249;
            oam.size = 4;
            oam.rotate = rotate ? -720 + i * 37 : 0;
            oam.scale = (i % 50) * 2 + 1;
            oam.sly = 1;
            oam.alpha = 0xFFFFFFFF;
            oam.attr = 0x10000;
        }
        std::vector<double> samples;
        for (int run = 0; run < 5; run++) {
            auto start = std::chrono::steady_clock::now();
            for (int frame = 0; frame < 60; frame++) vdp->render();
            samples.push_back(std::chrono::duration<double, std::milli>(
                std::chrono::steady_clock::now() - start).count() / 60);
        }
        std::sort(samples.begin(), samples.end());
        double ms = samples[samples.size() / 2];
        uint64_t hash = 0;
        for (auto pixel : vdp->ctx.display) hash = hash * 31 + pixel;
        std::printf("256 title cards (%s): median %.3f ms/frame, checksum=%llx\n",
            rotate ? "rotating" : "unrotated", ms, (unsigned long long)hash);
    }
}
