#include <vgs.h>

int main(int argc, char* argv[])
{
    vgs_draw_mode(0, TRUE);
    vgs_pfont_init(0);
    vgs_k8x12_print(0, 8, 8, 0xFFFFFF, "VGS-X �� 8�~12�h�b�g���{��t�H���g�uk8x12�v�ɑΉ��B");
    vgs_k8x12_print(0, 8, 24, 0xFFFFFF, "ShiftJIS �� JIS-X-0208 or JIS-X-0201 �ɕϊ����Ă��܂��B");
    vgs_k8x12_print(0, 8, 40, 0xFFFFFF, "RPG�Ƃ����J���������ꍇ�ɕ֗���������܂���B");
    vgs_k8x12_print(0, 8, 56, 0xFFFFFF, "�p��̓v���|�[�V���i���t�H���g�����ł��B�i���݂��邱�Ƃ��ł��܂��j");
    vgs_k8x12_print(0, 8, 72, 0xFFFFFF, "When displayed in k8x12, it looks like this.");
    vgs_pfont_print(0, 8, 88, 0, 0, "When displayed in Proportional Font, it looks like this.");
    while (TRUE) {
        vgs_vsync();
    }
    return 0;
}
