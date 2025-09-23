#include <vgs.h>

enum KeyID {
    KeyID_A,
    KeyID_B,
    KeyID_X,
    KeyID_Y,
    KeyID_START,
};

static void put(enum KeyID id)
{
    static int pos = 5;
    VGS_IO_KEY_TYPE = VGS_KEY_ID_KEYBOARD;
    for (int i = 0; i < 4; i++) {
        VGS_IO_KEY_TYPE = i + 1;
        uint32_t buttonId = VGS_BUTTON_ID_UNKNOWN;
        switch (id) {
            case KeyID_A: buttonId = vgs_button_id_a(); break;
            case KeyID_B: buttonId = vgs_button_id_b(); break;
            case KeyID_X: buttonId = vgs_button_id_x(); break;
            case KeyID_Y: buttonId = vgs_button_id_y(); break;
            case KeyID_START: buttonId = vgs_button_id_start(); break;
        }
        vgs_print_bg(0, 2 + i * 9, pos, 0, vgs_button_name(buttonId));
    }
    pos++;
    if (26 <= pos) {
        vgs_exit(0);
    }
}

int main(int argc, char* argv[])
{
    struct Previous {
        BOOL a;
        BOOL b;
        BOOL x;
        BOOL y;
        BOOL start;
    } prev;
    vgs_memset(&prev, 0, sizeof(prev));

    vgs_print_bg(0, 2, 1, 0, "PUSH BUTTON");
    vgs_print_bg(0, 2, 3, 0, "KEYBOARD XBOX     SWITCH   PS");
    vgs_print_bg(0, 2, 4, 0, "-------- -------- -------- --------");

    while (TRUE) {
        vgs_vsync();
        if (!prev.a && vgs_key_a()) put(KeyID_A);
        if (!prev.b && vgs_key_b()) put(KeyID_B);
        if (!prev.x && vgs_key_x()) put(KeyID_X);
        if (!prev.y && vgs_key_y()) put(KeyID_Y);
        if (!prev.start && vgs_key_start()) put(KeyID_START);
        prev.a = vgs_key_a();
        prev.b = vgs_key_b();
        prev.x = vgs_key_x();
        prev.y = vgs_key_y();
        prev.start = vgs_key_start();
    }
    return 0;
}
