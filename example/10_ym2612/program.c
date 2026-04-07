#include <vgs.h>

enum {
    YM2612_CHANNELS = 6,
    PIANO_KEYS = 88,
    YM2612_VOLUME_IDLE = 8,
};

static const uint32_t noteMetrics[PIANO_KEYS] = {
    1083,
    1147,
    1215,
    1288,
    1364,
    1445,
    1531,
    1622,
    1719,
    1821,
    1929,
    2044,
    2166,
    2294,
    2430,
    2576,
    2728,
    2890,
    3062,
    3244,
    3438,
    3642,
    3858,
    4088,
    4332,
    4588,
    4860,
    5152,
    5456,
    5780,
    6124,
    6488,
    6876,
    7284,
    7716,
    8176,
    8664,
    9176,
    9720,
    10304,
    10912,
    11560,
    12248,
    12976,
    13752,
    14568,
    15432,
    16352,
    17328,
    18352,
    19440,
    20608,
    21824,
    23120,
    24496,
    25952,
    27504,
    29136,
    30864,
    32704,
    34656,
    36704,
    38880,
    41216,
    43648,
    46240,
    48992,
    51904,
    55008,
    58272,
    61728,
    65408,
    69312,
    73408,
    77760,
    82432,
    87296,
    92480,
    97984,
    103808,
    110016,
    116544,
    123456,
    130816,
    138624,
    146816,
    155520,
    164864,
};

static const BOOL noteIsBlack[12] = {
    FALSE,
    TRUE,
    FALSE,
    FALSE,
    TRUE,
    FALSE,
    TRUE,
    FALSE,
    FALSE,
    TRUE,
    FALSE,
    TRUE,
};

static const int noteWhiteIndex[12] = {
    0,
    0,
    1,
    2,
    2,
    3,
    3,
    4,
    5,
    5,
    6,
    6,
};

static uint32_t getYm2612Frequency(int ch)
{
    switch (ch) {
        case 0: return VGS_IN_YM2612_FREQ0;
        case 1: return VGS_IN_YM2612_FREQ1;
        case 2: return VGS_IN_YM2612_FREQ2;
        case 3: return VGS_IN_YM2612_FREQ3;
        case 4: return VGS_IN_YM2612_FREQ4;
        case 5: return VGS_IN_YM2612_FREQ5;
    }
    return 0;
}

static uint32_t getYm2612Volume(int ch)
{
    switch (ch) {
        case 0: return VGS_IN_YM2612_VOL0;
        case 1: return VGS_IN_YM2612_VOL1;
        case 2: return VGS_IN_YM2612_VOL2;
        case 3: return VGS_IN_YM2612_VOL3;
        case 4: return VGS_IN_YM2612_VOL4;
        case 5: return VGS_IN_YM2612_VOL5;
    }
    return 0;
}

// octave 0 の A (A0) から 88 鍵
static void renderKeyboard(int bx, int by, int ch, int activeKey)
{
    int y = by + ch * 20;

    // 白鍵を描画
    for (int i = 0; i < 52; i++) {
        vgs_draw_boxf(0, bx + i * 5, y, 4, 16, 0xFFFFFF);
    }
    // 白鍵のアクティブキーを描画
    if (0 <= activeKey && !noteIsBlack[activeKey % 12]) {
        int octave = activeKey / 12;
        int semitone = activeKey % 12;
        int whiteIndex = octave * 7 + noteWhiteIndex[semitone];
        vgs_draw_boxf(0, bx + whiteIndex * 5, y, 4, 16, 0xFF0000);
    }
    // 黒鍵を描画
    for (int i = 0; i < 50; i++) {
        int i7 = i % 7;
        if (1 == i7 || 4 == i7) {
            continue;
        }
        vgs_draw_boxf(0, bx + i * 5 + 2, y, 4, 12, 0x0001);
    }
    // 黒鍵のアクティブキーを描画
    if (0 <= activeKey && noteIsBlack[activeKey % 12]) {
        int octave = activeKey / 12;
        int semitone = activeKey % 12;
        int whiteIndex = octave * 7 + noteWhiteIndex[semitone];
        vgs_draw_boxf(0, bx + whiteIndex * 5 + 2, y, 4, 12, 0xFF0000);
    }
}

static int getPianoKeyFromFrequency(uint32_t frequency)
{
    uint32_t fnum = frequency & 0x7FF;
    uint32_t block = (frequency >> 11) & 0x7;
    uint32_t metric;
    int bestKey = -1;
    uint32_t bestDiff = 0xFFFFFFFF;

    if (!fnum) {
        return -1;
    }
    metric = fnum << block;
    for (int i = 0; i < PIANO_KEYS; i++) {
        uint32_t diff = (metric < noteMetrics[i]) ? (noteMetrics[i] - metric) : (metric - noteMetrics[i]);
        if (diff < bestDiff) {
            bestDiff = diff;
            bestKey = i;
        }
    }
    return bestKey;
}

static void render(int bx, int by)
{
    char chtext[4];
    vgs_strcpy(chtext, "FM1");
    for (int ch = 0; ch < YM2612_CHANNELS; ch++) {
        uint32_t volume;
        int activeKey = -1;
        chtext[2] = '1' + ch;
        vgs_pfont_print(0, bx - vgs_pfont_strlen(chtext) - 4, by + ch * 20 + 4, 0, 0, chtext);
        volume = getYm2612Volume(ch);
        if (YM2612_VOLUME_IDLE < volume) {
            activeKey = getPianoKeyFromFrequency(getYm2612Frequency(ch));
        }
        renderKeyboard(bx, by, ch, activeKey);
    }
}

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, ON);   // BG0: Bitmap Mode
    vgs_draw_mode(1, ON);   // BG1: Bitmap Mode
    vgs_draw_mode(2, ON);   // BG2: Bitmap Mode
    vgs_draw_mode(3, OFF);  // BG3: Character Pattern Mode
    vgs_sprite_priority(3); // Sprite: Displayed on top of BG3
    vgs_pfont_init(0);

    int bx = 32;
    int by = 16;
    vgs_pfont_print(0, 2, 2, 0, 0, "YM2612 Realtime Keyboard Preview");
    render(bx, by);

    while (!vgs_key_a() && !vgs_key_b() && !vgs_key_x() && !vgs_key_y() & !vgs_key_start()) {
        vgs_vsync();
    }
    vgs_bgm_play(0);

    while (ON) {
        render(bx, by);
        vgs_vsync();
    }
    return 0;
}
