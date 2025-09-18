#include <vgs.h>

int main(int argc, char* argv)
{
    char d32[12];
    int32_t ret = 123;
    vgs_console_print("call vgs_exit with exit code: ");
    vgs_d32str(d32, ret);
    vgs_console_println(d32);
    vgs_exit(ret);

    vgs_console_println("return 456 (this message will not shown)");
    return 456;
}
