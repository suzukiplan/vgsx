#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <thread>
#include "vgmdrv.hpp"
#include "vgm_fixture.hpp"

static void check(bool condition, const char* message)
{
    if (!condition) {
        std::fprintf(stderr, "FAIL: %s\n", message);
        std::exit(1);
    }
}

static std::vector<int16_t> render(VgmDriver& driver, int frames)
{
    std::vector<int16_t> pcm(frames * 2);
    driver.render(pcm.data(), pcm.size());
    return pcm;
}

int main()
{
    for (int rate : {22050, 44100, 48000}) {
        VgmDriver driver(rate, 2);
        check(driver.getPositionSamples() == 0, "initial position");
        auto data = makeVgm({0x61, 100, 0, 0x62, 0x63, 0x66});
        check(driver.load(data.data(), data.size()), "load synthetic VGM");
        check(driver.getPositionSamples() == 0, "load position");
        render(driver, 40);
        check(driver.getPositionSamples() == 40, "partial 0x61 wait must not read ahead");
        render(driver, 60);
        check(driver.getPositionSamples() == 100, "0x61 wait");
        render(driver, 1);
        check(driver.getPositionSamples() == 101, "partial 0x62 wait");
        render(driver, 734);
        check(driver.getPositionSamples() == 835, "0x62 wait");
        render(driver, 300);
        check(driver.getPositionSamples() == 1135, "partial 0x63 wait");
        render(driver, 582);
        check(driver.getPositionSamples() == 1717, "0x63 wait");
        render(driver, 500);
        check(driver.isEnded() && driver.getPositionSamples() == 1717, "end holds without counting buffer tail");
        render(driver, 123);
        check(driver.getPositionSamples() == 1717, "render after end holds");
        driver.reset();
        check(driver.getPositionSamples() == 0, "reset position");
        render(driver, 50);
        check(driver.getPositionSamples() == 0, "unloaded position");

        // DAC output makes PCM comparison sensitive to command timing.
        data = makeVgm({0x52, 0x2b, 0x80, 0x52, 0x2a, 0xd0,
                        0x61, 100, 0, 0x52, 0x2a, 0x30, 0x62, 0x63, 0x66}, 9);
        VgmDriver split(rate, 2);
        check(driver.load(data.data(), data.size()) && split.load(data.data(), data.size()), "load loop");
        auto whole = render(driver, 4000);
        std::vector<int16_t> parts;
        int remaining = 4000;
        for (int size : {1, 39, 777, 2, 1000, 13, 2168}) {
            auto pcm = render(split, size);
            parts.insert(parts.end(), pcm.begin(), pcm.end());
            remaining -= size;
        }
        check(remaining == 0 && parts == whole, "partitioned PCM");
        check(driver.getPositionSamples() == split.getPositionSamples(), "partitioned position");
        check(driver.getLoopCount() == 2 && driver.getPositionSamples() == 766, "loop rewinds to nonzero song position");
        check(driver.load(data.data(), data.size()), "restart");
        render(driver, 1717);
        check(driver.getPositionSamples() == 1717, "position immediately before loop");
        render(driver, 1);
        check(driver.getPositionSamples() == 101, "first consumed sample after loop start at 100");
        driver.setMute(ChipType::YM2612, 0, true);
        render(driver, 10);
        check(driver.getPositionSamples() == 111, "muting advances position");
        driver.setEnded();
        check(driver.getPositionSamples() == 0, "explicit stop clears position");
        data[0] = 0;
        check(!driver.load(data.data(), data.size()) && driver.getPositionSamples() == 0, "invalid load clears position");
    }

    VgmDriver driver(44100, 2);
    auto longSong = makeVgm({0x61, 0xff, 0xff, 0x66}, 0);
    check(driver.load(longSong.data(), longSong.size()), "load concurrent fixture");
    std::atomic<bool> done{false};
    std::thread audio([&]() {
        for (int i = 0; i < 100; i++) render(driver, 1000);
        done.store(true);
    });
    while (!done.load()) check(driver.getPositionSamples() <= 65535, "concurrent snapshot range");
    audio.join();
    check(driver.getPositionSamples() == 34465, "loop at song start");
    std::puts("VGM OK");
}
