#include <cstdint>
#include <cstdio>
#include <vector>

#include "vgsx.h"
#include "vgs_io.h"

static int fail(const char* msg)
{
    std::fprintf(stderr, "FAIL: %s\n", msg);
    return 1;
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

int main()
{
    vgsx.disableBootBios();

    if (int rc = test_random_full_cycle(vgsx); rc) return rc;
    if (int rc = test_dma_memset_last_byte(vgsx); rc) return rc;
    if (int rc = test_seq_write_clamps_to_1mb(vgsx); rc) return rc;

    std::fprintf(stderr, "OK\n");
    return 0;
}
