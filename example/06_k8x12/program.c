#include <vgs.h>

void print_utf8(int x, int y, const char* utf8)
{
    char sjis[256];
    vgs_sjis_from_utf8(sjis, utf8);
    vgs_k8x12_print(0, x, y, 0xFFFFFF, sjis);
}

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, TRUE);
    vgs_pfont_init(0);
    print_utf8(8, 8, "8x12ドット日本語フォント「k8x12」を用いてVGS-Xで日本語を表示できます。");
    print_utf8(8, 24, "日本語は「vgs_k8x12_print関数」で Bitmap Mode の BG に表示できます。");
    print_utf8(8, 40, "vgs_k8x12_print関数はゼロ終端の SJIS 文字列を指定する必要があります。");
    print_utf8(8, 56, "UTF-8 文字列は「vgs_sjis_from_utf8関数」で SJIS 文字列に変換できます。");
    print_utf8(8, 72, "ソースコードで直接日本語を指定したい場合は恐らくそれが便利です。");
    print_utf8(8, 88, "（ソースコードはSJISで記述するのであればそれは不要ですが...）");
    print_utf8(8, 104, "ゲームでは恐らく文字データを別アセットで管理すると思われるので、");
    print_utf8(8, 120, "アセットの文字データをSJISにして変換不要な形にすることを推奨します。");
    while (TRUE) {
        vgs_vsync();
    }
    return 0;
}
