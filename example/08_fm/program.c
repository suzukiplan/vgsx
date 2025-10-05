#include <vgs.h>

static inline int ym2612_read(int regNumber)
{
    return vgs_bgm_chip_read(VGS_FM_YM2612, regNumber);
}

unsigned int estimate_frequency_int(int fNumber, int block)
{
    const unsigned int clock = 7670450;
    unsigned long numerator = (unsigned long)fNumber * clock;
    unsigned int denominator = 1 << (block + 2);
    return (unsigned int)(numerator / denominator);
}

int get_channel_frequencies(int ch)
{
    int fNumber = ym2612_read(0xA0 + ch); // F-Number値
    int block = ym2612_read(0xB0 + ch);   // Block値
    return estimate_frequency_int(fNumber, block);
}

int main(int argc, char* argv[])
{
    vgs_bgm_play(0);
    vgs_print_bg(0, 1, 1, 0, "CH0 ");
    vgs_print_bg(0, 1, 2, 0, "CH1 ");
    vgs_print_bg(0, 1, 3, 0, "CH2 ");
    vgs_print_bg(0, 1, 4, 0, "CH3 ");
    vgs_print_bg(0, 1, 5, 0, "CH4 ");
    vgs_print_bg(0, 1, 6, 0, "CH5 ");

    char buf[80];
    while (ON) {
        for (int ch = 0; ch < 6; ch++) {
            vgs_d32str(buf, get_channel_frequencies(ch));
            vgs_strcat(buf, " HZ      ");
            vgs_print_bg(0, 5, 1 + ch, 0, buf);
        }
        vgs_vsync();
    }
    return 0;
}
