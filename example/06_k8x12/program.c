#include <vgs.h>

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, TRUE);
    vgs_pfont_init(0);
    vgs_k8x12_print(0, 8, 8, 0xFFFFFF, "VGS-X で 8×12ドット日本語フォント「k8x12」に対応。");
    vgs_k8x12_print(0, 8, 24, 0xFFFFFF, "ShiftJIS を JIS-X-0208 or JIS-X-0201 に変換しています。");
    vgs_k8x12_print(0, 8, 40, 0xFFFFFF, "RPGとかを開発したい場合に便利かもしれません。");
    vgs_k8x12_print(0, 8, 56, 0xFFFFFF, "英語はプロポーショナルフォント推奨です。（混在することができます）");
    vgs_k8x12_print(0, 8, 72, 0xFFFFFF, "When displayed in k8x12, it looks like this.");
    vgs_pfont_print(0, 8, 88, 0, 0, "When displayed in Proportional Font, it looks like this.");
    while (TRUE) {
        vgs_vsync();
    }
    return 0;
}
